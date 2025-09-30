#include "jc.h"
#include "utils.h"
#include <dirent.h>
#include <libgen.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Forward declarations
static int add_file(const char *src_path, const char *dst_path);
static int add_directory(const char *src_path, const char *dst_path);
static int add_dependency(const char *dep_name);
static int update_makefile_am(const char *file_path);
static int is_c_source_file(const char *path);
static int is_header_file(const char *path);
static void print_add_usage(void);

// Check if file is a C source file
static int is_c_source_file(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return 0;
    return (strcmp(ext, ".c") == 0);
}

// Check if file is a header file (currently unused but kept for future use)
__attribute__((unused)) static int is_header_file(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return 0;
    return (strcmp(ext, ".h") == 0);
}

// Print usage information for add command
static void print_add_usage(void) {
    printf("Usage: jc add <type> <target>\n\n");
    printf("Types:\n");
    printf("  file <path>        Add a single file to the project\n");
    printf("  dir <path>         Add a directory to the project\n");
    printf("  dep <library>      Add a library dependency\n\n");
    printf("Examples:\n");
    printf("  jc add file utils.c\n");
    printf("  jc add file src/utils.c\n");
    printf("  jc add dir src/lib\n");
    printf("  jc add dep math\n");
    printf("  jc add dep pthread\n\n");
}

// Add a single file to the project
static int add_file(const char *src_path, const char *dst_path) {
    // Check if source file exists
    if (!file_exists(src_path)) {
        fprintf(stderr, "Error: Source file '%s' does not exist\n", src_path);
        return 1;
    }

    // Create destination directory if it doesn't exist
    char dst_dir[PATH_MAX];
    strncpy(dst_dir, dst_path, sizeof(dst_dir) - 1);
    dst_dir[sizeof(dst_dir) - 1] = '\0';
    char *dir = dirname(dst_dir);
    
    if (strcmp(dir, ".") != 0) {
        if (create_directory(dir) != 0) {
            fprintf(stderr, "Error: Failed to create destination directory\n");
            return 1;
        }
    }

    // Copy the file
    if (copy_file(src_path, dst_path) != 0) {
        fprintf(stderr, "Error: Failed to copy file from '%s' to '%s'\n", src_path, dst_path);
        return 1;
    }

    printf("✓ Added file: %s -> %s\n", src_path, dst_path);

    // If it's a C source file, update Makefile.am
    if (is_c_source_file(dst_path)) {
        update_makefile_am(dst_path);
    }

    return 0;
}

// Add a directory to the project
static int add_directory(const char *src_path, const char *dst_path) {
    // Check if source directory exists
    if (!directory_exists(src_path)) {
        fprintf(stderr, "Error: Source directory '%s' does not exist\n", src_path);
        return 1;
    }

    // Create destination directory
    if (create_directory(dst_path) != 0) {
        fprintf(stderr, "Error: Failed to create destination directory\n");
        return 1;
    }

    // Open source directory
    DIR *dir = opendir(src_path);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open source directory '%s'\n", src_path);
        return 1;
    }

    struct dirent *entry;
    int error = 0;
    int added_files = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files and directories
        if (entry->d_name[0] == '.') {
            continue;
        }

        char src_file_path[PATH_MAX];
        char dst_file_path[PATH_MAX];
        
        snprintf(src_file_path, sizeof(src_file_path), "%s/%s", src_path, entry->d_name);
        snprintf(dst_file_path, sizeof(dst_file_path), "%s/%s", dst_path, entry->d_name);

        struct stat st;
        if (stat(src_file_path, &st) != 0) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Recursively add subdirectory
            if (add_directory(src_file_path, dst_file_path) != 0) {
                error = 1;
                break;
            }
            added_files++;
        } else {
            // Add file
            if (add_file(src_file_path, dst_file_path) != 0) {
                error = 1;
                break;
            }
            added_files++;
        }
    }

    closedir(dir);

    if (error) {
        fprintf(stderr, "Error: Failed to copy some files from directory '%s'\n", src_path);
        return 1;
    }

    if (added_files > 0) {
        printf("✓ Added directory: %s -> %s (%d items)\n", src_path, dst_path, added_files);
    } else {
        printf("✓ Added directory: %s -> %s (empty directory)\n", src_path, dst_path);
    }

    return 0;
}

