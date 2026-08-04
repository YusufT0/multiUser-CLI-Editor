#pragma once
#include "terminal_manager.hpp"
#include "models.hpp"

class Editor;

class InputHandler {
public:
    virtual ~InputHandler() = default;
    virtual void handle(Editor &editor, const TerminalManager::InputEvent &e) = 0;
};

InputHandler *create_client_handler();
InputHandler *create_host_handler();
InputHandler *create_offline_handler();