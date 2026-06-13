#pragma once
#include "models.hpp"

namespace ViewService {
const int TER_END_X = 60;
const int TER_END_Y = 20;
void print_buffer(const GapBuffer &buffer, const Highlight &hl,
                  bool debug_mode = false);
} // namespace ViewService
