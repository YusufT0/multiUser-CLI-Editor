#pragma once  
#include <vector>
#include <string>

struct CursorPos { int row; int col; };

struct GapBuffer {
    std::vector<char> data;
    std::size_t gap_start, gap_end;
};

struct Highlight {
    bool active = false;
    size_t start = 0;   // where selection started
    size_t end = 0;   // current position (same as your gap_start)
};
