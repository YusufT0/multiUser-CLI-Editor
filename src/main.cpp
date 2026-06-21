#include "editor.hpp"
#include <iostream>
using namespace std;
int main(int argc, char **argv) {
  if (argc >= 2 && (string(argv[1]) == "--help" || string(argv[1]) == "-h")) {
    cout << "Usage: ./editor <filename>\n";
    cout << "Shortcuts:\n";
    cout << "  Ctrl+Q          Save file and quit\n";
    cout << "  Ctrl+S          Save file\n";
    cout << "  Ctrl+N          Change file (opens input modal)\n";
    cout << "  Ctrl+C          Copy selected text\n";
    cout << "  Ctrl+V          Paste from clipboard\n";
    cout << "  Arrows          Move cursor\n";
    cout << "  Ctrl+Left/Right Move word left/right\n";
    cout << "  Shift+Arrows    Select text\n";
    cout << "  Backspace       Delete character before cursor\n";
    cout << "  Escape          Cancel modal / dismiss\n";
    return 0;
  }
  if (argc < 2) {
    cout << "Usage: ./editor <filename>.\n If you need help to understand the"
            "shorcuts --help.\n";
    return 1;
  }

  string file_name = "";
  try {
    Editor &editor = Editor::get_instance();
    file_name = argv[1];
    editor.init(file_name);
    editor.start_writing();

  } catch (const std::runtime_error &e) {
    cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
