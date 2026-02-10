#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include "buffer_service.hpp"
#include "models.hpp"

class Editor {
private:
    std::string path;
    GapBuffer gap_buffer;
    Highlight highligter;
    bool DEBUG_GAP;
    void process_input();

public:
    Editor(const std::string& p);
    void start_writing();
};

#endif
