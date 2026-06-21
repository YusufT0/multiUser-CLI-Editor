#include "view_service.hpp"
#include "editor.hpp"
#include "terminal_manager.hpp"
#include <algorithm>
#include <changefilemodal.hpp>
#include <iostream>
#include <string>

using namespace std;

ViewService::ViewService(int target_width, int target_height)
    : m_target_width(target_width), m_target_height(target_height),
      m_top_padding(0), m_left_padding(0) {

  // Calculate layout variables immediately on creation
  update_layout();
}

void ViewService::update_layout() {
  int term_w = 0, term_h = 0;
  TerminalManager::getTerminalSize(term_w, term_h);

  m_left_padding = std::max(0, (term_w - m_target_width) / 2);
  m_top_padding = std::max(0, (term_h - m_target_height) / 3);

  m_left_pad_str = std::string(m_left_padding, ' ');
}

void ViewService::draw_change_file_modal(std::string &frame,
                                         const ChangeFileModal &model,
                                         int &out_cursor_r,
                                         int &out_cursor_c) const {
  int term_w, term_h;
  TerminalManager::getTerminalSize(term_w, term_h);
  int box_w = TER_END_X + 2;
  int top = max(1, (term_h - 3) / 2 + 1);
  int left = max(1, (term_w - box_w) / 2 + 1);

  // Top border
  frame += "\033[" + to_string(top) + ";" + to_string(left) + "H";
  frame += "┌";
  for (int i = 0; i < TER_END_X; i++)
    frame += "─";
  frame += "┐";

  // Content line: │ File: /path                  │
  string prefix = "File: ";
  string content = prefix + model.get_path();
  frame += "\033[" + to_string(top + 1) + ";" + to_string(left) + "H";
  frame += "│ " + content;
  int used = 2 + content.length();
  int remaining = box_w - used - 1;
  for (int i = 0; i < remaining; i++)
    frame += " ";
  frame += "│";

  // Bottom border
  frame += "\033[" + to_string(top + 2) + ";" + to_string(left) + "H";
  frame += "└";
  for (int i = 0; i < TER_END_X; i++)
    frame += "─";
  frame += "┘";

  // Cursor position after "│ File: "
  out_cursor_r = top + 1;
  out_cursor_c = left + 2 + static_cast<int>(prefix.length() + model.get_path().length());
}
void ViewService::print_buffer(const GapBuffer &buffer, const Highlight &hl,
                               bool debug_mode) {

  std::string frame;
  int start_row = 0;
  bool needs_left_pad = true;
  int cursor_row_abs = 0; // Absolute row in the file
  size_t gap_size = buffer.gap_end - buffer.gap_start;

  // This is needed for scrolling screen.
  // With the absolute position of cursor we can scroll without losing cursor.
  {
    int col = 0;
    for (size_t i = 0; i < buffer.gap_start; i++) {
      if (buffer.data[i] == '\n') {
        cursor_row_abs++;
        col = 0;
      } else if (buffer.data[i] == '\t') {
        col += (8 - (col % 8));
      } else {
        col++;
      }
      if (col >= TER_END_X) {
        cursor_row_abs++;
        col = 0;
      }
    }
  }
  if (cursor_row_abs >= TER_END_Y / 2) {
    start_row = cursor_row_abs - TER_END_Y / 2;
  }

  frame.reserve(buffer.data.size() +
                512); // Increased reserve slightly to account for box drawing

  int row = 0;
  int col = 0;
  int cursor_r = 0;
  int cursor_c = 0;

  // Selection bounds (normalized)
  std::size_t sel_start = 0;
  std::size_t sel_end = 0;

  if (hl.active) {
    sel_start = std::min(hl.start, hl.end);
    sel_end = std::max(hl.start, hl.end);
  }

  frame += "\033[H";

  // ---- PADDING TOP ---------
  for (int i = 0; i < m_top_padding; i++) {
    frame += "\033[K\n";
  }

  // Build Frame AND Find Cursor
  for (size_t i = 0; i < buffer.data.size(); i++) {

    // --- CURSOR TRACKING ---
    if (i == buffer.gap_start) {
      cursor_r = row - start_row;
      cursor_c = col;
    }

    // --- GAP SKIPPING ---
    if (!debug_mode && i >= buffer.gap_start && i < buffer.gap_end) {
      continue;
    }

    // --- HIGHLIGHTING ---
    size_t logical_i = (i < buffer.gap_start) ? i : (i - gap_size);

    bool is_highlighted =
        hl.active && (logical_i >= sel_start && logical_i < sel_end);

    // --- DEBUG VISUALIZATION ---
    if (debug_mode) {
      if (i == buffer.gap_start)
        frame += "<";
      if (i >= buffer.gap_start && i < buffer.gap_end) {
        frame += "_";
        continue;
      }
      if (i == buffer.gap_end)
        frame += ">";
    }

    // --- DRAW CHARACTER ---
    char c = buffer.data[i];
    bool visible = (row >= start_row) && (row < start_row + TER_END_Y);

    if (visible) {

      if (needs_left_pad) {
        frame += m_left_pad_str;
        needs_left_pad = false;
      }
      if (is_highlighted)
        frame += "\033[7m"; // Start Highlight

      if (c == '\n')
        frame += "\033[K";
      frame += c;

      if (is_highlighted)
        frame += "\033[0m"; // Reset Highlight
    }

    if (c == '\n') {
      row++;
      col = 0;
      if (visible)
        needs_left_pad = true;
    } else if (c == '\t') {
      col += (8 - (col % 8));
    } else {
      col++;
    }

    // Force wrap at TER_END_X columns
    if (col >= TER_END_X && c != '\n') {
      if (visible) {
        frame += "\033[K\n";
        needs_left_pad = true;
      }
      row++;
      col = 0;
    }

    if (row >= start_row + TER_END_Y && i > buffer.gap_start) {
      break;
    }
  }

  // Clear everything
  frame += "\033[0J";
  // --- BOX DRAWING LOGIC---
  if (m_left_padding >= 2 && m_top_padding >= 2) {

    // Convert to 1-indexed coordinates for ANSI sequences
    int top_y = m_top_padding - 1;
    int bot_y = m_top_padding + TER_END_Y + 2;
    int left_x = m_left_padding - 1;
    int right_x = m_left_padding + TER_END_X + 2;

    frame += "\033[" + to_string(top_y) + ";" + to_string(left_x) + "H";
    frame += "┌";
    for (int x = left_x + 1; x < right_x; ++x)
      frame += "─";
    frame += "┐";

    frame += "\033[" + to_string(bot_y) + ";" + to_string(left_x) + "H";
    frame += "└";
    for (int x = left_x + 1; x < right_x; ++x)
      frame += "─";
    frame += "┘";

    for (int y = top_y + 1; y < bot_y; ++y) {
      frame += "\033[" + to_string(y) + ";" + to_string(left_x) + "H";
      frame += "│"; // Left

      frame += "\033[" + to_string(y) + ";" + to_string(right_x) + "H";
      frame += "│"; // Right
    }
    std::string filename = Editor::get_instance().get_filename();
    std::string display_name = " " + filename + " ";

    int filename_x = right_x - static_cast<int>(display_name.length());
    frame += "\033[" + to_string(bot_y + 1) + ";" + to_string(filename_x) + "H";
    frame += display_name;
  }

  if (Editor::get_instance().get_change_file_modal().is_active()) {
    int modal_r, modal_c;
    draw_change_file_modal(frame, Editor::get_instance().get_change_file_modal(),
                           modal_r, modal_c);
    frame += "\033[" + to_string(modal_r) + ";" + to_string(modal_c) + "H";
  } else {
    frame += "\033[" + to_string(cursor_r + 1 + m_top_padding) + ";" +
             to_string(cursor_c + 1 + m_left_padding) + "H";
  }
  cout << frame << flush;
}
