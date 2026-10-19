
cmake_minimum_required(VERSION 3.16)

project(TextEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Undo System test
add_executable(undo_test
    src/UndoManager.cpp
    tests/undo_test.cpp
)

target_include_directories(undo_test PRIVATE include)

enable_testing()
add_test(NAME UndoTest COMMAND undo_test)
