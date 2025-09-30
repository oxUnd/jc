#include "jc.h"
#include "utils.h"
#include <libgen.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Forward declarations
static int test_add(const char *source_file);
static int test_remove(const char *source_file);
static int test_run(const char *test_file);
static void print_test_usage(void);
static char *generate_test_template(const char *basename);
static int create_initial_test_makefile(void);
static int update_test_makefile(const char *test_file);
static int remove_from_test_makefile(const char *test_file);

// Print usage information for test command
static void print_test_usage(void) {
    printf("Usage: jc test <subcommand> [options]\n\n");
    printf("Subcommands:\n");
    printf("  add <file>         Create a test file for the given source file\n");
    printf("  remove <file>      Remove the test file for the given source file\n");
    printf("  run [test_file]    Run tests (all tests if no file specified)\n\n");
    printf("Examples:\n");
    printf("  jc test add src/utils.c       # Creates tests/test_utils.c\n");
    printf("  jc test remove src/utils.c    # Removes tests/test_utils.c\n");
    printf("  jc test run                   # Run all tests\n");
    printf("  jc test run test_utils        # Run specific test\n\n");
}

// Generate test template content
static char *generate_test_template(const char *basename) {
    static char template[4096];
    
    snprintf(template, sizeof(template),
        "#include <check.h>\n"
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "// Test: Example test case for %s\n"
        "START_TEST(test_example) {\n"
        "    // TODO: Add your test implementation\n"
        "    ck_assert_int_eq(1, 1);\n"
        "}\n"
        "END_TEST\n"
        "\n"
        "// Create test suite\n"
        "Suite *%s_suite(void) {\n"
        "    Suite *s;\n"
        "    TCase *tc_core;\n"
        "    \n"
        "    s = suite_create(\"%s\");\n"
        "    \n"
        "    // Core test case\n"
        "    tc_core = tcase_create(\"Core\");\n"
        "    tcase_add_test(tc_core, test_example);\n"
        "    suite_add_tcase(s, tc_core);\n"
        "    \n"
        "    return s;\n"
        "}\n"
        "\n"
        "// Main function\n"
        "int main(void) {\n"
        "    int number_failed;\n"
        "    Suite *s;\n"
        "    SRunner *sr;\n"
        "    \n"
        "    s = %s_suite();\n"
        "    sr = srunner_create(s);\n"
        "    \n"
        "    // Run tests\n"
        "    srunner_run_all(sr, CK_NORMAL);\n"
        "    number_failed = srunner_ntests_failed(sr);\n"
        "    srunner_free(sr);\n"
        "    \n"
        "    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;\n"
        "}\n",
        basename, basename, basename, basename);
    
    return template;
}

// Create initial tests/Makefile.am if it doesn't exist
static int create_initial_test_makefile(void) {
    const char *makefile_path = "tests/Makefile.am";
    const char *initial_content = 
        "if ENABLE_TESTS\n"
        "\n"
        "# Check framework based tests\n"
        "check_PROGRAMS =\n"
        "\n"
        "TESTS =\n"
        "\n"
        "endif\n";
    
    if (write_file(makefile_path, initial_content) != 0) {
        return 1;
    }
    
    printf("✓ Created tests/Makefile.am\n");
    return 0;
}

