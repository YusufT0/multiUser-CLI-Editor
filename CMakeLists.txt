cmake_minimum_required(VERSION 3.10)
project(editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Global fallbacks
include_directories(include libs)

# Platform flags
if(WIN32)
  add_compile_definitions(_WIN32)
  add_compile_options(/W4 /D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# 1. Find the OS thread library (Required for Asio backend)
find_package(Threads REQUIRED)

# Define the library target
add_library(gapbuffer
    src/buffer_service.cpp
    src/file_io.cpp
    src/terminal_manager.cpp
    src/clipboard.cpp
    src/editor.cpp
    src/view_service.cpp
    src/selection_service.cpp
    src/changefilemodal.cpp
    src/network_models.cpp
    src/logger.cpp
    src/input_handler.cpp
    src/input_handler.cpp
)

# 2. Tell Asio to compile without looking for Boost
target_compile_definitions(gapbuffer PRIVATE ASIO_STANDALONE)

# 3. Grant include path access to the actual code files inside the library
target_include_directories(gapbuffer PRIVATE
    include
    libs/asio/include
)

# Define the executable target
add_executable(editor
    src/main.cpp
)

target_compile_definitions(editor PRIVATE ASIO_STANDALONE)

target_include_directories(editor PRIVATE
    include
    libs/asio/include
)

# 4. Link the library AND the system threads to your final executable
target_link_libraries(editor PRIVATE
    gapbuffer
    Threads::Threads
)
