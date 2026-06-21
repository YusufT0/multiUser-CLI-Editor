#pragma once
#include "changefilemodal.hpp"
#include "models.hpp"
#include <string>

class ViewService {
public:
  ViewService(int terminal_width, int terminal_height);
  void print_buffer(const GapBuffer &buffer, const Highlight &hl,
                    bool debug_mode = false);
  void draw_change_file_modal(std::string &frame, const ChangeFileModal &model,
                              int &out_cursor_r, int &out_cursor_c) const;
  void update_layout();
  static const int TER_END_Y = 20;
  static const int TER_END_X = 60;

private:
  int m_target_width;
  int m_target_height;

  int m_top_padding;
  int m_left_padding;
  std::string m_left_pad_str;
};
