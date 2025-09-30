#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Utility function prototypes
int create_directory(const char *path);
int file_exists(const char *path);
int directory_exists(const char *path);
int copy_file(const char *src, const char *dst);
int write_file(const char *path, const char *content);
char *read_file(const char *path);
int execute_command(const char *cmd);
int execute_command_quiet(const char *cmd);
char *get_template_path(const char *template_name);
int is_automake_project(void);
int find_executable(const char *dir, char *output, size_t output_size);

#endif // UTILS_H
