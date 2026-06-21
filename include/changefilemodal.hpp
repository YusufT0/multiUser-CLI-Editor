#pragma once
#include "terminal_manager.hpp"
#include <stdlib.h>
#include <string>

class ChangeFileModal {
private:
  bool active = false;
  std::string path;
  bool confirmed = false;

public:
  void activate();
  void deactivate();
  bool is_active() const;
  bool is_confirmed() const;
  bool handleInput(TerminalManager::InputEvent e);
  const std::string &get_path() const;
  void reset();
};
