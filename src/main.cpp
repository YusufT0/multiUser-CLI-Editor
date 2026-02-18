#include "terminal_manager.hpp"
#include "buffer_service.hpp"
#include "file_io.hpp"
#include "clipboard.hpp"
#include "editor.hpp"
#include <iostream>
using namespace std;
int main(int argc, char**argv){
    // if (argc < 2) {
    //     cout << "Usage: ./editor <filename>\n";
    //     return 1;
    // }
    string file_name = "./test.txt";
    try {
        Editor editor(file_name);
        editor.start_writing();
    } catch (const std::runtime_error& e) {
        cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
