#include "editor.hpp"
#include "buffer_service.hpp"
#include "clipboard.hpp"
#include "file_io.hpp"
#include "input_handler.hpp"
#include "logger.hpp"
#include "selection_service.hpp"
#include "terminal_manager.hpp"
#include "view_service.hpp"
#include <iostream>
#include <memory>
#include <unistd.h>

void Editor::init(const std::string &p) {
  path = p;
  gap_buffer = load_file(p);
  DEBUG_GAP = false;
}

void Editor::run() {
  running_ = true;
  if (!input_handler_)
    input_handler_ = create_offline_handler();
  TerminalManager::enableRawMode();
  view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);
  while (running_) {
    poll_network();
    process_input();
    view_service.print_buffer(gap_buffer, highligter, DEBUG_GAP);
    usleep(10000);
  }
  shutdown();
}

void Editor::poll_network() {
  if (mode_ != Mode::Client || !client_)
    return;
  if (!client_->has_new_data())
    return;
  std::string text = client_->get_latest_buffer();
  size_t cursor = gap_buffer.gap_start;
  gap_buffer.data.assign(text.begin(), text.end());
  if (cursor > text.size())
    cursor = text.size();
  gap_buffer.gap_start = cursor;
  gap_buffer.gap_end = cursor;
}

void Editor::start_host(int port) {
  mode_ = Mode::Host;
  input_handler_ = create_host_handler();
  io_context_ = std::make_shared<asio::io_context>();
  server_ = std::make_unique<tcp_server>(*io_context_, port);
  server_->start();
  Logger::instance().log("Listening on port " + std::to_string(port));
  network_thread_ = std::thread([this]() { io_context_->run(); });
}

void Editor::start_client(const std::string &host, int port) {
  mode_ = Mode::Client;
  input_handler_ = create_client_handler();
  io_context_ = std::make_shared<asio::io_context>();
  client_ = std::make_unique<tcp_client>(*io_context_, host, port);
  client_->start_receiving();
  network_thread_ = std::thread([this]() { io_context_->run(); });
}

void Editor::shutdown() {
  delete input_handler_;
  input_handler_ = nullptr;
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

void Editor::process_input() {
  TerminalManager::InputEvent e = TerminalManager::read_input_non_blocking();
  if (e.key == TerminalManager::Key::None)
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

  if (e.shift_held && !highligter.active) {
    SelectionService::start(highligter, gap_buffer);
  }

  if (!e.shift_held && e.key != TerminalManager::Key::Char &&
      e.key != TerminalManager::Key::Backspace &&
      e.key != TerminalManager::Key::Copy &&
      e.key != TerminalManager::Key::Paste) {
    SelectionService::clear(highligter);
  }

  input_handler_->handle(*this, e);

  if (e.shift_held) {
    SelectionService::update_endpoint(highligter, gap_buffer);
  }
  if (mode_ == Mode::Host) {
    broadcast_buffer();
  }
}