# C++ Text Editor

A C++ text editor project for **CS-218**, demonstrating data structures and algorithms through text editing features.

## About the Project

This project explores how core data structures and algorithms power the features we use in everyday text editors. Instead of treating DSA as pure theory, we apply it to real editing operations such as inserting, deleting, searching, and undoing changes.

## Technologies Used (For Now)

- **C++**
- **Data Structures and Algorithms**

> This list will grow as the project develops.

## Features

The editor is being built around the following ideas (the list will be updated as features are completed):

- [ ] Insert and delete text
- [ ] Cursor movement
- [ ] Undo / redo
- [ ] Search and replace
- [ ] Open and save files

## Project Structure

```
.
├── src/        # C++ source files
├── include/    # Header files
├── tests/      # Test cases
└── docs/       # Documentation
```

## Getting Started

### Prerequisites

- A C++ compiler with C++17 support (e.g. `g++`, `clang++`, or MSVC)
- Git

### Clone the Repository

```bash
git clone https://github.com/<your-username>/<repo-name>.git
cd <repo-name>
```

### Build and Run

The build setup is still being finalized. For now, you can compile manually:

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o editor
./editor
```

On Windows (PowerShell / CMD):

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o editor.exe
editor.exe
```

## Running Tests

Test cases live in the `tests/` folder. Instructions for running them will be added once the first tests are written.

## Team Members

| Name | GitHub | Responsibilities |
|------|--------|------------------|
| Rameen Zulfi | [r2meen](https://github.com/r2meen) | To be assigned |
| Shifa Soomro | [shifafareed79](https://github.com/shifafareed79) | To be assigned |
| Muhammad Saif | [Muhammad-Saif07](https://github.com/Muhammad-Saif07) | To be assigned |
| Muhammad Ibad Mirza | [ibad-mirza](https://github.com/ibad-mirza) | To be assigned |

## Contributing

1. Create a new branch for your work: `git checkout -b feature/your-feature-name`
2. Commit your changes with a clear message: `git commit -m "Add: short description"`
3. Push your branch: `git push origin feature/your-feature-name`
4. Open a Pull Request and let a teammate review it before merging

## Course Information

This project is developed as part of **CS-218** (Data Structures and Algorithms).

## License

This project is for academic purposes. A license can be added here later if needed.