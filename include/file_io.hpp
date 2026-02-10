#pragma once
#include <string>
#include "models.hpp"

GapBuffer load_file(const std::string&);
void save_file(const GapBuffer&, const std::string&);