// Update tests/Makefile.am to include new test
static int update_test_makefile(const char *test_file) {
    const char *makefile_path = "tests/Makefile.am";
    
    if (!file_exists(makefile_path)) {
        if (create_initial_test_makefile() != 0) {
            fprintf(stderr, "Error: Failed to create tests/Makefile.am\n");
            return 1;
        }
    }
    
    // Read current Makefile.am content
    char *content = read_file(makefile_path);
    if (!content) {
        fprintf(stderr, "Error: Failed to read tests/Makefile.am\n");
        return 1;
    }
    
    // Extract test program name (without .c extension)
    char test_prog[256];
    strncpy(test_prog, test_file, sizeof(test_prog) - 1);
    test_prog[sizeof(test_prog) - 1] = '\0';
    char *ext = strstr(test_prog, ".c");
    if (ext) *ext = '\0';
    
    // Check if test is already in the makefile
    if (strstr(content, test_prog) != NULL) {
        printf("✓ Test '%s' is already in tests/Makefile.am\n", test_prog);
        free(content);
        return 0;
    }
    
    // Add the test program to check_PROGRAMS and TESTS
    char new_content[8192];
    char *output = new_content;
    char *line_start = content;
    int added_to_programs = 0;
    int added_to_tests = 0;
    int added_program_entry = 0;
    
    while (*line_start) {
        char *line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }
        
        // Copy the current line, but modify if needed
        size_t len = line_end - line_start;
        
        // Add to check_PROGRAMS line
        if (!added_to_programs && strstr(line_start, "check_PROGRAMS") != NULL && strstr(line_start, "check_PROGRAMS") < line_end) {
            // Insert test program name
            memcpy(output, line_start, len);
            output += len;
            output += sprintf(output, " %s", test_prog);
            if (*line_end == '\n') {
                *output++ = '\n';
            }
            added_to_programs = 1;
        }
        // Add program definition after check_PROGRAMS line
        else if (added_to_programs && !added_program_entry && *line_start == '\n') {
            // Add new test program definition
            *output++ = '\n';
            output += sprintf(output, "%s_SOURCES = %s\n", test_prog, test_file);
            output += sprintf(output, "%s_CFLAGS = -I$(top_srcdir)/src $(CHECK_CFLAGS) -Wall -Wextra -g\n", test_prog);
            output += sprintf(output, "%s_LDADD = $(CHECK_LIBS)\n", test_prog);
            added_program_entry = 1;
        }
        // Add to TESTS line
        else if (!added_to_tests && strstr(line_start, "TESTS =") != NULL && strstr(line_start, "TESTS =") < line_end) {
            // Insert test program name
            memcpy(output, line_start, len);
            output += len;
            output += sprintf(output, " %s", test_prog);
            if (*line_end == '\n') {
                *output++ = '\n';
            }
            added_to_tests = 1;
        }
        else {
            // Copy the line as is
            memcpy(output, line_start, len);
            output += len;
            if (*line_end == '\n') {
                *output++ = '\n';
            }
        }
        
        if (*line_end == '\n') {
            line_start = line_end + 1;
        } else {
            break;
        }
    }
    
    *output = '\0';
    
    // Write updated content
    if (write_file(makefile_path, new_content) != 0) {
        fprintf(stderr, "Error: Failed to update tests/Makefile.am\n");
        free(content);
        return 1;
    }
    
    printf("✓ Updated tests/Makefile.am to include %s\n", test_prog);
    free(content);
    return 0;
}

