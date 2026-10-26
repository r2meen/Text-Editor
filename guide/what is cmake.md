The problem CMake solves:

Right now, to build our project with plain g++, we have to type something like:

bash g++ -I../include src/UndoManager.cpp src/Stack.cpp tests/undo_test.cpp -o undo_test

That’s okay for 3 files. But our text editor will grow: which means more .cpp files, more tests, more folders. Typing (or remembering) that long command every time gets painful, and one forgotten file means an error.

What CMake does instead:

You write the instructions once in CMakeLists.txt:

which files to compile,
where the headers live,
which C++ version to use.

After that, we just run:

bash (folder)
cmake ..
make

and it builds everything for you.

Why it’s worth it for our project:

Without CMake	                                   With CMake
Type a long g++ command every time	          Write it once, reuse forever
Easy to forget a file or -I flag	          The list is saved in one place
Teammates each need your exact command        Teammates just run cmake and make
Recompiles everything each time	              make only rebuilds files you changed
Hard to add many tests	                      One add_executable line per test

The teammate point matters a lot for a group project. Everyone builds the same way, on any computer.