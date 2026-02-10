#include "view_service.hpp"
#include <iostream>

using namespace std;
namespace ViewService{
    
    CursorPos get_cursor_screen_pos(const GapBuffer& buffer) {
    int row = 0;
    int col = 0;

    for (size_t i = 0; i < buffer.gap_start; i++) {
        if (buffer.data[i] == '\n') {
            row++;
            col = 0;
        } 
        else if (buffer.data[i] == '\t'){
            col = (col / 8 + 1) * 8; // Tab logic
        }
        else {
            col++;
        }
    }
    return {row, col};}

    void print_buffer(const GapBuffer &buffer, const Highlight &hl, bool debug_mode) {
        // 1. Clear Screen (ANSI: Clear entire screen, Move to top-left)
        cout << "\033[2J\033[H";

        int sel_start = hl.active ? hl.start : -1;
        int sel_end   = hl.active ? hl.end   : -1;

        // 2. Render Text
        for (size_t i = 0; i < buffer.data.size(); i++) {
            
            // Debug Visualization
            if (debug_mode) {
                if (i == buffer.gap_start) cout << "<";
                if (i >= buffer.gap_start && i < buffer.gap_end) {
                    cout << "_";
                    continue;
                }
                if (i == buffer.gap_end) cout << ">";
            }

            // Skip Gap (Standard Mode)
            if (!debug_mode && i >= buffer.gap_start && i < buffer.gap_end)
                continue;

            // Handle Highlighting
            bool is_highlighted = hl.active && (i >= sel_start && i < sel_end);
            
            if (is_highlighted) cout << "\033[7m"; // Inverse colors
            cout << buffer.data[i];
            if (is_highlighted) cout << "\033[0m"; // Reset colors
        }

        // 3. Render Cursor
        // Notice we call the sibling function in the same namespace
        CursorPos pos = get_cursor_screen_pos(buffer);
        
        // ANSI: Move cursor to specific Row;Col
        cout << "\033[" << pos.row + 1 << ";" << pos.col + 1 << "H" << flush;
        }
}
