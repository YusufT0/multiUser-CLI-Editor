#pragma once
#include "models.hpp"
#include <tree_sitter/api.h>
#include <string>

namespace SyntaxService {
    // Call this once at startup
    void init();

    // Call this every time the buffer changes (e.g. in Editor::process_input)
    void update(const GapBuffer &buffer);

    // Call this during rendering to get the color for a specific character
    // Returns an ANSI color code (e.g., 31 for Red, 34 for Blue)
    // Returns 0 if no color.
    int get_color_at(size_t char_index);

    // Call this at exit
    void shutdown();
}