#include "changefilemodal.hpp"
#include "terminal_manager.hpp"
#include <stdlib.h>
#include <string>

void ChangeFileModal::activate() {
  active = true;
  confirmed = false;
  path.clear();
}
void ChangeFileModal::deactivate() { active = false; }
bool ChangeFileModal::is_active() const { return active; }
bool ChangeFileModal::handleInput(TerminalManager::InputEvent e) {
  switch (e.key) {
  case TerminalManager::Key::Char:
    path.push_back(e.value);
    return false;
  case TerminalManager::Key::Backspace:
    if (!path.empty())
      path.pop_back();
    return false;
  case TerminalManager::Key::Enter:
    confirmed = true;
    return true;
  case TerminalManager::Key::Escape:
    confirmed = false;
    return true;
  default:
    return false;
  }
}
bool ChangeFileModal::is_confirmed() const { return confirmed; }
const std::string &ChangeFileModal::get_path() const { return path; }
void ChangeFileModal::reset() {
  path.clear();
  active = false;
  confirmed = false;
}
