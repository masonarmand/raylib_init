/*
 * file: raylib-init.c
 * -------------------
 * Program to initialize a raylib project
 * This program will do multiple things:
 * - clones the raylib repository and puts the src/ in the deps/ folder
 * - creates a makefile that will properly compile the project
 * - creates a template c file for your project
 *
 * Author: Mason Armand
 * Contributors:
 * Date Created: May 25, 2023
 * Last Modified: May 25, 2023
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
void copy_dir(const char* src, const char* dest);
void remove_dir(const char* dirpath);
void create_makefile(const char* project_name);
void create_main_c(void);
void create_gdbinit(void);


int main(int argc, char** argv)
{
        const char* project_name;
        const char* directories[] = {
                "src",
                "res",
                "include",
                "deps",
                "deps/raylib",
                NULL
        };

        if (argc < 2) {
                fprintf(stderr, "Usage: %s <project name>\n", argv[0]);
                return 1;
        }

        project_name = argv[1];

        create_directories(directories);
        execute_command("git clone https://github.com/raysan5/raylib.git");
        copy_dir("raylib/src", "deps/raylib/src");
        remove_dir("raylib");
        rmdir("raylib");
        create_makefile(project_name);
        create_main_c();
        create_gdbinit();
        execute_command("make");

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
                bool is_index = strcmp(entry->d_name, "index") == 0;
                bool is_dest = strcmp(entry->d_name, dest) == 0;
                bool is_sh = strstr(entry->d_name, ".sh") != NULL;
                bool is_txt = strstr(entry->d_name, ".txt") != NULL;

                if (is_dir || is_pdir || is_index || is_dest || is_sh || is_txt)
                        continue;

                snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
                snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

                if (stat(src_path, &st) == -1)
                        continue;

                if (S_ISDIR(st.st_mode)) {
                        copy_dir(src_path, dest_path);
                }
                else if (S_ISREG(st.st_mode)) {
                        FILE* src_file = fopen(src_path, "rb");
                        FILE* dest_file = fopen(dest_path, "wb");

                        char buf[1024];
                        size_t size;

                        printf("Copying file: %s to %s\n", src_path, dest_path);

                        while ((size = fread(buf, 1, sizeof(buf), src_file)) > 0) {
                                fwrite(buf, 1, size, dest_file);
                        }

                        fclose(src_file);
                        fclose(dest_file);
                }
        }
        closedir(dir);
}


void create_makefile(const char* project_name)
{
        FILE *fp = fopen("Makefile", "w");
        if (fp == NULL) {
                perror("Failed to open file");
                exit(1);
        }

        printf("Creating Makefile...\n");

        fprintf(fp, "WARNINGS = -Werror -Wall -Wextra\n");
        fprintf(fp, "CC = gcc\n");
        fprintf(fp, "CFLAGS = -std=c99 -I./include/\n");
        fprintf(fp, "LDFLAGS = -lraylib -lglfw -lGL -lopenal -lm -pthread -ldl\n");
        fprintf(fp, "SOURCES = src/*.c\n");
        fprintf(fp, "EXEC = %s\n\n", project_name);
        fprintf(fp, ".PHONY: all raylib copy_lib\n\n");
        fprintf(fp, "all: raylib copy_lib build debug clean\n\n");
        fprintf(fp, "raylib:\n");
        fprintf(fp, "\t@echo \"Building raylib...\"\n");
        fprintf(fp, "\tcd deps/raylib/src/ && $(MAKE) PLATFORM=PLATFORM_DESKTOP\n\n");
        fprintf(fp, "copy_lib:\n");
        fprintf(fp, "\t@echo \"Copying libraries...\"\n");
        fprintf(fp, "\t@echo \"Copying libraylib.a to include/\"\n");
        fprintf(fp, "\tcp deps/raylib/src/libraylib.a ./include/\n\n");
        fprintf(fp, "build:\n");
        fprintf(fp, "\t$(CC) -g $(CFLAGS) $(WARNINGS) $(SOURCES) $(LDFLAGS) -o $(EXEC)\n\n");
        fprintf(fp, "run:\n");
        fprintf(fp, "\t./$(EXEC)\n\n");
        fprintf(fp, "install: all\n");
        fprintf(fp, "\tcp -f $(EXEC) /usr/bin\n");
        fprintf(fp, "\tchmod 755 /usr/bin/$(EXEC)\n\n");
        fprintf(fp, "debug:\n");
        fprintf(fp, "\tgdb -x gdbinit $(EXEC)\n\n");
        fprintf(fp, "clean:\n");
        fprintf(fp, "\trm $(EXEC)\n");

        fclose(fp);
}


void create_main_c(void)
{
        FILE *fp = fopen("src/main.c", "w");
        if (fp == NULL) {
                perror("Failed to open file");
                exit(1);
        }

        printf("Creating src/main.c...\n");

        fprintf(fp, "#include \"raylib.h\"\n\n");
        fprintf(fp, "#define SCREEN_WIDTH 640\n");
        fprintf(fp, "#define SCREEN_HEIGHT 480\n\n");
        fprintf(fp, "int main(void)\n");
        fprintf(fp, "{\n");
        fprintf(fp, "\tInitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, \"window\");\n");
        fprintf(fp, "\tSetTargetFPS(60);\n\n");
        fprintf(fp, "\twhile (!WindowShouldClose()) {\n");
        fprintf(fp, "\t\tBeginDrawing();\n");
        fprintf(fp, "\t\t\tClearBackground(RAYWHITE);\n");
        fprintf(fp, "\t\t\tDrawText(\"Hello, World!\", 0, 0, 40, BLACK);\n");
        fprintf(fp, "\t\tEndDrawing();\n");
        fprintf(fp, "\t}\n");
        fprintf(fp, "\tCloseWindow();\n\n");
        fprintf(fp, "\treturn 0;\n");
        fprintf(fp, "}\n");

        fclose(fp);
}


void create_gdbinit(void)
{
        FILE *fp = fopen("gdbinit", "w");
        if (fp == NULL) {
                perror("Failed to open file");
                exit(1);
        }

        printf("Creating gdbinit...\n");

        fprintf(fp, "set $_exitcode = -1\n");
        fprintf(fp, "run\n");
        fprintf(fp, "if $_exitcode != -1\n");
        fprintf(fp, "\tquit\n");
        fprintf(fp, "end\n");

        fclose(fp);

}
