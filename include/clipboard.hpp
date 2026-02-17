#pragma once
#include <string>

namespace ClipboardService {
    // Send text to the OS clipboard
    void copy(const std::string& text);

    // Get text from the OS clipboard
    std::string paste();
}

// THIS IS GARBAGE.

inline FILE* open_pipe(const char* cmd, const char* mode) {
#ifdef _WIN32
    return _popen(cmd, mode);
#else
    return popen(cmd, mode);
#endif
}

inline int close_pipe(FILE* stream) {
#ifdef _WIN32
    return _pclose(stream);
#else
    return pclose(stream);
#endif
}

inline std::pair<const char*, const char*> get_os_cmds() {
#ifdef __APPLE__
    return {"pbcopy", "pbpaste"};
#elif defined(_WIN32)
    return {"clip", "powershell.exe -command Get-Clipboard"};
#else
    static const char* LINUX_COPY = 
        "wl-copy 2>/dev/null || xsel --clipboard --input 2>/dev/null || xclip -selection clipboard";
    
    static const char* LINUX_PASTE = 
        "wl-paste --no-newline 2>/dev/null || xsel --clipboard --output 2>/dev/null || xclip -selection clipboard -o";
    return {LINUX_COPY, LINUX_PASTE};
#endif
}