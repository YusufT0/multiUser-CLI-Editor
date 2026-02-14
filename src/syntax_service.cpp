#include "syntax_service.hpp"
#include <iostream>
#include <cstring> // For strlen, etc.

// 1. Declare the external grammar function (linked from tree-sitter-cpp)
extern "C" TSLanguage *tree_sitter_cpp();

// 2. Global State (Private to this file)
static TSParser *parser = nullptr;
static TSTree *tree = nullptr;

// 3. The "Gap Bridge" Function
// Tree-sitter calls this to read text. We must skip the gap seamlessly.
const char* gap_buffer_read(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read) {
    const GapBuffer *b = static_cast<const GapBuffer*>(payload);
    
    // Calculate the size of the logical text (excluding gap)
    size_t gap_size = b->gap_end - b->gap_start;
    size_t logical_size = b->data.size() - gap_size;

    // Safety check: If asking for out of bounds
    if (byte_index >= logical_size) {
        *bytes_read = 0;
        return nullptr;
    }

    // CASE A: Reading BEFORE the gap
    // The logical index matches the physical index.
    if (byte_index < b->gap_start) {
        // We can read up until the gap starts
        *bytes_read = b->gap_start - byte_index;
        return &b->data[byte_index];
    }

    // CASE B: Reading AFTER the gap
    // We must offset the index to jump over the gap.
    // Logical Index 5 -> Physical Index 10 (if gap is size 5)
    size_t physical_index = byte_index + gap_size;
    
    if (physical_index < b->data.size()) {
        *bytes_read = b->data.size() - physical_index;
        return &b->data[physical_index];
    }

    *bytes_read = 0;
    return nullptr;
}

namespace SyntaxService {

    void init() {
        // Create the parser
        parser = ts_parser_new();
        
        // Set the language to C++
        // If this crashes, your linking in CMake is wrong.
        if (!ts_parser_set_language(parser, tree_sitter_cpp())) {
            std::cerr << "Failed to load C++ grammar!\n";
            exit(1);
        }
    }

    void shutdown() {
        if (tree) ts_tree_delete(tree);
        if (parser) ts_parser_delete(parser);
    }

    void update(const GapBuffer &buffer) {
        // Setup the input structure
        TSInput input;
        input.payload = (void*)&buffer;
        input.encoding = TSInputEncodingUTF8;
        input.read = gap_buffer_read; // Register our custom reader

        // Parse!
        // Passing 'tree' as the second argument allows 'Incremental Parsing'
        // (Tree-sitter will reuse old nodes if they didn't change)
        TSTree *new_tree = ts_parser_parse(parser, tree, input);
        
        if (tree) ts_tree_delete(tree);
        tree = new_tree;
    }

    int get_color_at(size_t char_index) {
        if (!tree) return 0;

        // NOTE: This is a naive implementation for testing.
        // Real implementation requires "Queries" (tree-sitter-query).
        // For now, we will just check if the node is "ERROR" (Red).
        
        TSNode root = ts_tree_root_node(tree);
        TSNode node = ts_node_descendant_for_byte_range(root, char_index, char_index + 1);
        
        if (ts_node_is_null(node)) return 0;

        const char* type = ts_node_type(node);
        
        // Simple string comparisons for testing (fast enough for now)
        // You can check the C++ grammar for these node names
        if (strcmp(type, "string_literal") == 0) return 32; // Green
        if (strcmp(type, "number_literal") == 0) return 33; // Yellow
        if (strcmp(type, "primitive_type") == 0) return 34; // Blue
        if (strcmp(type, "comment") == 0)        return 90; // Grey
        if (ts_node_has_error(node))             return 31; // Red (Syntax Error)

        return 0; // Default color
    }
}