// Add a library dependency
static int add_dependency(const char *dep_name) {
    printf("Adding dependency: %s\n", dep_name);
    
    // Check if we're in an automake project
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Run this command from a project created with 'jc new'\n");
        return 1;
    }

    // Check if src/Makefile.am exists
    if (!file_exists("src/Makefile.am")) {
        fprintf(stderr, "Error: src/Makefile.am not found\n");
        return 1;
    }

    // Read current Makefile.am content
    char *content = read_file("src/Makefile.am");
    if (!content) {
        fprintf(stderr, "Error: Failed to read src/Makefile.am\n");
        return 1;
    }

    // Check if dependency is already added
    char lib_flag[256];
    snprintf(lib_flag, sizeof(lib_flag), "-l%s", dep_name);
    
    if (strstr(content, lib_flag) != NULL) {
        printf("✓ Dependency '%s' is already added\n", dep_name);
        free(content);
        return 0;
    }

    // Find the line with _LDFLAGS or _LDADD and add the dependency
    char *line_start = content;
    char *new_content = malloc(strlen(content) + 256);
    if (!new_content) {
        free(content);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    char *output = new_content;
    int found_ldflags = 0;
    int found_ldadd = 0;

    while (*line_start) {
        char *line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }

        // Check if this line contains _LDFLAGS or _LDADD
        if (strstr(line_start, "_LDFLAGS") != NULL && !found_ldflags) {
            // Copy the line up to the end
            size_t len = line_end - line_start;
            memcpy(output, line_start, len);
            output += len;
            
            // Add our library flag
            if (line_end > line_start && *(line_end - 1) != ' ') {
                *output++ = ' ';
            }
            sprintf(output, "-l%s", dep_name);
            output += strlen(dep_name) + 2;
            
            found_ldflags = 1;
        } else if (strstr(line_start, "_LDADD") != NULL && !found_ldadd) {
            // Copy the line up to the end
            size_t len = line_end - line_start;
            memcpy(output, line_start, len);
            output += len;
            
            // Add our library flag
            if (line_end > line_start && *(line_end - 1) != ' ') {
                *output++ = ' ';
            }
            sprintf(output, "-l%s", dep_name);
            output += strlen(dep_name) + 2;
            
            found_ldadd = 1;
        } else {
            // Copy the line as is
            size_t len = line_end - line_start;
            memcpy(output, line_start, len);
            output += len;
        }

        if (*line_end == '\n') {
            *output++ = '\n';
            line_start = line_end + 1;
        } else {
            *output++ = '\0';
            break;
        }
    }

    // If neither _LDFLAGS nor _LDADD was found, add _LDFLAGS line
    if (!found_ldflags && !found_ldadd) {
        // Find the binary name from the first line
        char *bin_line = strstr(new_content, "bin_PROGRAMS");
        if (bin_line) {
            char *bin_name_start = strchr(bin_line, '=');
            if (bin_name_start) {
                bin_name_start++; // Skip '='
                while (*bin_name_start == ' ' || *bin_name_start == '\t') {
                    bin_name_start++;
                }
                char *bin_name_end = bin_name_start;
                while (*bin_name_end && *bin_name_end != '\n' && *bin_name_end != ' ') {
                    bin_name_end++;
                }
                
                // Extract binary name
                char bin_name[256];
                size_t name_len = bin_name_end - bin_name_start;
                if (name_len < sizeof(bin_name)) {
                    memcpy(bin_name, bin_name_start, name_len);
                    bin_name[name_len] = '\0';
                    
                    // Add _LDFLAGS line
                    sprintf(output, "\n%s_LDFLAGS = -l%s\n", bin_name, dep_name);
                }
            }
        }
    }

    // Write the updated content back
    if (write_file("src/Makefile.am", new_content) != 0) {
        fprintf(stderr, "Error: Failed to update src/Makefile.am\n");
        free(content);
        free(new_content);
        return 1;
    }

    printf("✓ Added dependency '%s' to src/Makefile.am\n", dep_name);
    printf("  You may need to run 'jc build' to rebuild the project\n");

    free(content);
    free(new_content);
    return 0;
}

