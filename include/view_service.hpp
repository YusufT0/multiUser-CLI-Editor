#pragma once
#include "models.hpp"

namespace ViewService {
    const int TER_END = 20;
    void print_buffer(const GapBuffer& buffer, const Highlight& hl, bool debug_mode = false);
}