#include "gap_buffer.hpp"
#include "file_io.hpp"
#include "clipboard.hpp"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

bool DEBUG_GAP = false;   
GapBuffer create_gap_buffer(){
    GapBuffer buffer;
    buffer.data.resize(10);
    buffer.gap_start = 0;
    buffer.gap_end = 10;
    return buffer;
};

CursorPos get_cursor_pos(const GapBuffer& buffer) {
    int row = 0;
    int col = 0;

    for (int i = 0; i < buffer.gap_start; i++) {
        if (buffer.data[i] == '\n') {
            row++;
            col = 0;
        } 
        else if (buffer.data[i] == '\t'){
            col = (col/8 + 1) * 8;
        }
        else {
            col++;
        }
    }

    return {row, col};
}


Highlight create_highlight(){
    Highlight hl;
    hl.active = false;
    hl.start = 0;
    hl.end = 0;

    return hl;
}

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

void move_cursor_up(GapBuffer &b){
    
    CursorPos pos = get_cursor_pos(b);
    int target_col = pos.col;
    if (b.gap_start == 0) return;
    
    while (b.gap_start > 0 && b.data[b.gap_start - 1] != '\n')
        move_cursor_left(b);
    
    if (b.gap_start == 0) return;
    
    move_cursor_left(b);
    
    // We need to go back to the start of the pervious line because it is dumb. 
    // It being dumb is the reason we are doing that. Totally.
    // Jk the reason is we just know the length from the start. We don't know the length from column to end.
    while (b.gap_start > 0 && b.data[b.gap_start - 1] != '\n')
        move_cursor_left(b);
    
    int col = 0;
    while (col < target_col && b.gap_end < b.data.size() && b.data[b.gap_end] != '\n') {
        move_cursor_right(b);
        col++;
    }
}

void move_cursor_down(GapBuffer &b){
    CursorPos pos = get_cursor_pos(b);
    int target_col = pos.col;
    
    while (b.gap_end < b.data.size()){
        if (b.data[b.gap_end] == '\n') break;
        move_cursor_right(b);
    }

    if (b.gap_end < b.data.size()) move_cursor_right(b);
    else return;

    int col = 0;
    while (col < target_col && b.gap_end < b.data.size() && b.data[b.gap_end] != '\n') {
        move_cursor_right(b);
        col++;
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

void print_buffer(const GapBuffer &buffer, Highlight &hl) {
    cout << "\033[2J\033[H";

    int sel_start = hl.active ? hl.start : -1;
    int sel_end   = hl.active ? hl.end   : -1;

    for (int i = 0; i < buffer.data.size(); i++) {

        if (DEBUG_GAP) {
            if (i == buffer.gap_start)
                cout << "<";
            if (i >= buffer.gap_start && i < buffer.gap_end) {
                cout << "_";
                continue;
            }
            if (i == buffer.gap_end)
                cout << ">";
        }

        if (i >= buffer.gap_start && i < buffer.gap_end)
            continue;

        bool highlighted = hl.active && (i >= sel_start && i < sel_end);

        if (highlighted)
            cout << "\033[7m";

        cout << buffer.data[i];

        if (highlighted)
            cout << "\033[0m";
    }

    CursorPos pos = get_cursor_pos(buffer);
    cout << "\033[" << pos.row+1 << ";" << pos.col+1 << "H" << flush;
}


void insert_char(GapBuffer &buffer, char c){
    
    if (buffer.gap_start == buffer.gap_end) {
        grow_gap(buffer);
    }
    
    if (c == '\n' || c == '\r') {
        buffer.data[buffer.gap_start] = '\n';
        buffer.gap_start++;
        return;
    }

    buffer.data[buffer.gap_start] = c;
    buffer.gap_start++;
}

void clear_highlight(Highlight &hl) {
    hl.active = false;
    hl.start = 0;
    hl.end = 0;
}

void begin_highlight(const GapBuffer &buffer, Highlight &hl) {
    hl.active = true;
    hl.start = buffer.gap_start;
    hl.end   = buffer.gap_start;
}

void extend_selection_right(GapBuffer &buffer, Highlight &hl){
    move_cursor_right(buffer);         // updates gap_start
    hl.end = buffer.gap_start;         // selection end = cursor
}

void extend_selection_left(GapBuffer &buffer, Highlight &hl){
    move_cursor_left(buffer);
    hl.end = buffer.gap_start;
}

void extend_selection_up(GapBuffer &buffer, Highlight &hl){
    move_cursor_up(buffer);
    hl.end = buffer.gap_start;
}

void extend_selection_down(GapBuffer &buffer, Highlight &hl){
    move_cursor_down(buffer);
    hl.end = buffer.gap_start;
}



void update_gap_buffer(GapBuffer &buffer, const std::string &filename, Highlight &hl) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return;

    unsigned char uc = (unsigned char)c;

    if (uc >= 0xC2) {
        char throwaway;
        read(STDIN_FILENO, &throwaway, 1);
        return;  
    }

    // ESC / arrows / shift+arrows
    if (c == 27) { // ESC
        char seq1;
        if (read(STDIN_FILENO, &seq1, 1) != 1) return;

        if (seq1 == '[') {
            char seq2;
            if (read(STDIN_FILENO, &seq2, 1) != 1) return;

            if (seq2 == 'A') {
                clear_highlight(hl);
                move_cursor_up(buffer);
                return;
            }
            if (seq2 == 'B') {
                clear_highlight(hl);
                move_cursor_down(buffer);
                return;
            }
            if (seq2 == 'C') {
                clear_highlight(hl);
                move_cursor_right(buffer);
                return;
            }
            if (seq2 == 'D') {
                clear_highlight(hl);
                move_cursor_left(buffer);
                return;
            }

            if (seq2 == '1') {
                char semicolon, mod, dir;
                if (read(STDIN_FILENO, &semicolon, 1) != 1) return;
                if (read(STDIN_FILENO, &mod, 1) != 1) return;
                if (read(STDIN_FILENO, &dir, 1) != 1) return;

                if (mod == '2') {
                    if (!hl.active) {
                        begin_highlight(buffer, hl);
                    }

                    if (dir == 'A') { extend_selection_up(buffer, hl);    return; }
                    if (dir == 'B') { extend_selection_down(buffer, hl);  return; }
                    if (dir == 'C') { extend_selection_right(buffer, hl); return; }
                    if (dir == 'D') { extend_selection_left(buffer, hl);  return; }
                }

                clear_highlight(hl);
                return;
            }
        }

        clear_highlight(hl);
        return;
    }

    if (c == 127 || c == '\b') {

        if (buffer.gap_start > 0 && !hl.active) {
            buffer.gap_start--;
        }
        return;
    }

    // CTRL+Q
    if (c == 17) {
        save_file(buffer, filename);
        exit(0);
    }

    if (c == 3){
        if(hl.active){
            string out;
            for (int i = hl.start; i < hl.end; i++) {
                if (i >= buffer.gap_start && i < buffer.gap_end) continue;
                out.push_back(buffer.data[i]);
            }
            
            clipboard = out;
        }
        return;
    }
    if (c == 22){
        for (char c : clipboard){
            insert_char(buffer, c);
        }
        return;
    }

    // if (c == 9){
    //     insert_char(buffer, '\t');
    //     return;
    // }

    insert_char(buffer, c);
}

