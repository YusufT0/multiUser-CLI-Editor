#pragma once
#include "models.hpp"

namespace ViewService {
    // Calculates Row/Col based on buffer content
    CursorPos get_cursor_screen_pos(const GapBuffer& buffer);
    void print_buffer(const GapBuffer& buffer, const Highlight& hl, bool debug_mode = false);
}