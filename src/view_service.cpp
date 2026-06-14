#include "view_service.hpp"
#include "terminal_manager.hpp"
#include <iostream>

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
  m_top_padding = std::max(0, (term_h - m_target_height) / 2);

  m_left_pad_str = std::string(m_left_padding, ' ');
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

  frame.reserve(buffer.data.size() + 256);

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

  // Cursor movement
  frame += "\033[" + to_string(cursor_r + 1 + m_top_padding) + ";" +
           to_string(cursor_c + 1 + m_left_padding) + "H"; // OUTPUT CALL
  cout << frame << flush;
}
