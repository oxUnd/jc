#include "jc.h"
#include "utils.h"
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef __APPLE__
#define HAVE_LLDB 1
#define DEBUGGER "lldb"
#else
#ifdef __linux__
#define HAVE_GDB 1
#define DEBUGGER "gdb"
#else
#define DEBUGGER "gdb"
#endif
#endif

#ifdef HAVE_LLDB
static void print_lldb_usage(const char *executable) {
    printf("\nTo debug with lldb:\n");
    printf("  lldb %s\n", executable);
    printf("  (lldb) run\n");
    printf("  ... program crashes ...\n");
    printf("  (lldb) bt        # Show backtrace\n");
    printf("  (lldb) frame select <n>  # Select a frame\n");
    printf("  (lldb) print <var>       # Print variable\n");
    printf("  (lldb) quit\n");
    printf("\nOr run directly with backtrace:\n");
    printf("  lldb -o run -o bt %s\n", executable);
}
#endif

#ifdef HAVE_GDB
static void print_gdb_usage(const char *executable) {
    printf("\nTo debug with gdb:\n");
    printf("  gdb %s\n", executable);
    printf("  (gdb) run\n");
    printf("  ... program crashes ...\n");
    printf("  (gdb) bt         # Show backtrace\n");
    printf("  (gdb) frame <n>  # Select a frame\n");
    printf("  (gdb) print <var>  # Print variable\n");
    printf("  (gdb) quit\n");
    printf("\nOr run directly with backtrace:\n");
    printf("  gdb -ex run -ex bt %s\n", executable);
}
#endif

int cmd_bt(int argc, char *argv[]) {
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        return 1;
    }

    // Ensure project is built
    if (!file_exists("Makefile")) {
        printf("Project not built. Building first...\n");
        if (cmd_build(0, NULL) != 0) {
            return 1;
        }
    }

    // Find the executable
    char executable[PATH_MAX];
    const char *search_dirs[] = {"src", ".", NULL};
    
    int found = 0;
    for (int i = 0; search_dirs[i] != NULL; i++) {
        if (find_executable(search_dirs[i], executable, sizeof(executable)) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        fprintf(stderr, "Error: Could not find executable\n");
        return 1;
    }

    // Check if core dump exists
    int has_core = file_exists("core");
    
#ifdef HAVE_LLDB
    printf("Using lldb debugger...\n");
    printf("Executable: %s\n", executable);
    
    if (has_core) {
        printf("Core dump found: core\n");
        printf("\nLoading core dump in lldb...\n");
        
        char cmd[PATH_MAX * 2];
        snprintf(cmd, sizeof(cmd), "lldb %s -c core", executable);
        
        printf("Run 'bt' in lldb to see the backtrace\n");
        printf("----------------------------------------\n");
        return system(cmd) == 0 ? 0 : 1;
    } else {
        // Run with debugger
        char cmd[PATH_MAX * 2];
        int offset = snprintf(cmd, sizeof(cmd), "lldb -o run -o bt");
        
        // Add any additional arguments
        for (int i = 1; i < argc; i++) {
            offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -o 'settings set target.run-args %s'", argv[i]);
        }
        
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " %s", executable);
        
        printf("\nRunning with lldb...\n");
        printf("----------------------------------------\n");
        int ret = system(cmd);
        
        if (ret != 0) {
            print_lldb_usage(executable);
        }
        return ret == 0 ? 0 : 1;
    }
#elif defined(HAVE_GDB)
    printf("Using gdb debugger...\n");
    printf("Executable: %s\n", executable);
    
    if (has_core) {
        printf("Core dump found: core\n");
        printf("\nLoading core dump in gdb...\n");
        
        char cmd[PATH_MAX * 2];
        snprintf(cmd, sizeof(cmd), "gdb %s core", executable);
        
        printf("Run 'bt' in gdb to see the backtrace\n");
        printf("----------------------------------------\n");
        return system(cmd) == 0 ? 0 : 1;
    } else {
        // Run with debugger
        char cmd[PATH_MAX * 2];
        snprintf(cmd, sizeof(cmd), "gdb -ex run -ex bt %s", executable);
        
        printf("\nRunning with gdb...\n");
        printf("----------------------------------------\n");
        int ret = system(cmd);
        
        if (ret != 0) {
            print_gdb_usage(executable);
        }
        return ret == 0 ? 0 : 1;
    }
#else
    fprintf(stderr, "Error: No debugger found (lldb or gdb required)\n");
    return 1;
#endif
}
