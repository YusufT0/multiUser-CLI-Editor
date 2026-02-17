#include "clipboard.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <array>
#include <string>


namespace ClipboardService {

    void copy(const std::string& text) {
        auto [copy_cmd, _] = get_os_cmds();

        FILE* pipe = open_pipe(copy_cmd, "w");
        if (!pipe) {
            std::cerr << "Failed to open clipboard pipe!\n";
            return;
        }

        fwrite(text.c_str(), 1, text.size(), pipe);
        
        close_pipe(pipe);
    }

    std::string paste() {
        auto [_, paste_cmd] = get_os_cmds();

        FILE* pipe = open_pipe(paste_cmd, "r");
        if (!pipe) {
            std::cerr << "Failed to open clipboard pipe!\n";
            return "";
        }

        std::array<char, 128> buffer;
        std::string result;
        
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

        close_pipe(pipe);
        return result;
    }
}