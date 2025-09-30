#include "jc.h"
#include "utils.h"

int cmd_install(int argc, char *argv[]) {
    (void)argc;  // unused
    (void)argv;  // unused
    
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        return 1;
    }

    // Ensure project is built
    if (!file_exists("Makefile")) {
        printf("Project not configured. Building first...\n");
        if (cmd_build(0, NULL) != 0) {
            return 1;
        }
    }

    printf("Installing project...\n\n");

    // Run make install
    if (execute_command("make install") != 0) {
        fprintf(stderr, "\nError: Installation failed\n");
        fprintf(stderr, "You may need to run with sudo: sudo jc install\n");
        return 1;
    }

    printf("\n✓ Installation completed successfully!\n");
    return 0;
}
