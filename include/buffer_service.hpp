#pragma once
#include "models.hpp"

namespace BufferService{
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
}

