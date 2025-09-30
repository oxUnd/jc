#include "jc.h"
#include "utils.h"
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int cmd_run(int argc, char *argv[]) {
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        return 1;
    }

    // First, ensure the project is built
    if (!file_exists("Makefile")) {
        printf("Project not built yet. Building first...\n");
        if (cmd_build(0, NULL) != 0) {
            return 1;
        }
    }

    // Find the executable in src/ directory
    char executable[PATH_MAX];
    
    // Try to find executable in common locations
    const char *search_dirs[] = {
        "src",
        ".",
        NULL
    };
    
    int found = 0;
    for (int i = 0; search_dirs[i] != NULL; i++) {
        if (find_executable(search_dirs[i], executable, sizeof(executable)) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        fprintf(stderr, "Error: Could not find executable to run\n");
        fprintf(stderr, "Make sure the project is built successfully\n");
        return 1;
    }

    printf("Running: %s\n", executable);
    printf("----------------------------------------\n");

    // Build command with any additional arguments
    char cmd[PATH_MAX * 2];
    int offset = snprintf(cmd, sizeof(cmd), "%s", executable);
    
    // Pass through any additional arguments
    for (int i = 1; i < argc; i++) {
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " %s", argv[i]);
    }

    // Execute the program
    int ret = system(cmd);
    
    printf("----------------------------------------\n");
    
    if (ret != 0) {
        printf("Program exited with code: %d\n", ret);
        if (ret == 139 || ret == 11 || ret / 256 == 139 || ret / 256 == 11) {
            printf("\nSegmentation fault detected!\n");
            printf("Run 'jc bt' to debug the issue\n");
        } else if (ret == 134 || ret == 6 || ret / 256 == 134 || ret / 256 == 6) {
            printf("\nAbort signal detected!\n");
            printf("Run 'jc bt' to debug the issue\n");
        }
        return 1;
    }
    
    return 0;
}
