#include "terminal_manager.hpp"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios orig;
namespace TerminalManager
{
    void disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    }

    void enableRawMode() {
        tcgetattr(STDIN_FILENO, &orig);
        atexit(disableRawMode);

        struct termios raw = orig;
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    
    InputEvent read_input() {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) return {Key::None};

        // 1. Handle Regular Characters
        if (c == 127 || c == '\b') return {Key::Backspace};
        if (c == 13 || c == '\n')  return {Key::Enter, '\n'};
        if (c == 17) return {Key::Quit};  // Ctrl+Q
        if (c == 3)  return {Key::Copy};  // Ctrl+C
        if (c == 22) return {Key::Paste}; // Ctrl+V
        if (c == 19) return {Key::Save};  // Ctrl+S (Optional)

        // 2. Handle Escape Sequences (Arrows, Shift+Arrows)
        if (c == 27) {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return {Key::Escape};
            
            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) != 1) return {Key::Escape};

                // Standard Arrows
                if (seq[1] == 'A') return {Key::Up};
                if (seq[1] == 'B') return {Key::Down};
                if (seq[1] == 'C') return {Key::Right};
                if (seq[1] == 'D') return {Key::Left};

                // Shift + Arrows (1;2A)
                if (seq[1] == '1') {
                    char dummy;
                    read(STDIN_FILENO, &dummy, 1); // ;
                    read(STDIN_FILENO, &dummy, 1); // 2
                    char dir;
                    read(STDIN_FILENO, &dir, 1);   // Direction

                    if (dir == 'A') return {Key::Up, 0, true};
                    if (dir == 'B') return {Key::Down, 0, true};
                    if (dir == 'C') return {Key::Right, 0, true};
                    if (dir == 'D') return {Key::Left, 0, true};
                }
            }
            return {Key::Escape};
        }

        // 3. Skip UTF-8 (Your logic)
        if ((unsigned char)c >= 0xC2) {
            char throwaway;
            read(STDIN_FILENO, &throwaway, 1);
            return {Key::None};
        }

        // 4. Default: It's a character
        return {Key::Char, c};
    }
}
