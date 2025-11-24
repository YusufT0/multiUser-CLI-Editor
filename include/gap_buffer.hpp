#pragma once
#include <vector>
#include <string>
struct CursorPos { int row; int col; };
struct GapBuffer {
    std::vector<char> data;
    int gap_start, gap_end;
};
struct Highlight {
    bool active = false;
    int start = 0;   // where selection started
    int end = 0;   // current position (same as your gap_start)
};

GapBuffer create_gap_buffer();
CursorPos get_cursor_pos(const GapBuffer&);
Highlight create_highlight();
void move_cursor_left(GapBuffer&);
void move_cursor_right(GapBuffer&);
void move_cursor_up(GapBuffer&);
void move_cursor_down(GapBuffer&);
void grow_gap(GapBuffer &buffer, size_t amount = 8);
void insert_char(GapBuffer &buffer, char c);
void update_gap_buffer(GapBuffer &buffer, const std::string &filename, Highlight &hl);
void print_buffer(const GapBuffer&, Highlight&);