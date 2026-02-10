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
