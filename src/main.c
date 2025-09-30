#include "jc.h"
#include "utils.h"

void print_usage(const char *program_name) {
    printf("jc - A modern C project management tool\n");
    printf("Version: %s\n\n", JC_VERSION);
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  new <project>   Create a new automake project\n");
    printf("  add <type> <target> Add files, directories, or dependencies\n");
    printf("  build           Build the current project\n");
    printf("  run             Run the current project\n");
    printf("  install         Install the current project\n");
    printf("  clean           Clean build artifacts\n");
    printf("  bt              Show backtrace (debug crashed program)\n");
    printf("  help            Show this help message\n");
    printf("  version         Show version information\n");
    printf("\n");
}

void print_version(void) {
    printf("jc version %s\n", JC_VERSION);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "new") == 0) {
        return cmd_new(argc - 1, argv + 1);
    } else if (strcmp(command, "add") == 0) {
        return cmd_add(argc - 1, argv + 1);
    } else if (strcmp(command, "build") == 0) {
        return cmd_build(argc - 1, argv + 1);
    } else if (strcmp(command, "run") == 0) {
        return cmd_run(argc - 1, argv + 1);
    } else if (strcmp(command, "install") == 0) {
        return cmd_install(argc - 1, argv + 1);
    } else if (strcmp(command, "clean") == 0) {
        return cmd_clean(argc - 1, argv + 1);
    } else if (strcmp(command, "bt") == 0) {
        return cmd_bt(argc - 1, argv + 1);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    } else if (strcmp(command, "version") == 0 || strcmp(command, "--version") == 0 || strcmp(command, "-v") == 0) {
        print_version();
        return 0;
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", command);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
