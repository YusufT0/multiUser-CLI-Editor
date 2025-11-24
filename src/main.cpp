#include "terminal.hpp"
#include "gap_buffer.hpp"
#include "file_io.hpp"
#include "clipboard.hpp"
#include <iostream>
using namespace std;
int main(int argc, char**argv){
    if (argc < 2) {
        cout << "Usage: ./editor <filename>\n";
        return 1;
    }
    GapBuffer gb = create_gap_buffer();
    Highlight hl = create_highlight();
    load_file(gb, argv[1]);
    CursorPos curp = {0, 0};
    enableRawMode();
    print_buffer(gb, hl);
    while (true) {
        update_gap_buffer(gb, argv[1], hl);
        print_buffer(gb, hl);
    }
    disableRawMode();
    return 0;
}
