/*
 * file: raylib-init.c
 * -------------------
 * Program to initialize a raylib project
 * This program will do multiple things:
 * - clones the raylib repository and puts the src/ in the deps/ folder
 * - creates a CMakeLists that will properly compile the project on linux and windows
 * - creates a template c file for your project
 * - creates three scripts:
 *   - build.sh - runs cmake to compile the project
 *   - build.bat - same thing as above but for windows
 *   - debug.sh - runs build.sh and then executes gdb with gdbinit script
 *   - gdbinit - makes gdb auto start and auto close
 *
 * Author: Mason Armand
 * Contributors:
 * Date Created: May 25, 2023
 * Last Modified: May 28, 2023
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void execute_command(const char* cmd);
void create_directories(const char** dirs);
void create_directory(const char* dir);
void copy_file(const char* src, const char* dest);
void copy_dir(const char* src, const char* dest);
void create_file(const char* filename, const char* contents[], const char* project_name);
void remove_dir(const char* dirpath);

/* file contents */

const char* build_sh_contents[] = {
        "#!/bin/bash",
        "# check if build directory exists",
        "if [ ! -d \"build\" ]; then",
        "  mkdir build",
        "fi",
        "cd build",
        "cmake ..",
        "make",
        NULL
};

const char* build_bat_contents[] = {
        "@echo off",
        "IF NOT EXIST \"build\" (",
        "  mkdir build",
        ")",
        "cd build",
        "cmake .. -G \"MinGW Makefiles\"",
        "cmake --build .",
        NULL
};

const char* debug_sh_contents[] = {
        "#!/bin/bash",
        "./build.sh",
        "gdb -x gdbinit ./build/bin/%s",
        NULL
};

const char* gdbinit_contents[] = {
        "set $_exitcode = -1",
        "run",
        "if $_exitcode != -1",
        "    quit",
        "end",
        NULL
};

const char* main_c_contents[] = {
    "#include \"raylib.h\"\n",
    "#define SCREEN_WIDTH 640",
    "#define SCREEN_HEIGHT 480",
    "int main(void)",
    "{",
    "    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, \"%s\");",
    "    SetTargetFPS(60);",
    "    while (!WindowShouldClose()) {",
    "        BeginDrawing();",
    "            ClearBackground(RAYWHITE);",
    "            DrawText(\"Hello, World!\", 0, 0, 40, BLACK);",
    "        EndDrawing();",
    "    }",
    "    CloseWindow();",
    "    return 0;",
    "}",
    NULL
};


const char* cmake_lists_contents[] = {
    "cmake_minimum_required(VERSION 3.10)",
    "project(%s VERSION 0.1)",
    "set(CMAKE_C_STANDARD 99)",
    "set(CMAKE_C_STANDARD_REQUIRED True)",
    "set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)",
    "file(GLOB SOURCES \"src/*.c\")",
    "add_executable(%s ${SOURCES})",
    "target_include_directories(%s PUBLIC ${PROJECT_BINARY_DIR})",
    "add_subdirectory(./deps/raylib)",
    "target_link_libraries(%s PRIVATE raylib)",
    "if (${CMAKE_SYSTEM_NAME} MATCHES \"Linux\")",
    "    target_link_libraries(%s PRIVATE glfw m pthread)",
    "elseif (${CMAKE_SYSTEM_NAME} MATCHES \"Windows\")",
    "    target_link_libraries(%s PRIVATE opengl32 gdi32)",
    "endif()",
    "file(COPY res/ DESTINATION ${EXECUTABLE_OUTPUT_PATH}/res)",
    NULL
};

