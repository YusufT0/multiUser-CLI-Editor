#include "clipboard.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <array>
#include <string>

// Detect OS (Simple check)
#ifdef __APPLE__
    const char* COPY_CMD = "pbcopy";
    const char* PASTE_CMD = "pbpaste";
#elif defined(_WIN32)
    const char* COPY_CMD = "clip"; // Windows only has easy Copy, Paste is hard in C++ without APIs
    const char* PASTE_CMD = "powershell.exe -command Get-Clipboard";
#else 
    // LINUX (Try xclip for X11, or change to wl-copy for Wayland)
    const char* COPY_CMD = "wl-copy";
    const char* PASTE_CMD = "wl-paste --no-newline";
#endif

namespace ClipboardService {

    void copy(const std::string& text) {
        // 1. Open a pipe to the system command in WRITE mode ("w")
        FILE* pipe = popen(COPY_CMD, "w");
        if (!pipe) {
            std::cerr << "Failed to open clipboard pipe!\n";
            return;
        }

        // 2. Write our text into that pipe (Standard Input of the tool)
        fwrite(text.c_str(), 1, text.size(), pipe);

        // 3. Close the pipe to signal "End of Input"
        pclose(pipe);
    }

    std::string paste() {
        // 1. Open a pipe to the system command in READ mode ("r")
        FILE* pipe = popen(PASTE_CMD, "r");
        if (!pipe) {
            std::cerr << "Failed to open clipboard pipe!\n";
            return "";
        }

        // 2. Read the output (Standard Output of the tool)
        std::array<char, 128> buffer;
        std::string result;
        
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

        // 3. Close
        pclose(pipe);
        return result;
    }
}