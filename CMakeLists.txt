cmake_minimum_required(VERSION 3.16)
project(TextEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Undo System test
add_executable(undo_test
    src/UndoManager.cpp
    tests/undo_test.cpp
)

# Tell the compiler where your headers live (Stack.h, UndoManager.h, ...)
target_include_directories(undo_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()
add_test(NAME UndoTest COMMAND undo_test)