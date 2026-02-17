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
        Key key = Key::None;
        char value = '\0';
        bool shift_held = false;
        bool ctrl_held = false;
    };
    InputEvent read_input();

}
