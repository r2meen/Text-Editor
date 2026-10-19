cmake_minimum_required(VERSION 3.16)

project(TextEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(search_replace_test
    src/Search.cpp
    src/Replace.cpp
    tests/search_replace_test.cpp
)

target_include_directories(search_replace_test PRIVATE include)

enable_testing()
add_test(NAME SearchReplaceTest COMMAND search_replace_test)