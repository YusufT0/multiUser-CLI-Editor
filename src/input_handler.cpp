#include "input_handler.hpp"
#include "buffer_service.hpp"
#include "clipboard.hpp"
#include "editor.hpp"
#include "file_io.hpp"
#include "selection_service.hpp"
#include "logger.hpp"
#include <algorithm>

class ClientInputHandler : public InputHandler {
public:
  void handle(Editor &editor, const TerminalManager::InputEvent &e) override {
    auto &buf = editor.get_buffer();
    switch (e.key) {
    case TerminalManager::Key::Up:
      BufferService::move_cursor_up(buf);
      break;
    case TerminalManager::Key::Down:
      BufferService::move_cursor_down(buf);
      break;
    case TerminalManager::Key::Left:
      if (e.ctrl_held)
        BufferService::move_word_left(buf);
      else
        BufferService::move_cursor_left(buf);
      break;
    case TerminalManager::Key::Right:
      if (e.ctrl_held)
        BufferService::move_word_right(buf);
      else
        BufferService::move_cursor_right(buf);
      break;
    case TerminalManager::Key::Quit:
      save_file(buf, editor.get_filename());
      editor.set_running(false);
      break;
    case TerminalManager::Key::Save:
      save_file(buf, editor.get_filename());
      Logger::instance().log("File saved.");
      break;
    default:
      break;
    }
  }
};

class HostInputHandler : public InputHandler {
public:
  void handle(Editor &editor, const TerminalManager::InputEvent &e) override {
    auto &buf = editor.get_buffer();
    auto &hl = editor.get_highlight();
    switch (e.key) {
    case TerminalManager::Key::Up:
      BufferService::move_cursor_up(buf);
      break;
    case TerminalManager::Key::Down:
      BufferService::move_cursor_down(buf);
      break;
    case TerminalManager::Key::Left:
      if (e.ctrl_held)
        BufferService::move_word_left(buf);
      else
        BufferService::move_cursor_left(buf);
      break;
    case TerminalManager::Key::Right:
      if (e.ctrl_held)
        BufferService::move_word_right(buf);
      else
        BufferService::move_cursor_right(buf);
      break;
    case TerminalManager::Key::Backspace:
      if (buf.gap_start > 0)
        buf.gap_start--;
      break;
    case TerminalManager::Key::Char:
    case TerminalManager::Key::Enter:
      BufferService::insert_char(buf, e.value);
      break;
    case TerminalManager::Key::Quit:
      save_file(buf, editor.get_filename());
      editor.set_running(false);
      break;
    case TerminalManager::Key::Save:
      save_file(buf, editor.get_filename());
      Logger::instance().log("File saved.");
      break;
    case TerminalManager::Key::Copy:
      if (hl.active) {
        std::string out;
        size_t start = std::min(hl.start, hl.end);
        size_t end = std::max(hl.start, hl.end);
        for (size_t i = start; i < end; i++)
          out.push_back(buf.data[i]);
        ClipboardService::copy(out);
      }
      break;
    case TerminalManager::Key::Paste: {
      std::string text = ClipboardService::paste();
      for (char c : text) {
        if (c == '\r')
          continue;
        BufferService::insert_char(buf, c);
      }
      SelectionService::clear(hl);
    } break;
    default:
      break;
    }
  }
};

class OfflineInputHandler : public HostInputHandler {
public:
  void handle(Editor &editor, const TerminalManager::InputEvent &e) override {
    if (e.key == TerminalManager::Key::ChangeFile) {
      editor.get_change_file_modal().activate();
      return;
    }
    HostInputHandler::handle(editor, e);
  }
};

std::unique_ptr<InputHandler> create_client_handler() {
  return std::make_unique<ClientInputHandler>();
}
std::unique_ptr<InputHandler> create_host_handler() {
  return std::make_unique<HostInputHandler>();
}
std::unique_ptr<InputHandler> create_offline_handler() {
  return std::make_unique<OfflineInputHandler>();
}
