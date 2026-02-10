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
        // 1. Prepare the memory buffer (prevents flickering)
        std::string frame;
        frame.reserve(buffer.data.size() + 256); // Pre-allocate memory

        // 2. Variables for tracking position
        int row = 0;
        int col = 0;
        int cursor_r = 0;
        int cursor_c = 0;
        
        // Selection bounds (normalized)
        int sel_start = hl.active ? std::min(hl.start, hl.end) : -1;
        int sel_end   = hl.active ? std::max(hl.start, hl.end) : -1;

        // 3. Move terminal cursor to top-leftt (Do NOT clear screen yet)
        frame += "\033[H"; 

        // 4. Single Pass Loop: Build Frame AND Find Cursor
        for (size_t i = 0; i < buffer.data.size(); i++) {
            
            // --- CURSOR TRACKING ---
            // If we are at the gap_start, this is where the cursor belongs!
            if (i == buffer.gap_start) {
                cursor_r = row;
                cursor_c = col;
            }

            // --- GAP SKIPPING ---
            // If inside gap (and not debugging), skip drawing
            if (!debug_mode && i >= buffer.gap_start && i < buffer.gap_end) {
                continue;
            }
            
            // --- DEBUG VISUALIZATION ---
            if (debug_mode) {
                 if (i == buffer.gap_start) frame += "<";
                 if (i >= buffer.gap_start && i < buffer.gap_end) { frame += "_"; continue; }
                 if (i == buffer.gap_end) frame += ">";
            }

            // --- HIGHLIGHTING ---
            bool is_highlighted = hl.active && (i >= sel_start && i < sel_end);
            if (is_highlighted) frame += "\033[7m";

            // --- DRAW CHARACTER ---
            char c = buffer.data[i];
            frame += c;

            // --- POSITION TRACKING ---
            if (c == '\n') {
                row++;
                col = 0;
            } else if (c == '\t') {
                // If you want visual tabs, you need to append spaces to 'frame' here
                // For now, we just update the column counter logic
                int spaces = 8 - (col % 8);
                col += spaces;
            } else {
                col++;
            }

            if (is_highlighted) frame += "\033[0m";
        }

        // 5. Clear everything *after* our text (Cleaner than clearing the whole screen)
        frame += "\033[0J";

        // 6. Append the cursor move command
        // We use the coordinates we found during the loop!
        frame += "\033[" + to_string(cursor_r + 1) + ";" + to_string(cursor_c + 1) + "H";

        // 7. SINGLE OUTPUT CALL (Very Fast)
        cout << frame << flush;
    }
}
