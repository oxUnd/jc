#include "jc.h"
#include "utils.h"

int cmd_build(int argc, char *argv[]) {
    (void)argc;  // unused
    (void)argv;  // unused
    
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Please run this command in a directory containing configure.ac\n");
        return 1;
    }

    printf("Building project...\n\n");

    // Check if configure script exists
    if (!file_exists("configure")) {
        printf("Running autogen.sh to generate configure script...\n");
        
        // Try autogen.sh if it exists
        if (file_exists("autogen.sh")) {
            if (execute_command("./autogen.sh") != 0) {
                fprintf(stderr, "Error: autogen.sh failed\n");
                return 1;
            }
        } else {
            // Fall back to autoreconf
            if (execute_command("autoreconf --install") != 0) {
                fprintf(stderr, "Error: autoreconf failed\n");
                return 1;
            }
        }
        printf("\n");
    }

    // Check if Makefile exists (need to run configure)
    if (!file_exists("Makefile")) {
        printf("Running configure...\n");
        if (execute_command("./configure") != 0) {
            fprintf(stderr, "Error: configure failed\n");
            return 1;
        }
        printf("\n");
    }

    // Run make
    printf("Running make...\n");
    if (execute_command("make") != 0) {
        fprintf(stderr, "Error: make failed\n");
        return 1;
    }

    printf("\n✓ Build completed successfully!\n");
    return 0;
}