// Remove test from tests/Makefile.am
static int remove_from_test_makefile(const char *test_file) {
    const char *makefile_path = "tests/Makefile.am";
    
    if (!file_exists(makefile_path)) {
        return 0; // Nothing to remove
    }
    
    // Extract test program name
    char test_prog[256];
    strncpy(test_prog, test_file, sizeof(test_prog) - 1);
    test_prog[sizeof(test_prog) - 1] = '\0';
    char *ext = strstr(test_prog, ".c");
    if (ext) *ext = '\0';
    
    // Read current content
    char *content = read_file(makefile_path);
    if (!content) {
        return 1;
    }
    
    // Remove test from content
    char new_content[8192];
    char *output = new_content;
    char *line_start = content;
    int skip_next_empty = 0;
    
    while (*line_start) {
        char *line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }
        
        size_t len = line_end - line_start;
        int should_copy = 1;
        
        // Check if this line is related to the test program
        // Skip lines: test_prog_SOURCES, test_prog_CFLAGS, test_prog_LDADD
        if (len > 0 && strncmp(line_start, test_prog, strlen(test_prog)) == 0 &&
            (strstr(line_start, "_SOURCES") || strstr(line_start, "_CFLAGS") || 
             strstr(line_start, "_LDADD"))) {
            should_copy = 0;
            skip_next_empty = 1;  // Skip the following empty line if any
        }
        // Remove from check_PROGRAMS line
        else if (strstr(line_start, "check_PROGRAMS") != NULL && strstr(line_start, test_prog) != NULL) {
            // Reconstruct line without test_prog
            char temp_line[512];
            strncpy(temp_line, line_start, len);
            temp_line[len] = '\0';
            
            char *prog_pos = strstr(temp_line, test_prog);
            if (prog_pos) {
                // Remove test_prog and any surrounding spaces
                char new_line[512] = "";
                size_t before_len = prog_pos - temp_line;
                strncat(new_line, temp_line, before_len);
                
                char *after = prog_pos + strlen(test_prog);
                while (*after == ' ') after++;  // Skip spaces after test_prog
                
                // Remove trailing space before test_prog if needed
                size_t new_len = strlen(new_line);
                if (new_len > 0 && new_line[new_len - 1] == ' ') {
                    new_line[new_len - 1] = '\0';
                }
                
                strcat(new_line, after);
                output += sprintf(output, "%s\n", new_line);
                should_copy = 0;
            }
        }
        // Remove from TESTS line
        else if (strstr(line_start, "TESTS =") != NULL && strstr(line_start, test_prog) != NULL) {
            // Reconstruct line without test_prog
            char temp_line[512];
            strncpy(temp_line, line_start, len);
            temp_line[len] = '\0';
            
            char *test_pos = strstr(temp_line, test_prog);
            if (test_pos) {
                // Remove test_prog and any surrounding spaces
                char new_line[512] = "";
                size_t before_len = test_pos - temp_line;
                strncat(new_line, temp_line, before_len);
                
                char *after = test_pos + strlen(test_prog);
                while (*after == ' ') after++;  // Skip spaces after test_prog
                
                // Remove trailing space before test_prog if needed
                size_t new_len = strlen(new_line);
                if (new_len > 0 && new_line[new_len - 1] == ' ') {
                    new_line[new_len - 1] = '\0';
                }
                
                strcat(new_line, after);
                output += sprintf(output, "%s\n", new_line);
                should_copy = 0;
            }
        }
        // Skip empty lines after test program definitions
        else if (skip_next_empty && len == 0) {
            should_copy = 0;
            skip_next_empty = 0;
        }
        
        // Copy the line if we should
        if (should_copy) {
            skip_next_empty = 0;
            memcpy(output, line_start, len);
            output += len;
            if (*line_end == '\n') {
                *output++ = '\n';
            }
        }
        
        if (*line_end == '\n') {
            line_start = line_end + 1;
        } else {
            break;
        }
    }
    
    *output = '\0';
    
    // Write updated content
    if (write_file(makefile_path, new_content) != 0) {
        free(content);
        return 1;
    }
    
    free(content);
    return 0;
}

// Add a test file for a source file
static int test_add(const char *source_file) {
    // Check if we're in an automake project
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Run this command from a project created with 'jc new'\n");
        return 1;
    }
    
    // Check if source file exists
    if (!file_exists(source_file)) {
        fprintf(stderr, "Error: Source file '%s' does not exist\n", source_file);
        return 1;
    }
    
    // Extract basename without extension
    char *source_copy = strdup(source_file);
    char *base = basename(source_copy);
    char basename_no_ext[256];
    strncpy(basename_no_ext, base, sizeof(basename_no_ext) - 1);
    basename_no_ext[sizeof(basename_no_ext) - 1] = '\0';
    
    // Remove .c extension if present
    char *dot = strrchr(basename_no_ext, '.');
    if (dot) *dot = '\0';
    
    // Create test file path
    char test_file_path[PATH_MAX];
    snprintf(test_file_path, sizeof(test_file_path), "tests/test_%s.c", basename_no_ext);
    
    // Check if test file already exists
    if (file_exists(test_file_path)) {
        fprintf(stderr, "Error: Test file '%s' already exists\n", test_file_path);
        free(source_copy);
        return 1;
    }
    
    // Ensure tests directory exists
    if (!directory_exists("tests")) {
        if (create_directory("tests") != 0) {
            fprintf(stderr, "Error: Failed to create tests directory\n");
            free(source_copy);
            return 1;
        }
    }
    
    // Generate test template
    char *template_content = generate_test_template(basename_no_ext);
    
    // Write test file
    if (write_file(test_file_path, template_content) != 0) {
        fprintf(stderr, "Error: Failed to write test file '%s'\n", test_file_path);
        free(source_copy);
        return 1;
    }
    
    printf("✓ Created test file: %s\n", test_file_path);
    
    // Update tests/Makefile.am
    char test_filename[256];
    snprintf(test_filename, sizeof(test_filename), "test_%s.c", basename_no_ext);
    update_test_makefile(test_filename);
    
    printf("\nNext steps:\n");
    printf("  1. Edit %s to add your test cases\n", test_file_path);
    printf("  2. Run 'jc test run' to execute tests\n");
    
    free(source_copy);
    return 0;
}

