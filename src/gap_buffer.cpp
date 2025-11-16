#include "gap_buffer.hpp"
#include "file_io.hpp"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;


GapBuffer create_gap_buffer(){
    GapBuffer buffer;
    buffer.data.resize(1024);
    buffer.gap_start = 0;
    buffer.gap_end = 1024;
    return buffer;
};

CursorPos get_cursor_pos(const GapBuffer& buffer) {
    int row = 0;
    int col = 0;

    for (int i = 0; i < buffer.gap_start; i++) {
        if (buffer.data[i] == '\n') {
            row++;
            col = 0;
        } else {
            col++;
        }
    }

    return {row, col};
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

void print_buffer(const GapBuffer &buffer) {
    
    cout << "\033[2J\033[H";

    for (int i = 0; i < buffer.gap_start; i++)
        cout << buffer.data[i];
    for (int i = buffer.gap_end; i < buffer.data.size(); i++)
        cout << buffer.data[i];

    
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

void update_gap_buffer(GapBuffer &buffer, const std::string &filename) {
    char c;
    read(STDIN_FILENO, &c, 1);


    if (c == 27) {  // ESC
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return;

        if (seq[0] == '[') {
            switch (seq[1]) {

                case 'A': // UP
                    move_cursor_up(buffer);
                    return;

                case 'B': // DOWN
                    move_cursor_down(buffer);
                    return;

                case 'D': // LEFT
                    move_cursor_left(buffer);
                    return;

                case 'C': // RIGHT
                    move_cursor_right(buffer);
                    return;
            }
        }
        return;
    }

    
    if (c == 127 || c == '\b') { // backspace
        if (buffer.gap_start > 0){
            buffer.gap_start--;
        }
        return;
    }
    if (c == 17){ // CTRL + Q
        save_file(buffer, filename);
        exit(0);
    }

    insert_char(buffer, c);
}


