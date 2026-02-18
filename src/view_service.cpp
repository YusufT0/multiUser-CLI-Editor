#include "view_service.hpp"
#include <iostream>

using namespace std;
namespace ViewService{
    // CursorPos get_cursor_screen_pos(const GapBuffer& buffer) {
    // int row = 0;
    // int col = 0;

    // for (size_t i = 0; i < buffer.gap_start; i++) {
    //     if (buffer.data[i] == '\n') {
    //         row++;
    //         col = 0;
    //     } 
    //     else if (buffer.data[i] == '\t'){
    //         col = (col / 8 + 1) * 8; // Tab logic
    //     }
    //     else {
    //         col++;
    //     }
    // }
    // return {row, col};}

    void print_buffer(const GapBuffer &buffer, const Highlight &hl, bool debug_mode) {
        
        std::string frame;
        int start_row = 0;
        int cursor_row_abs = 0; // Absolute row in the file
        size_t gap_size = buffer.gap_end - buffer.gap_start;
        
        
        for (size_t i = 0; i < buffer.gap_start; i++) {
            if (buffer.data[i] == '\n') cursor_row_abs++;
        }
        if (cursor_row_abs >= TER_END) {
            start_row = cursor_row_abs - TER_END + 1;
        }
        
        frame.reserve(buffer.data.size() + 256); // Pre-allocate memory

        int row = 0;
        int col = 0;
        int cursor_r = 0;
        int cursor_c = 0;
        
        // Selection bounds (normalized)
        std::size_t sel_start = 0;
        std::size_t sel_end = 0;

        if (hl.active) {
            sel_start = std::min(hl.start, hl.end);
            sel_end   = std::max(hl.start, hl.end);
        }

        // 3. Move terminal cursor to top-left (Do NOT clear screen yet)
        frame += "\033[H"; 

        // Build Frame AND Find Cursor
        for (size_t i = 0; i < buffer.data.size(); i++) {
            
            // --- CURSOR TRACKING ---
            // If we are at the gap_start, this is where the cursor belongs!
            if (i == buffer.gap_start) {
                cursor_r = row - start_row;
                cursor_c = col;
            }

            // --- GAP SKIPPING ---
            // If inside gap (and not debugging), skip drawing
            if (!debug_mode && i >= buffer.gap_start && i < buffer.gap_end) {
                continue;
            }
            
            size_t logical_i = (i < buffer.gap_start) ? i : (i - gap_size);

            bool is_highlighted = hl.active && (logical_i >= sel_start && logical_i < sel_end);
            // --- DEBUG VISUALIZATION ---
            if (debug_mode) {
                 if (i == buffer.gap_start) frame += "<";
                 if (i >= buffer.gap_start && i < buffer.gap_end) { frame += "_"; continue; }
                 if (i == buffer.gap_end) frame += ">";
            }

            // --- HIGHLIGHTING ---
            
            // if (is_highlighted) frame += "\033[7m";

            // --- DRAW CHARACTER ---
            char c = buffer.data[i];
            bool visible = (row >= start_row) && (row < start_row + TER_END);

            if (visible) {
                if (is_highlighted) frame += "\033[7m"; // Start Highlight
                
                if (c == '\n') frame += "\033[K"; 
                frame += c;
                
                if (is_highlighted) frame += "\033[0m"; // Reset Highlight
            }
            
            if (c == '\n') {
                row++;
                col = 0;
            } else if (c == '\t') {
                col += (8 - (col % 8));
            } else {
                col++;
            }
            
            // Optimization: If we went past the bottom of the screen, stop.
            // (But only if we already found the cursor!)
            if (row >= start_row + TER_END && i > buffer.gap_start) {
                break; 
            }   
        }

        // Clear everything
        frame += "\033[0J";

        // Cursor movement
        frame += "\033[" + to_string(cursor_r + 1) + ";" + to_string(cursor_c + 1) + "H";

        // OUTPUT CALL
        cout << frame << flush;
    }
    
}
