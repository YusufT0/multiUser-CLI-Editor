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
    TerminalManager::enableRawMode();
    ViewService::print_buffer(gap_buffer, highligter, DEBUG_GAP);
    while (true) {
        // BufferService::update_gap_buffer(gap_buffer, path, highligter);
        process_input();
        ViewService::print_buffer(gap_buffer, highligter, DEBUG_GAP);
    }
}

using namespace TerminalManager;
void Editor::process_input() {
    InputEvent e = read_input();
    if (e.key == Key::None) return;

    // --- GLOBAL SELECTION LOGIC ---
    // If Shift is held, ensure we are in selection mode.
    if (e.shift_held && !highligter.active) {
        SelectionService::start(highligter, gap_buffer);
    }
    // If Shift is NOT held and we move, clear selection.
    // (Exceptions: Char insertion and Backspace handle their own logic)
    if (!e.shift_held && e.key != Key::Char && e.key != Key::Backspace && 
        e.key != Key::Copy && e.key != Key::Paste) {
        SelectionService::clear(highligter);
    }

    // --- COMMAND DISPATCH ---
    switch (e.key) {
        case Key::Up:    
            BufferService::move_cursor_up(gap_buffer);    
            break;
        case Key::Down:  
            BufferService::move_cursor_down(gap_buffer);  
            break;
        case Key::Left:  
            BufferService::move_cursor_left(gap_buffer);  
            break;
        case Key::Right: 
            BufferService::move_cursor_right(gap_buffer); 
            break;
            
        case Key::Backspace:
            // BufferService::delete_backspace(gap_buffer); // Implement this in buffer_service
            if (gap_buffer.gap_start > 0) gap_buffer.gap_start--; 
            break;
            
        case Key::Char:
        case Key::Enter: // Treated as char '\n'
            BufferService::insert_char(gap_buffer, e.value);
            break;

        case Key::Quit:
            save_file(gap_buffer, path);
            exit(0);

        case Key::Copy:
            // Logic to copy selection to clipboard
            break;

        case Key::Paste:
            for (char c : clipboard) BufferService::insert_char(gap_buffer, c);
            break;
            
        default: break;
    }

    // --- POST-MOVE UPDATE ---
    if (e.shift_held) {
        SelectionService::update_endpoint(highligter, gap_buffer);
    }
}