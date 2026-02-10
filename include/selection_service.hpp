#pragma once
#include "models.hpp"

namespace SelectionService {
    void clear(Highlight &hl);

    void start(Highlight &hl, const GapBuffer &buffer);

    void update_endpoint(Highlight &hl, const GapBuffer &buffer);
}