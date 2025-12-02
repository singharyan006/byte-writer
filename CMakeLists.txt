cmake_minimum_required(VERSION 3.15)
project(byte-writer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    src/main.cpp
    src/editor/Editor.cpp
    src/editor/Row.cpp
    src/editor/Syntax.cpp
    src/editor/FileIO.cpp
    src/terminal/Terminal.cpp
    src/utils/Buffer.cpp
    src/utils/Helpers.cpp
)

# Header files
set(HEADERS
    src/editor/Editor.h
    src/editor/Row.h
    src/editor/Syntax.h
    src/editor/FileIO.h
    src/terminal/Terminal.h
    src/utils/Buffer.h
    src/utils/Helpers.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/terminal
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Enable testing
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
