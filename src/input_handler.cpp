#include "input_handler.hpp"
#include "buffer_service.hpp"
#include "clipboard.hpp"
#include "editor.hpp"
#include "file_io.hpp"
#include "selection_service.hpp"
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

class OfflineInputHandler : public InputHandler {
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
    case TerminalManager::Key::ChangeFile:
      editor.get_change_file_modal().activate();
      break;
    default:
      break;
    }
  }
};

InputHandler *create_client_handler() { return new ClientInputHandler(); }
InputHandler *create_host_handler() { return new HostInputHandler(); }
InputHandler *create_offline_handler() { return new OfflineInputHandler(); }
