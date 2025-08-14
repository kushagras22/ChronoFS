# ChronoFS

## Overview
**ChronoFS** is a lightweight, C++17-based file management and version control system **built from scratch**, no external VCS like Git.  

It allows you to:
- Create, delete, rename, and move files or directories (folders/sub-folders)
- Maintain **version history** for every file with the ability to rollback or restore
- Perform **diff** operations between file versions
- Log and track all file changes
- Query **status** of the working directory
- Use a **command-line interface** for all operations

Designed for scalability and modularity, **ChronoFS** uses an **object-oriented architecture** that makes it easy to extend with new commands and storage backends.

---

## Architecture
ChronoFS follows a **layered modular architecture**:

### **Modules**
- **CLI Module (`cli/`)** → Parses commands (`status`, `log`, `diff`, etc.) and executes corresponding service calls.
- **File System Module (`fs/`)** → Abstracts file/directory manipulation, path handling, and file metadata.
- **Version Control Module (`vcs/`)** → Manages snapshot creation, storage, and retrieval of file versions.
- **Utilities Module (`util/`)** → Logging, color output, config parsing, error handling.

---

##  Features
- Create, delete, move, and rename files/folders
- Track file history with version timestamps
- Restore any previous file version
- Show file changes using a `diff` command
- Query working directory status
- Logging with detailed change history
- Modular architecture for easy future expansion

---

## Folder Structure
.
├── cli/ # Command-line interface handlers
├── fs/ # File system abstraction layer
├── vcs/ # Version control implementation
├── util/ # Utility functions and helpers
├── CMakeLists.txt
└── README.md

## Prerequisites
- **C++17 or newer** compiler (GCC ≥ 9, Clang ≥ 9, MSVC ≥ 2019)
- **CMake 3.10+**
- **MinGW-w64** or **MSYS2** (for Windows users)
- **Visual Studio Code** or any preferred IDE


## Setup & Build Instructions
1️⃣ Clone the repository
git clone https://github.com/kushagras22/ChronoFS.git
cd chronofs

2️⃣ Create a build directory
mkdir build && cd build

3️⃣ Run CMake
cmake ..

4️⃣ Build the project
cmake --build .

5️⃣ Run ChronoFS
./chronofs

##  Commands

| Command         | Description                                           |
|-----------------|-------------------------------------------------------|
| `init`          | Initialize ChronoFS in the current directory          |
| `status`        | Show current working directory status                 |
| `log`           | Show commit history and version logs                   |
| `diff`          | Show differences between file versions                |
| `add <file>`    | Track or update a file in ChronoFS                     |
| `rm <file>`     | Remove a file from working directory and history       |
| `restore <id>`  | Restore a file version using version ID                |


## Contributing
Contributions are welcome!

1. **Fork the repository** on GitHub.
2. **Clone your fork** locally:
git clone https://github.com/kushagras22/ChronoFS.git

3. **Create a feature branch**:
git checkout -b feature-name

4. **Commit changes**:
git commit -m "Description of changes"

5. **Push to your fork**:
git push origin feature-name

6. **Open a Pull Request**.


## Roadmap
- Add compression for stored versions
- Add networking support for remote repositories
- Implement file tagging and search
- Cross-platform GUI frontend


## Acknowledgments
Built with ❤️ in C++17 to show how **version control concepts** can be implemented **from scratch** without Git.