int main(int argc, char** argv)
{
        const char* project_name;
        const char* directories[] = {
                "src",
                "res",
                "deps",
                "deps/raylib",
                NULL
        };

        if (argc < 2) {
                fprintf(stderr, "Usage: %s <project name>\n", argv[0]);
                return 1;
        }

        project_name = argv[1];

        /* create project files */
        create_directories(directories);
        create_file("gdbinit", gdbinit_contents, project_name);
        create_file("CMakeLists.txt", cmake_lists_contents, project_name);
        create_file("build.sh", build_sh_contents, project_name);
        create_file("debug.sh", debug_sh_contents, project_name);
        create_file("build.bat", build_bat_contents, project_name);
        create_file("src/main.c", main_c_contents, project_name);

        /* clone raylib repo */
        execute_command("git clone https://github.com/raysan5/raylib.git");

        /* copy only the required files and directories for building raylib */
        copy_dir("raylib/src", "deps/raylib/src");
        copy_dir("raylib/cmake", "deps/raylib/cmake");
        copy_file("raylib/CMakeLists.txt", "deps/raylib/CMakeLists.txt");
        copy_file("raylib/CMakeOptions.txt", "deps/raylib/CMakeOptions.txt");
        copy_file("raylib/raylib.pc.in", "deps/raylib/raylib.pc.in");
        copy_file("raylib/README.md", "deps/raylib/README.md");
        copy_file("raylib/LICENSE", "deps/raylib/LICENSE");

        /* cleanup raylib git repo */
        remove_dir("raylib");
        rmdir("raylib");

        execute_command("chmod +x build.sh");
        execute_command("chmod +x debug.sh");
        execute_command("./debug.sh");

        printf("Finished!\n");

        return 0;
}


void create_directories(const char** dirs)
{
        int i = 0;
        while (dirs[i] != NULL) {
                create_directory(dirs[i]);
                i++;
        }
}


void create_directory(const char* dir)
{
        int result = mkdir(dir, 0777);

        if (result == 0) {
                printf("Created directory: %s\n", dir);
        }
        else {
                perror("Error creating directory");
                exit(1);
        }
}


void remove_dir(const char* dirpath)
{
        DIR* dir = opendir(dirpath);
        struct dirent* entry;
        char path[1024];
        struct stat st;

        if (dir == NULL)
                return;

        while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                }

                snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);
                if (stat(path, &st) == -1) {
                        continue;
                }

                if (S_ISDIR(st.st_mode)) {
                        remove_dir(path);
                        rmdir(path);
                }
                else if (S_ISREG(st.st_mode)) {
                        remove(path);
                }
        }
        closedir(dir);
}


void execute_command(const char* cmd)
{
        int result = system(cmd);
        if (result != 0) {
                fprintf(stderr, "Failed to execute `%s` command.\n", cmd);
                exit(1);
        }

        if (WEXITSTATUS(result) != 0) {
                fprintf(stderr, "`%s` command failed with exit status %d\n", cmd, WEXITSTATUS(result));
                exit(1);
        }
}


void copy_file(const char* src, const char* dest)
{
        FILE* src_file = fopen(src, "rb");
        FILE* dest_file = fopen(dest, "wb");

        char buf[1024];
        size_t size;

        printf("Copying file: %s to %s\n", src, dest);

        while ((size = fread(buf, 1, sizeof(buf), src_file)) > 0) {
                fwrite(buf, 1, size, dest_file);
        }

        fclose(src_file);
        fclose(dest_file);
}


void copy_dir(const char* src, const char* dest)
{
        DIR* dir = opendir(src);
        struct dirent* entry;
        char src_path[1024];
        char dest_path[1024];
        struct stat st;

        if (dir == NULL)
                return;
        mkdir(dest, 0755);


        while ((entry = readdir(dir)) != NULL) {
                bool is_dir = strcmp(entry->d_name, ".") == 0;
                bool is_pdir = strcmp(entry->d_name, "..") == 0;
                bool is_dest = strcmp(entry->d_name, dest) == 0;

                if (is_dir || is_pdir || is_dest)
                        continue;

                snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
                snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

                if (stat(src_path, &st) == -1)
                        continue;

                if (S_ISDIR(st.st_mode)) {
                        copy_dir(src_path, dest_path);
                }
                else if (S_ISREG(st.st_mode)) {
                        copy_file(src_path, dest_path);
                }
        }
        closedir(dir);
}


void create_file(const char* filename, const char* contents[], const char* project_name)
{
        FILE* file = fopen(filename, "w");
        unsigned int i = 0;
        if (file == NULL) {
                perror("Failed to open file");
                exit(1);
        }

        printf("Creating %s...\n", filename);

        while (contents[i] != NULL) {
                fprintf(file, contents[i], project_name);
                fprintf(file, "\n");
                i++;
        }

        fclose(file);
}
