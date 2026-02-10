#pragma once

namespace TerminalManager{
    void enableRawMode();
    void disableRawMode();
    enum class Key {
        None,
        Char,       // Regular typing (a, b, 1, !)
        Up, Down, Left, Right,
        Backspace,
        Enter,
        Escape,
        Copy, Paste, Quit, Save
    };
    struct InputEvent {
        Key key;
        char value;      // Only used if key == Key::Char
        bool shift_held; // True if Shift was held (for selection)
    };
    InputEvent read_input();

}
