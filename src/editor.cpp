#include "editor.hpp"
#include "buffer_service.hpp"
#include "clipboard.hpp"
#include "file_io.hpp"
#include "logger.hpp"
#include "selection_service.hpp"
#include "terminal_manager.hpp"
#include "view_service.hpp"
#include <iostream>
#include <memory>
#include <unistd.h>
using namespace std;

void Editor::init(const std::string &p) {
  path = p;
  gap_buffer = load_file(path);
  DEBUG_GAP = false;
}
void Editor::start_writing() {
  TerminalManager::enableRawMode();
  running_ = true;
  view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);

  if (!is_host) {
    client_loop();
    return;
  }
  broadcast_buffer();
  while (running_) {
    process_input();
    view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);
  }
  shutdown();
}
void Editor::start_host(int port) {
  is_host = true;
  io_context_ = std::make_shared<asio::io_context>();
  server_ = std::make_unique<tcp_server>(*io_context_, port);
  server_->start();
  Logger::instance().log("Listening on port " + std::to_string(port));
  network_thread_ = std::thread([this]() { io_context_->run(); });
}

void Editor::start_client(const std::string &host, int port) {
  is_host = false;
  io_context_ = std::make_shared<asio::io_context>();
  client_ = std::make_unique<tcp_client>(*io_context_, host, port);
  client_->start_receiving();
  network_thread_ = std::thread([this]() { io_context_->run(); });
}
void Editor::shutdown() {
  if (server_)
    server_->stop();
  if (client_)
    client_->stop();
  if (network_thread_.joinable())
    network_thread_.join();
}
void Editor::broadcast_buffer() {
  if (!server_)
    return;
  std::string data = get_buffer_text();
  asio::post(server_->get_io_context(),
             [this, data]() { server_->broadcast(data); });
}

std::string Editor::get_buffer_text() {
  return BufferService::serialize_buffer(gap_buffer);
}

using namespace TerminalManager;
void Editor::client_loop() {
  TerminalManager::enableRawMode();
  running_ = true;
  view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);

  while (running_) {
    TerminalManager::InputEvent e = TerminalManager::read_input_non_blocking();
    bool should_print = false;
    if (e.key == TerminalManager::Key::Quit) {
      running_ = false;
      break;
    }
    switch (e.key) {
    case Key::Up:
      BufferService::move_cursor_up(gap_buffer);
      should_print = true;
      break;
    case Key::Down:
      BufferService::move_cursor_down(gap_buffer);
      should_print = true;
      break;
    case Key::Left:
      if (e.ctrl_held) {
        BufferService::move_word_left(gap_buffer);
        should_print = true;
      } else {
        BufferService::move_cursor_left(gap_buffer);
        should_print = true;
      }
      break;
    case Key::Right:
      if (e.ctrl_held) {
        BufferService::move_word_right(gap_buffer);
        should_print = true;
      } else {
        BufferService::move_cursor_right(gap_buffer);
        should_print = true;
      }
      break;
    default:
      break;
    }
    if (client_ && client_->has_new_data()) {
      std::string text = client_->get_latest_buffer();
      size_t cursor = gap_buffer.gap_start;
      gap_buffer.data.assign(text.begin(), text.end());
      if (cursor > text.size())
        cursor = text.size();
      gap_buffer.gap_start = cursor;
      gap_buffer.gap_end = cursor;
      should_print = true;
    }
    if (should_print) {
      view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);
    }

    usleep(10000); // ~100fps poll rate
  }
  shutdown();
}
void Editor::process_input() {
  InputEvent e = read_input();
  if (e.key == Key::None)
    return;

  if (changefileModal.is_active()) {
    bool done = changefileModal.handleInput(e);
    if (done) {
      if (changefileModal.is_confirmed()) {
        save_file(gap_buffer, path);
        init(changefileModal.get_path());
      }
      changefileModal.deactivate();
    }
    return;
  }

  // If Shift is held, ensure we are in selection mode.
  if (e.shift_held && !highligter.active) {
    SelectionService::start(highligter, gap_buffer);
  }

  // If Shift is NOT held and we move, clear selection.
  if (!e.shift_held && e.key != Key::Char && e.key != Key::Backspace &&
      e.key != Key::Copy && e.key != Key::Paste) {
    SelectionService::clear(highligter);
  }

  switch (e.key) {
  case Key::Up:
    BufferService::move_cursor_up(gap_buffer);
    break;
  case Key::Down:
    BufferService::move_cursor_down(gap_buffer);
    break;
  case Key::Left:
    if (e.ctrl_held) {
      BufferService::move_word_left(gap_buffer);
    } else {
      BufferService::move_cursor_left(gap_buffer);
    }
    break;
  case Key::Right:
    if (e.ctrl_held) {
      BufferService::move_word_right(gap_buffer);
    } else {
      BufferService::move_cursor_right(gap_buffer);
    }
    break;

  case Key::Backspace:
    // BufferService::delete_backspace(gap_buffer); // Implement this in
    // buffer_service
    if (gap_buffer.gap_start > 0)
      gap_buffer.gap_start--;
    break;

  case Key::Char:
  case Key::Enter: // Treated as char '\n'
    BufferService::insert_char(gap_buffer, e.value);
    break;

  case Key::Quit:
    save_file(gap_buffer, path);
    running_ = false;
    break;

  case Key::Copy:
    if (highligter.active) {

      string out;
      size_t start = std::min(highligter.start, highligter.end);
      size_t end = std::max(highligter.start, highligter.end);

      for (size_t i = start; i < end; i++) {
        // if (i >= gap_buffer.gap_start && i < gap_buffer.gap_end) continue;
        out.push_back(gap_buffer.data[i]);
      }

      ClipboardService::copy(out);
    }
    break;
  case Key::Paste: {
    std::string system_text = ClipboardService::paste();

    for (char c : system_text) {

      if (c == '\r')
        continue;
      BufferService::insert_char(gap_buffer, c);
    }

    SelectionService::clear(highligter);
  } break;
  case Key::ChangeFile: {
    changefileModal.activate();
    break;
  }
  default:
    break;
  }
  if (e.shift_held) {
    SelectionService::update_endpoint(highligter, gap_buffer);
  }
  if (is_host) {
    broadcast_buffer();
  }
}
