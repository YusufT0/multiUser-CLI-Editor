#include <iostream>
#include <unistd.h>
#include "buffer_service.hpp"
#include "selection_service.hpp"
#include "editor.hpp"
#include "file_io.hpp"
#include "clipboard.hpp"
#include "terminal_manager.hpp"
#include "view_service.hpp"
using namespace std;

Editor::Editor(const std::string& p) {
    path = p;
    gap_buffer = load_file(path);
    DEBUG_GAP = false;
}

void Editor::start_writing() {
    enableRawMode();
    ViewService::print_buffer(gap_buffer, highligter, DEBUG_GAP);
    while (true) {
        BufferService::update_gap_buffer(gap_buffer, path, highligter);
        ViewService::print_buffer(gap_buffer, highligter, DEBUG_GAP);
    }
}

void Editor::process_input() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return;

    if ((unsigned char)c >= 0xC2) {
        char throwaway;
        read(STDIN_FILENO, &throwaway, 1);
        return;
    }

    if (c == 27) { // ESC
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
        if (seq[0] == '[') {
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return;

            // Arrows (Move & Clear Selection)
            if (seq[1] == 'A') { SelectionService::clear(highligter); BufferService::move_cursor_up(gap_buffer);    return; }
            if (seq[1] == 'B') { SelectionService::clear(highligter); BufferService::move_cursor_down(gap_buffer);  return; }
            if (seq[1] == 'C') { SelectionService::clear(highligter); BufferService::move_cursor_right(gap_buffer); return; }
            if (seq[1] == 'D') { SelectionService::clear(highligter); BufferService::move_cursor_left(gap_buffer);  return; }

            // Shift+Arrows (Move & Update Selection)
            if (seq[1] == '1') {
                char dummy;
                read(STDIN_FILENO, &dummy, 1); read(STDIN_FILENO, &dummy, 1);
                char dir;
                if (read(STDIN_FILENO, &dir, 1) != 1) return;

                if (!highligter.active) SelectionService::start(highligter, gap_buffer);

                if (dir == 'A') BufferService::move_cursor_up(gap_buffer);
                if (dir == 'B') BufferService::move_cursor_down(gap_buffer);
                if (dir == 'C') BufferService::move_cursor_right(gap_buffer);
                if (dir == 'D') BufferService::move_cursor_left(gap_buffer);
                
                SelectionService::update_endpoint(highligter, gap_buffer);
                return;
            }
        }
        SelectionService::clear(highligter);
        return;
    }

    if (c == 127 || c == '\b') {
        if (gap_buffer.gap_start > 0 && !highligter.active) {
            gap_buffer.gap_start--; // Or BufferService::delete_backspace(gap_buffer)
        }
        return;
    }

    // CTRL+Q (Quit)
    if (c == 17) {
        save_file(gap_buffer, path); // Access 'path' member directly
        exit(0);
    }

    // CTRL+C (Copy)
    if (c == 3) {
        if (highligter.active) {
            string out;
            int start = min(highligter.start, highligter.end);
            int end   = max(highligter.start, highligter.end);
            for (int i = start; i < end; i++) {
                if (i >= gap_buffer.gap_start && i < gap_buffer.gap_end) continue;
                out.push_back(gap_buffer.data[i]);
            }
            clipboard = out;
        }
        return;
    }

    // CTRL+V (Paste)
    if (c == 22) {
        for (char clip_c : clipboard) {
            BufferService::insert_char(gap_buffer, clip_c);
        }
        return;
    }

    BufferService::insert_char(gap_buffer, c);
}