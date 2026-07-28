#include "terminal_manager.hpp"
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>

static HANDLE hStdin;
static DWORD originalMode;

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif

#else
// LINUX & MAC OS
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig;
#endif

namespace TerminalManager {
void getTerminalSize(int &width, int &height) {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // Fallback sizes if ioctl fails
    width = 80;
    height = 24;
  } else {
    width = ws.ws_col;
    height = ws.ws_row;
  }
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
InputEvent read_input_non_blocking() {
#ifdef _WIN32
  // Windows: use PeekNamedPipe or WaitForSingleObject with 0 timeout
  DWORD events;
  INPUT_RECORD rec;
  if (!GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &events) ||
      events == 0)
    return {Key::None};
  // Fall through to blocking read if there is something
  return read_input();
#else
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) <= 0)
    return {Key::None};
  return read_input();
#endif
}
InputEvent read_input() {
  char buf[8]; // This is 8 char because the longest sequence we can get is 8.
  int n;
#ifdef _WIN32
  n = _read(0, buf, sizeof(buf));
#else
  n = read(STDIN_FILENO, buf, sizeof(buf));
#endif
  if (n <= 0)
    return {Key::None};

  char c = buf[0];

  if (c == 127 || c == '\b')
    return {Key::Backspace};
  if (c == 13 || c == '\n')
    return {Key::Enter, '\n'};
  if (c == 17)
    return {Key::Quit};
  if (c == 3)
    return {Key::Copy};
  if (c == 22)
    return {Key::Paste};
  if (c == 24)
    return {Key::AqLock};
  if (c == 19)
    return {Key::Save};
  if (c == 14)
    return {Key::ChangeFile};

  if (c == 27) {
    if (n < 2)
      return {Key::Escape};

    if (buf[1] == '[') {
      if (n < 3)
        return {Key::Escape};
      char dir = buf[2];
      if (dir == 'A')
        return {Key::Up};
      if (dir == 'B')
        return {Key::Down};
      if (dir == 'C')
        return {Key::Right};
      if (dir == 'D')
        return {Key::Left};

      if (dir == '1' && n >= 6) {
        char mod = buf[4];
        char dir2 = buf[5];
        bool is_shift = (mod == '2' || mod == '6');
        bool is_ctrl = (mod == '5' || mod == '6');
        Key k = Key::None;
        if (dir2 == 'A')
          k = Key::Up;
        if (dir2 == 'B')
          k = Key::Down;
        if (dir2 == 'C')
          k = Key::Right;
        if (dir2 == 'D')
          k = Key::Left;
        return {k, 0, is_shift, is_ctrl};
      }
    }
    return {Key::Escape};
  }

  // Skip UTF-8 continuation (entire multi-byte consumed in the single read)
  if ((unsigned char)c >= 0xC2)
    return {Key::None};

  return {Key::Char, c};
}
} // namespace TerminalManager
