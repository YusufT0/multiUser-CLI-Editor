#pragma once
#include <string>
#include "gap_buffer.hpp"
void load_file(GapBuffer&, const std::string&);
void save_file(const GapBuffer&, const std::string&);