// Remove a test file for a source file
static int test_remove(const char *source_file) {
    // Extract basename without extension
    char *source_copy = strdup(source_file);
    char *base = basename(source_copy);
    char basename_no_ext[256];
    strncpy(basename_no_ext, base, sizeof(basename_no_ext) - 1);
    basename_no_ext[sizeof(basename_no_ext) - 1] = '\0';
    
    // Remove .c extension if present
    char *dot = strrchr(basename_no_ext, '.');
    if (dot) *dot = '\0';
    
    // Create test file path
    char test_file_path[PATH_MAX];
    snprintf(test_file_path, sizeof(test_file_path), "tests/test_%s.c", basename_no_ext);
    
    // Check if test file exists
    if (!file_exists(test_file_path)) {
        fprintf(stderr, "Error: Test file '%s' does not exist\n", test_file_path);
        free(source_copy);
        return 1;
    }
    
    // Remove from Makefile.am first
    char test_filename[256];
    snprintf(test_filename, sizeof(test_filename), "test_%s.c", basename_no_ext);
    remove_from_test_makefile(test_filename);
    
    // Delete the test file
    if (unlink(test_file_path) != 0) {
        fprintf(stderr, "Error: Failed to delete test file '%s'\n", test_file_path);
        free(source_copy);
        return 1;
    }
    
    printf("✓ Removed test file: %s\n", test_file_path);
    printf("✓ Updated tests/Makefile.am\n");
    
    free(source_copy);
    return 0;
}

// Run tests
static int test_run(const char *test_file) {
    // Check if we're in an automake project
    if (!is_automake_project()) {
        fprintf(stderr, "Error: Not in an automake project directory\n");
        fprintf(stderr, "Run this command from a project created with 'jc new'\n");
        return 1;
    }
    
    // Check if tests directory exists
    if (!directory_exists("tests")) {
        fprintf(stderr, "Error: No tests directory found\n");
        fprintf(stderr, "Use 'jc test add <file>' to create tests\n");
        return 1;
    }
    
    if (test_file) {
        // Run specific test
        char test_path[PATH_MAX];
        
        // Handle different input formats
        if (strstr(test_file, "test_") == test_file) {
            // Input is like "test_utils"
            snprintf(test_path, sizeof(test_path), "tests/%s", test_file);
        } else if (strstr(test_file, ".c") != NULL) {
            // Input is like "test_utils.c"
            char *base = basename((char*)test_file);
            char *dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            snprintf(test_path, sizeof(test_path), "tests/%s", base);
        } else {
            // Input is like "utils" - add test_ prefix
            snprintf(test_path, sizeof(test_path), "tests/test_%s", test_file);
        }
        
        // Check if test binary exists
        if (!file_exists(test_path)) {
            fprintf(stderr, "Error: Test '%s' not found\n", test_path);
            fprintf(stderr, "Run 'make check' first to build tests\n");
            return 1;
        }
        
        printf("Running test: %s\n\n", test_path);
        return execute_command(test_path);
        
    } else {
        // Run all tests using make check
        printf("Running all tests...\n\n");
        return execute_command("make check");
    }
}

// Main command handler
int cmd_test(int argc, char *argv[]) {
    if (argc < 2) {
        print_test_usage();
        return 1;
    }
    
    const char *subcommand = argv[1];
    
    if (strcmp(subcommand, "add") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'add' requires a source file argument\n\n");
            print_test_usage();
            return 1;
        }
        return test_add(argv[2]);
        
    } else if (strcmp(subcommand, "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'remove' requires a source file argument\n\n");
            print_test_usage();
            return 1;
        }
        return test_remove(argv[2]);
        
    } else if (strcmp(subcommand, "run") == 0) {
        const char *test_file = (argc >= 3) ? argv[2] : NULL;
        return test_run(test_file);
        
    } else {
        fprintf(stderr, "Error: Unknown subcommand '%s'\n\n", subcommand);
        print_test_usage();
        return 1;
    }
}
