# raylib_init
Generates a template raylib project.  
Heres what it does
- Creates directories: `src`, `res`, `include`, `deps`
- Clones the raylib repository
- Moves the raylib source folder into a `deps` folder.
- Creates a makefile that will properly handle compiling raylib and your project
- Creates a simple main.c file that will display a hello world raylib program

The makefile is also configured in a way that automatically launches your game in gdb, this is useful for debugging crashes.

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
