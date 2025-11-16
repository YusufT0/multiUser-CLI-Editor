cmake_minimum_required(VERSION 3.10)
project(editor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include libs)

# Core library
add_library(gapbuffer
    src/gap_buffer.cpp
    src/file_io.cpp
    src/terminal.cpp
)

# Main editor executable
add_executable(editor
    src/main.cpp
)
target_link_libraries(editor gapbuffer)

# Tests executable
add_executable(editor_tests
    tests/test_gap_buffer.cpp
)
target_link_libraries(editor_tests gapbuffer)
