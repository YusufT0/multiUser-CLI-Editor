
#ifndef EDITOR_HPP
#define EDITOR_HPP
#include "models.hpp"
#include "view_service.hpp"
#include <string>
class Editor {
private:
  std::string path;
  GapBuffer gap_buffer;
  Highlight highligter;
  bool DEBUG_GAP;
  ViewService view_service;
  Editor()
      : DEBUG_GAP(false),
        view_service(ViewService::TER_END_X, ViewService::TER_END_Y) {}
  ~Editor() = default;
  void process_input();

public:
  static Editor &get_instance() {
    static Editor instance;
    return instance;
  }

  Editor(const Editor &) = delete; // We do not want copy constructors. Do not
                                   // create a new editor by copying an old one.
  Editor &operator=(const Editor &) =
      delete; // Do not overwrite an existing editor with an old one.
  void init(const std::string &p);
  void start_writing();
};

#endif
