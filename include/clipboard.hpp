#pragma once
#include <string>

namespace ClipboardService {
    // Send text to the OS clipboard
    void copy(const std::string& text);

    // Get text from the OS clipboard
    std::string paste();
}