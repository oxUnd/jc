#include "jc.h"
#include "utils.h"
#include <dirent.h>

// Helper function to remove a directory recursively
static int remove_directory(const char *path) {
    DIR *d = opendir(path);
    if (!d) {
        return 0; // Directory doesn't exist or can't be opened
    }
    
    struct dirent *entry;
    char filepath[1024];
    
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                remove_directory(filepath);
            } else {
                unlink(filepath);
            }
        }
    }
    
    closedir(d);
    rmdir(path);
    return 0;
}

// Helper function to remove a file if it exists
static void remove_file_if_exists(const char *path) {
    if (file_exists(path)) {
        printf("  Removing %s\n", path);
        unlink(path);
    }
}

// Helper function to remove files matching a pattern in current directory
static void remove_files_with_extension(const char *ext) {
    DIR *d = opendir(".");
    if (!d) {
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        size_t len = strlen(entry->d_name);
        size_t ext_len = strlen(ext);
        
        if (len > ext_len && strcmp(entry->d_name + len - ext_len, ext) == 0) {
            printf("  Removing %s\n", entry->d_name);
            unlink(entry->d_name);
        }
    }
    
    closedir(d);
}

// Recursively clean src directory
static void clean_src_directory(void) {
    if (!directory_exists("src")) {
        return;
    }
    
    DIR *d = opendir("src");
    if (!d) {
        return;
    }
    
    struct dirent *entry;
    char filepath[1024];
    
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "src/%s", entry->d_name);
        
        // Skip template directory
        if (strcmp(entry->d_name, "templates") == 0) {
            continue;
        }
        
        // Remove .o files and executables
        size_t len = strlen(entry->d_name);
        if (len > 2 && strcmp(entry->d_name + len - 2, ".o") == 0) {
            printf("  Removing %s\n", filepath);
            unlink(filepath);
        } else {
            // Check if it's an executable
            struct stat st;
            if (stat(filepath, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
                // Check if it's not a script (scripts usually have extensions or start with #!)
                FILE *f = fopen(filepath, "r");
                if (f) {
                    char first_line[3];
                    if (fgets(first_line, sizeof(first_line), f) && first_line[0] != '#') {
                        // Likely a compiled binary
                        printf("  Removing %s\n", filepath);
                        unlink(filepath);
                    }
                    fclose(f);
                }
            }
        }
    }
    
    closedir(d);
}

int cmd_clean(int argc, char *argv[]) {
    (void)argc;  // unused
    (void)argv;  // unused
    
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Please run this command in a directory containing configure.ac\n");
        return 1;
    }

    printf("Cleaning project...\n\n");

    // If Makefile exists, use make clean and make distclean
    if (file_exists("Makefile")) {
        printf("Running make clean...\n");
        execute_command_quiet("make clean 2>/dev/null");
        
        printf("Running make distclean...\n");
        execute_command_quiet("make distclean 2>/dev/null");
        printf("\n");
    }

    // Manually remove common build artifacts
    printf("Removing build artifacts:\n");
    
    // Remove autotools cache
    if (directory_exists("autom4te.cache")) {
        printf("  Removing autom4te.cache/\n");
        remove_directory("autom4te.cache");
    }
    
    // Remove object files and executables in src/
    clean_src_directory();
    
    // Remove generated files
    remove_file_if_exists("config.log");
    remove_file_if_exists("config.status");
    remove_file_if_exists("config.h");
    remove_file_if_exists("stamp-h1");
    remove_file_if_exists("Makefile");
    remove_file_if_exists("src/Makefile");
    remove_file_if_exists("tests/Makefile");
    
    // Remove libtool files if they exist
    remove_file_if_exists("libtool");
    remove_files_with_extension(".la");
    remove_files_with_extension(".lo");
    
    // Remove backup files
    remove_files_with_extension("~");
    
    printf("\n✓ Clean completed successfully!\n");
    return 0;
}
