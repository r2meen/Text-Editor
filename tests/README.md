Contains test cases to verify the functionality of the text editor.

1. Run the Undo System branch

cd "your folder"
git switch Undo-System---Rameen
cmake -S . -B build
cmake --build build
./build/undo_test

2. Run the Search-Replace branch

cd "your folder"
git switch Search-Replace
cmake -S . -B build
cmake --build build
./build/search_replace_test

3. Quick reference:
    
     Task                                 	Command
Run Undo tests	                       ./build/undo_test
Run Search-Replace tests	           ./build/search_replace_test
Run configured automated tests	        ctest --test-dir build --output-on-failure
Check current branch	                git branch --show-current
Check changes before committing	        git status

Important: The build/ folder is shared between branches. If the two branches have different CMakeLists.txt files, rerun the CMake configure and build commands after switching branches, as shown above.