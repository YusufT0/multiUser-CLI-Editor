#include "terminal_manager.hpp"
#include <stdlib.h>


#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    
    static HANDLE hStdin;
    static DWORD originalMode;
    
    #ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
    #define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
    #endif

#else
    // LINUX & MAC OS
    #include <termios.h>
    #include <unistd.h>
    
    static struct termios orig;
#endif

namespace TerminalManager
{
    static bool read_byte(char& c) {
    #ifdef _WIN32
            // _read reads from file descriptor 0 (stdin)
            return _read(0, &c, 1) == 1; 
    #else
            return read(STDIN_FILENO, &c, 1) == 1;
    #endif
        }

    void disableRawMode() {
    #ifdef _WIN32
            SetConsoleMode(hStdin, originalMode);
    #else
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    #endif
        }

    void enableRawMode() {
        atexit(disableRawMode);

    #ifdef _WIN32
            hStdin = GetStdHandle(STD_INPUT_HANDLE);
            GetConsoleMode(hStdin, &originalMode);

            DWORD newMode = originalMode;
            
            newMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
            
            newMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

            SetConsoleMode(hStdin, newMode);
    #else
            
            tcgetattr(STDIN_FILENO, &orig);
            struct termios raw = orig;
            
            raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
            
            raw.c_iflag &= ~(IXON | ICRNL);
            
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    #endif
        }
        
    InputEvent read_input() {
        char c;
        if (!read_byte(c)) return {Key::None};

        if (c == 127 || c == '\b') return {Key::Backspace};
        if (c == 13 || c == '\n')  return {Key::Enter, '\n'}; // Windows often sends 13 (\r)
        if (c == 17) return {Key::Quit};  // Ctrl+Q
        if (c == 3)  return {Key::Copy};  // Ctrl+C (Raw)
        if (c == 22) return {Key::Paste}; // Ctrl+V
        if (c == 19) return {Key::Save};  // Ctrl+S

        
        if (c == 27) { // \x1b
            char seq[3];
            
            if (!read_byte(seq[0])) return {Key::Escape};
            
            if (seq[0] == '[') {
                if (!read_byte(seq[1])) return {Key::Escape};

                // Standard Arrows
                if (seq[1] == 'A') return {Key::Up};
                if (seq[1] == 'B') return {Key::Down};
                if (seq[1] == 'C') return {Key::Right};
                if (seq[1] == 'D') return {Key::Left};

                // (Shift/Ctrl + Arrow)"
                if (seq[1] == '1') {
                    char sep, mod, dir;

                    if (!read_byte(sep)) return {Key::Escape}; // Expect ';'
                    if (!read_byte(mod)) return {Key::Escape}; // Expect Modifier
                    if (!read_byte(dir)) return {Key::Escape}; // Expect Direction

                    bool is_shift = (mod == '2' || mod == '6');
                    bool is_ctrl  = (mod == '5' || mod == '6');

                    Key k = Key::None;
                    if (dir == 'A') k = Key::Up;
                    if (dir == 'B') k = Key::Down;
                    if (dir == 'C') k = Key::Right;
                    if (dir == 'D') k = Key::Left;

                    return {k, 0, is_shift, is_ctrl};
                }
            }
            return {Key::Escape};
        }

        // Skip UTF-8 Continuation
        if ((unsigned char)c >= 0xC2) {
            char throwaway;
            read_byte(throwaway); 
            return {Key::None};
        }

        return {Key::Char, c};
    }
}
