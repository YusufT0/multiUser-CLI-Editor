cmake_minimum_required(VERSION 3.10)
project(editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include libs)

if(WIN32)
    # WINDOWS (MSVC)
    add_compile_definitions(_WIN32)
    add_compile_options(/W4 /D_CRT_SECURE_NO_WARNINGS)

else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_library(gapbuffer
    src/buffer_service.cpp
    src/file_io.cpp
    src/terminal_manager.cpp
    src/clipboard.cpp
    src/editor.cpp
    src/view_service.cpp
    src/selection_service.cpp
)

add_executable(editor
    src/main.cpp
)

target_link_libraries(editor gapbuffer)

