#include "selection_service.hpp"

namespace SelectionService {

    Highlight create_highlight(){
        Highlight hl;
        hl.active = false;
        hl.start = 0;
        hl.end = 0;

        return hl;
    }
    void clear(Highlight &hl) {
        hl.active = false;
        hl.start = 0;
        hl.end = 0;
    }

    void start(Highlight &hl, const GapBuffer &buffer) {
        hl.active = true;
        // The selection starts where the cursor (gap) currently is
        hl.start = buffer.gap_start; 
        hl.end   = buffer.gap_start;
    }

    void update_endpoint(Highlight &hl, const GapBuffer &buffer) {
        // If we are selecting, the end of the selection is always the cursor
        if (hl.active) {
            hl.end = buffer.gap_start;
        }
    }
}