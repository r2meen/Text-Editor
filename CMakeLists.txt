cmake_minimum_required(VERSION 3.16)

project(TextEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(text_editor
    main.cpp
    src/Document.cpp
    src/Search.cpp
    src/Replace.cpp
    src/UndoManager.cpp
)

target_include_directories(text_editor PRIVATE include)
