cmake_minimum_required(VERSION 3.10)
project(editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the include directories
include_directories(include libs)

# --- OS Specific Configuration ---
if(WIN32)
    # WINDOWS (MSVC)
    add_compile_definitions(_WIN32)
    add_compile_options(/W4 /D_CRT_SECURE_NO_WARNINGS)

else()
    # LINUX / MACOS (GCC/Clang)
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# --- Core Library ---
add_library(gapbuffer
    src/buffer_service.cpp
    src/file_io.cpp
    src/terminal_manager.cpp
    src/clipboard.cpp
    src/editor.cpp
    src/view_service.cpp
    src/selection_service.cpp
)

# --- Main Editor Executable ---
add_executable(editor
    src/main.cpp
)

# Link the library to the executable
target_link_libraries(editor gapbuffer)

# --- Windows Specific Linking (Optional but Recommended) ---
# If you use sockets or advanced Windows APIs later, you might need:
# if(WIN32)
#     target_link_libraries(editor ws2_32) 
# endif()

# --- Tests Executable (Commented Out) ---
# add_executable(editor_tests
#     tests/test_gap_buffer.cpp
# )
# target_link_libraries(editor_tests gapbuffer)