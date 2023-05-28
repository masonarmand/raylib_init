# raylib_init
Generates a template raylib project.  
Heres what it does
- Creates directories: `src`, `res`, `include`, `deps`
- Clones the raylib repository
- Moves the raylib source folder and other files required for building into a `deps` folder.
- Creates a CMakeLists that will properly handle compiling raylib and your project
- Creates a simple main.c file that will display a hello world raylib program
- creates three scripts:
  - build.sh - runs cmake to compile the project
  - build.bat - same thing as above but for windows
  - debug.sh - runs build.sh and then executes gdb with gdbinit script
  - gdbinit - makes gdb auto start and auto close

This program assumes you have cmake, git, and gdb installed.

## Installation
```
git clone https://github.com/masonarmand/raylib_init.git
cd raylib_init
sudo make install
```
## Usage
cd into your empty project directory and simply type:
```
raylib-init <name of your project>
```
