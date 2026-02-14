#include "buffer_service.hpp"
#include "models.hpp"
#include "file_io.hpp"
#include "clipboard.hpp"
#include "selection_service.hpp"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

namespace BufferService{

    GapBuffer create_gap_buffer(){
    GapBuffer buffer;
    buffer.data.resize(1024);
    buffer.gap_start = 0;
    buffer.gap_end = 1024;
    return buffer;
};
void move_cursor_left(GapBuffer &b) {
    if (b.gap_start == 0) return;

    b.gap_start--;
    b.gap_end--;
    b.data[b.gap_end] = b.data[b.gap_start];
}

void move_cursor_right(GapBuffer &b) {
    if (b.gap_end == b.data.size()) return;

    b.data[b.gap_start] = b.data[b.gap_end];
    b.gap_start++;
    b.gap_end++;
}

static int get_col_on_line(const GapBuffer &b) {
    int col = 0;
    // Scan backwards from gap_start until we hit a newline or start of file
    for (int i = b.gap_start - 1; i >= 0; i--) {
        if (b.data[i] == '\n') break;
        
        // Handle tabs if you want perfect alignment, otherwise just col++
        if (b.data[i] == '\t') 
                col = (col / 8 + 1) * 8; 
        else 
                col++;
    }
    return col;
}

// Now this function is pure logic. No "CursorPos" struct needed.
void move_cursor_up(GapBuffer &b) {
    if (b.gap_start == 0) return;

    // 1. Calculate target column locally
    int target_col = get_col_on_line(b); 

    // 2. Move to start of current line (Logic from your loop, but cleaner)
    while (b.gap_start > 0 && b.data[b.gap_start - 1] != '\n') {
        move_cursor_left(b);
    }
    
    // 3. Move past the newline to go to the previous line
    if (b.gap_start > 0) move_cursor_left(b);

    // 4. Move to start of THAT previous line
    while (b.gap_start > 0 && b.data[b.gap_start - 1] != '\n') {
        move_cursor_left(b);
    }

    // 5. Move forward to the target column
    int current_col = 0;
    while (current_col < target_col && b.gap_end < b.data.size() && b.data[b.gap_end] != '\n') {
        move_cursor_right(b);
        current_col++; 
    }
}

void move_cursor_down(GapBuffer &b) {
    int target_col = get_col_on_line(b);

    // 1. Move to end of current line
    while (b.gap_end < b.data.size() && b.data[b.gap_end] != '\n') {
        move_cursor_right(b);
    }

    // 2. Jump over the newline
    if (b.gap_end < b.data.size()) move_cursor_right(b);
    else return; // End of file

    // 3. Move forward to target column on the new line
    int current_col = 0;
    while (current_col < target_col && b.gap_end < b.data.size() && b.data[b.gap_end] != '\n') {
        move_cursor_right(b);
        current_col++;
    }
}

void grow_gap(GapBuffer &buffer, size_t amount) {
    size_t old_size = buffer.data.size();
    size_t gap_size = amount;

    buffer.data.resize(old_size + gap_size);

    size_t right_len = old_size - buffer.gap_end;

    for (size_t i = right_len; i-- > 0; ) {
        // cout << "Moving " << buffer.data[buffer.gap_end + i] << " from: " << buffer.gap_end + i << " to: " << buffer.gap_end + gap_size + i;
        // cout << endl;
        buffer.data[buffer.gap_end + gap_size + i] =
            buffer.data[buffer.gap_end + i];
    }

    buffer.gap_end += gap_size;
}

// void print_buffer(const GapBuffer &buffer, Highlight &hl) {
//     cout << "\033[2J\033[H";

//     int sel_start = hl.active ? hl.start : -1;
//     int sel_end   = hl.active ? hl.end   : -1;

//     for (int i = 0; i < buffer.data.size(); i++) {

//         if (DEBUG_GAP) {
//             if (i == buffer.gap_start)
//                 cout << "<";
//             if (i >= buffer.gap_start && i < buffer.gap_end) {
//                 cout << "_";
//                 continue;
//             }
//             if (i == buffer.gap_end)
//                 cout << ">";
//         }

//         if (i >= buffer.gap_start && i < buffer.gap_end)
//             continue;

//         bool highlighted = hl.active && (i >= sel_start && i < sel_end);

//         if (highlighted)
//             cout << "\033[7m";

//         cout << buffer.data[i];

//         if (highlighted)
//             cout << "\033[0m";
//     }

//     CursorPos pos = get_cursor_pos(buffer);
//     cout << "\033[" << pos.row+1 << ";" << pos.col+1 << "H" << flush;
// }


void insert_char(GapBuffer &buffer, char c){
    
    if (buffer.gap_start == buffer.gap_end) {
        grow_gap(buffer, 1024);
    }
    
    if (c == '\n' || c == '\r') {
        buffer.data[buffer.gap_start] = '\n';
        buffer.gap_start++;
        return;
    }

    buffer.data[buffer.gap_start] = c;
    buffer.gap_start++;
}

// void clear_highlight(Highlight &hl) {
//     hl.active = false;
//     hl.start = 0;
//     hl.end = 0;
// }

// void begin_highlight(const GapBuffer &buffer, Highlight &hl) {
//     hl.active = true;
//     hl.start = buffer.gap_start;
//     hl.end   = buffer.gap_start;
// }

// void extend_selection_right(GapBuffer &buffer, Highlight &hl){
//     move_cursor_right(buffer);         // updates gap_start
//     hl.end = buffer.gap_start;         // selection end = cursor
// }

// void extend_selection_left(GapBuffer &buffer, Highlight &hl){
//     move_cursor_left(buffer);
//     hl.end = buffer.gap_start;
// }

// void extend_selection_up(GapBuffer &buffer, Highlight &hl){
//     move_cursor_up(buffer);
//     hl.end = buffer.gap_start;
// }

// void extend_selection_down(GapBuffer &buffer, Highlight &hl){
//     move_cursor_down(buffer);
//     hl.end = buffer.gap_start;
// }



// Ensure you have #include "selection_service.hpp" at the top of this file

// void update_gap_buffer(GapBuffer &buffer, const std::string &filename, Highlight &hl) {
//     char c;
//     if (read(STDIN_FILENO, &c, 1) != 1) return;

//     unsigned char uc = (unsigned char)c;

//     if (uc >= 0xC2) {
//         char throwaway;
//         read(STDIN_FILENO, &throwaway, 1);
//         return;  
//     }

//     // ESC / arrows / shift+arrows
//     if (c == 27) { // ESC
//         char seq1;
//         if (read(STDIN_FILENO, &seq1, 1) != 1) return;

//         if (seq1 == '[') {
//             char seq2;
//             if (read(STDIN_FILENO, &seq2, 1) != 1) return;

//             // --- STANDARD ARROWS (Clear selection, then move) ---
//             if (seq2 == 'A') {
//                 SelectionService::clear(hl);
//                 move_cursor_up(buffer);
//                 return;
//             }
//             if (seq2 == 'B') {
//                 SelectionService::clear(hl);
//                 move_cursor_down(buffer);
//                 return;
//             }
//             if (seq2 == 'C') {
//                 SelectionService::clear(hl);
//                 move_cursor_right(buffer);
//                 return;
//             }
//             if (seq2 == 'D') {
//                 SelectionService::clear(hl);
//                 move_cursor_left(buffer);
//                 return;
//             }

//             // --- SHIFT + ARROWS (Selection Logic) ---
//             if (seq2 == '1') {
//                 char semicolon, mod, dir;
//                 if (read(STDIN_FILENO, &semicolon, 1) != 1) return;
//                 if (read(STDIN_FILENO, &mod, 1) != 1) return;
//                 if (read(STDIN_FILENO, &dir, 1) != 1) return;

//                 if (mod == '2') {
//                     // 1. Start selection if needed
//                     if (!hl.active) {
//                         SelectionService::start(hl, buffer);
//                     }

//                     // 2. Move Cursor & Update Endpoint
//                     if (dir == 'A') { 
//                         move_cursor_up(buffer);    
//                         SelectionService::update_endpoint(hl, buffer); 
//                         return; 
//                     }
//                     if (dir == 'B') { 
//                         move_cursor_down(buffer);  
//                         SelectionService::update_endpoint(hl, buffer); 
//                         return; 
//                     }
//                     if (dir == 'C') { 
//                         move_cursor_right(buffer); 
//                         SelectionService::update_endpoint(hl, buffer); 
//                         return; 
//                     }
//                     if (dir == 'D') { 
//                         move_cursor_left(buffer);  
//                         SelectionService::update_endpoint(hl, buffer); 
//                         return; 
//                     }
//                 }

//                 SelectionService::clear(hl);
//                 return;
//             }
//         }

//         SelectionService::clear(hl);
//         return;
//     }

//     if (c == 127 || c == '\b') {
//         if (buffer.gap_start > 0 && !hl.active) {
//             buffer.gap_start--;
//         }
//         return;
//     }

//     // CTRL+Q
//     if (c == 17) {
//         save_file(buffer, filename);
//         exit(0);
//     }

//     // CTRL+C
//     if (c == 3){
//         if(hl.active){
//             string out;
//             // You can keep this manual loop or move it to SelectionService later
//             for (int i = hl.start; i < hl.end; i++) {
//                 if (i >= buffer.gap_start && i < buffer.gap_end) continue;
//                 out.push_back(buffer.data[i]);
//             }
//             clipboard = out;
//         }
//         return;
//     }

//     // CTRL+V
//     if (c == 22){
//         for (char c : clipboard){
//             insert_char(buffer, c);
//         }
//         return;
//     }

//     insert_char(buffer, c);
// }
}
