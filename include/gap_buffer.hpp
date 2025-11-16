#pragma once
#include <vector>
#include <string>
struct CursorPos { int row; int col; };
struct GapBuffer {
    std::vector<char> data;
    int gap_start, gap_end;
};
GapBuffer create_gap_buffer();
CursorPos get_cursor_pos(const GapBuffer&);
void move_cursor_left(GapBuffer&);
void move_cursor_right(GapBuffer&);
void move_cursor_up(GapBuffer&);
void move_cursor_down(GapBuffer&);
void grow_gap(GapBuffer &buffer, size_t amount = 8);
void insert_char(GapBuffer &buffer, char c);
void update_gap_buffer(GapBuffer &buffer, const std::string &filename);
void print_buffer(const GapBuffer&);