// Update Makefile.am to include new source files
static int update_makefile_am(const char *file_path) {
    // Only update if we're in an automake project
    if (!is_automake_project()) {
        return 0;
    }

    if (!file_exists("src/Makefile.am")) {
        return 0;
    }

    // Read current Makefile.am content
    char *content = read_file("src/Makefile.am");
    if (!content) {
        return -1;
    }

    // Extract just the filename from the full path
    char *filename = basename((char*)file_path);
    
    // Check if file is already in the SOURCES list
    char *sources_line = strstr(content, "_SOURCES");
    if (sources_line) {
        if (strstr(sources_line, filename) != NULL) {
            // File already in SOURCES, nothing to do
            free(content);
            return 0;
        }
    }

    // Add the file to SOURCES
    char *new_content = malloc(strlen(content) + strlen(filename) + 10);
    if (!new_content) {
        free(content);
        return -1;
    }

    // Find the end of the SOURCES line and add our file
    char *output = new_content;
    char *line_start = content;
    int sources_updated = 0;

    while (*line_start) {
        char *line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }

        // Look for the specific SOURCES line (e.g., "test_project_SOURCES = main.c")
        if (strstr(line_start, "_SOURCES =") != NULL && !sources_updated) {
            // Copy the line up to the end
            size_t len = line_end - line_start;
            memcpy(output, line_start, len);
            output += len;
            
            // Add our source file
            if (line_end > line_start && *(line_end - 1) != ' ') {
                *output++ = ' ';
            }
            sprintf(output, " %s", filename);
            output += strlen(filename) + 1;
            
            sources_updated = 1;
        } else {
            // Copy the line as is
            size_t len = line_end - line_start;
            memcpy(output, line_start, len);
            output += len;
        }

        if (*line_end == '\n') {
            *output++ = '\n';
            line_start = line_end + 1;
        } else {
            *output++ = '\0';
            break;
        }
    }

    // Write the updated content back
    if (write_file("src/Makefile.am", new_content) != 0) {
        free(content);
        free(new_content);
        return -1;
    }

    printf("✓ Updated src/Makefile.am to include %s\n", filename);

    free(content);
    free(new_content);
    return 0;
}

int cmd_add(int argc, char *argv[]) {
    if (argc < 3) {
        print_add_usage();
        return 1;
    }

    const char *type = argv[1];
    const char *target = argv[2];

    // Check if we're in an automake project (except for dependency addition)
    if (strcmp(type, "dep") != 0 && !is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Run this command from a project created with 'jc new'\n");
        return 1;
    }

    if (strcmp(type, "file") == 0) {
        // Add a single file
        char dst_path[PATH_MAX];
        
        // Always put files in src/ directory, extract filename from source path
        char *filename = basename((char*)target);
        snprintf(dst_path, sizeof(dst_path), "src/%s", filename);

        return add_file(target, dst_path);

    } else if (strcmp(type, "dir") == 0) {
        // Add a directory
        char dst_path[PATH_MAX];
        
        // Extract directory name from source path
        char *dir_name = basename((char*)target);
        snprintf(dst_path, sizeof(dst_path), "src/%s", dir_name);

        return add_directory(target, dst_path);

    } else if (strcmp(type, "dep") == 0) {
        // Add a library dependency
        return add_dependency(target);

    } else {
        fprintf(stderr, "Error: Unknown type '%s'\n\n", type);
        print_add_usage();
        return 1;
    }
}
