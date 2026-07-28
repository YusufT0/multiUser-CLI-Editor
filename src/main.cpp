#include "editor.hpp"
#include <iostream>
using namespace std;
int main(int argc, char **argv) {
  if (argc >= 2 && (string(argv[1]) == "--help" || string(argv[1]) == "-h")) {
    cout << "Usage:\n";
    cout << "  ./editor <filename>                      Single-user mode\n";
    cout << "  ./editor <filename> --host <port>        Host mode\n";
    cout << "  ./editor --client <ip>:<port>            Client mode\n";
    cout << "\nShortcuts:\n";
    cout << "  Ctrl+Q          Save and quit (host) / Quit (client)\n";
    cout << "  Ctrl+C          Copy selected text\n";
    cout << "  Ctrl+V          Paste from clipboard\n";
    cout << "  Ctrl+N          Change file (host only)\n";
    cout << "  Arrows          Move cursor\n";
    cout << "  Ctrl+Arrows     Move word left/right\n";
    cout << "  Shift+Arrows    Select text\n";
    cout << "  Backspace       Delete character before cursor\n";
    cout << "  Escape          Cancel modal\n";
    return 0;
  }

  try {
    Editor &editor = Editor::get_instance();

    // Check for client mode
    for (int i = 1; i < argc; i++) {
      if (string(argv[i]) == "--client" && i + 1 < argc) {
        string target = argv[i + 1];
        // Parse ip:port
        size_t colon = target.rfind(':');
        if (colon == string::npos) {
          cerr << "Error: --client requires ip:port (e.g. 127.0.0.1:1313)\n";
          return 1;
        }
        string host = target.substr(0, colon);
        int port = stoi(target.substr(colon + 1));
        editor.init("");
        editor.start_client(host, port);
        editor.start_writing();
        return 0;
      }
    }

    // Check for host mode
    bool host_mode = false;
    int port = 0;
    string filename;
    for (int i = 1; i < argc; i++) {
      if (string(argv[i]) == "--host" && i + 1 < argc) {
        host_mode = true;
        port = stoi(argv[i + 1]);
        i++;
      } else if (argv[i][0] != '-') {
        filename = argv[i];
      }
    }

    if (filename.empty()) {
      cerr << "Error: filename required for host mode.\n";
      return 1;
    }

    editor.init(filename);
    if (host_mode) {
      editor.start_host(port);
    }
    editor.start_writing();

  } catch (const std::exception &e) {
    cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
