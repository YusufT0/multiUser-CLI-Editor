#pragma once
#include "terminal_manager.hpp"
#include "models.hpp"
#include <memory>

class Editor;

class InputHandler {
public:
    virtual ~InputHandler() = default;
    virtual void handle(Editor &editor, const TerminalManager::InputEvent &e) = 0;
};

std::unique_ptr<InputHandler> create_client_handler();
std::unique_ptr<InputHandler> create_host_handler();
std::unique_ptr<InputHandler> create_offline_handler();