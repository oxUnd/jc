#include <check.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "utils.h"

// Global test directory for fixture
static char test_dir[256];

// Setup function - runs before each test
static void setup(void) {
    strcpy(test_dir, "/tmp/jc_test_XXXXXX");
    if (mkdtemp(test_dir) == NULL) {
        ck_abort_msg("Failed to create temporary directory");
    }
}

// Teardown function - runs after each test
static void teardown(void) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", test_dir);
    system(cmd);
}

// Test: Create directory
START_TEST(test_create_directory) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testdir", test_dir);

    ck_assert_int_eq(create_directory(path), 0);
    ck_assert_int_eq(directory_exists(path), 1);

    // Creating again should succeed (idempotent)
    ck_assert_int_eq(create_directory(path), 0);
}
END_TEST

// Test: File exists
START_TEST(test_file_exists) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testfile.txt", test_dir);

    ck_assert_int_eq(file_exists(path), 0);

    // Create the file
    FILE *f = fopen(path, "w");
    ck_assert_ptr_nonnull(f);
    fprintf(f, "test content");
    fclose(f);

    ck_assert_int_eq(file_exists(path), 1);
}
END_TEST

// Test: Write and read file
START_TEST(test_write_and_read_file) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testfile.txt", test_dir);

    const char *content = "Hello, World!";
    ck_assert_int_eq(write_file(path, content), 0);
    ck_assert_int_eq(file_exists(path), 1);

    char *read_content = read_file(path);
    ck_assert_ptr_nonnull(read_content);
    ck_assert_str_eq(read_content, content);

    free(read_content);
}
END_TEST

// Test: Copy file
START_TEST(test_copy_file) {
    char src_path[512];
    char dst_path[512];
    snprintf(src_path, sizeof(src_path), "%s/source.txt", test_dir);
    snprintf(dst_path, sizeof(dst_path), "%s/dest.txt", test_dir);

    const char *content = "Test content for copy";
    ck_assert_int_eq(write_file(src_path, content), 0);

    ck_assert_int_eq(copy_file(src_path, dst_path), 0);
    ck_assert_int_eq(file_exists(dst_path), 1);

    char *copied_content = read_file(dst_path);
    ck_assert_ptr_nonnull(copied_content);
    ck_assert_str_eq(copied_content, content);

    free(copied_content);
}
END_TEST

// Test: Directory exists
START_TEST(test_directory_exists) {
    ck_assert_int_eq(directory_exists(test_dir), 1);
    ck_assert_int_eq(directory_exists("/nonexistent/directory/path"), 0);
}
END_TEST

// Test: Execute command quiet
START_TEST(test_execute_command_quiet) {
    // Test successful command
    ck_assert_int_eq(execute_command_quiet("true"), 0);

    // Test failing command
    ck_assert_int_ne(execute_command_quiet("false"), 0);
}
END_TEST

// Test: Is automake project (standalone test without fixture)
START_TEST(test_is_automake_project) {
    // Save current directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    // Create a temporary directory
    char temp_dir[256];
    strcpy(temp_dir, "/tmp/jc_automake_test_XXXXXX");
    if (mkdtemp(temp_dir) == NULL) {
        ck_abort_msg("Failed to create temporary directory");
    }

    // Change to test directory
    chdir(temp_dir);

    // Should not be an automake project
    ck_assert_int_eq(is_automake_project(), 0);

    // Create configure.ac
    FILE *f = fopen("configure.ac", "w");
    ck_assert_ptr_nonnull(f);
    fprintf(f, "AC_INIT([test], [1.0])\n");
    fclose(f);

    // Now it should be an automake project
    ck_assert_int_eq(is_automake_project(), 1);

    // Restore directory and cleanup
    chdir(cwd);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}
END_TEST

// Test: regex_replace - basic functionality
START_TEST(test_regex_replace_basic) {
    const char *input = "Hello, World!";
    const char *pattern = "World";
    const char *replacement = "Universe";

    char *result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "Hello, Universe!");

    free(result);

    // Test multiple replacements
    input = "Hello, World! Hello, World!";
    result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "Hello, Universe! Hello, Universe!");

    free(result);

    // Test no match
    input = "Hello, Earth!";
    result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "Hello, Earth!");

    free(result);
}
END_TEST

// Test: regex_replace - capture groups
START_TEST(test_regex_replace_capture_groups) {
    // Test simple capture group
    const char *input = "Hello, World!";
    const char *pattern = "(Hello), (World)!";
    const char *replacement = "$2, $1!";

    char *result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "World, Hello!");

    free(result);

    // Test multiple capture groups and occurrences
    input = "John Doe (42) and Jane Smith (35)";
    pattern = "([a-zA-Z]+) ([a-zA-Z]+) \\(([0-9]+)\\)";
    replacement = "$2, $1: $3 years old";

    result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    printf("result: %s\n", result);
    ck_assert_str_eq(result, "Doe, John: 42 years old and Smith, Jane: 35 years old");

    free(result);

    // Test mixed capture groups and literal text
    input = "Item: 123, Quantity: 456";
    pattern = "Item: ([0-9]+), Quantity: ([0-9]+)";
    replacement = "Order - Item #$1, Count: $2 units";

    result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "Order - Item #123, Count: 456 units");

    free(result);

    // Test invalid capture group number
    input = "Hello, World!";
    pattern = "(Hello), (World)!";
    replacement = "$3, $4!";

    result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, ", !");  // Invalid groups should be replaced with empty string

    free(result);
}
END_TEST

// Test: regex_replace - edge cases
START_TEST(test_regex_replace_edge_cases) {
    // Test NULL inputs
    ck_assert_ptr_null(regex_replace(NULL, "pattern", "replacement"));
    ck_assert_ptr_null(regex_replace("input", NULL, "replacement"));
    ck_assert_ptr_null(regex_replace("input", "pattern", NULL));

    // Test empty strings
    const char *input = "";
    const char *pattern = "a";
    const char *replacement = "b";

    char *result = regex_replace(input, pattern, replacement);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(result, "");

    free(result);

    // Test empty pattern
    input = "Hello";
    pattern = "";

    result = regex_replace(input, pattern, replacement);
    // Pattern compilation may fail, but function should handle it gracefully
    // If it returns NULL, that's acceptable behavior
    if (result) {
        free(result);
    }
}
END_TEST

// Create test suite
Suite *utils_suite(void) {
    Suite *s;
    TCase *tc_core;
    TCase *tc_standalone;

    s = suite_create("Utils");

    // Core test case with fixture
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_create_directory);
    tcase_add_test(tc_core, test_file_exists);
    tcase_add_test(tc_core, test_write_and_read_file);
    tcase_add_test(tc_core, test_copy_file);
    tcase_add_test(tc_core, test_directory_exists);
    tcase_add_test(tc_core, test_execute_command_quiet);
    suite_add_tcase(s, tc_core);

    // Standalone test case without fixture
    tc_standalone = tcase_create("Standalone");
    tcase_add_test(tc_standalone, test_is_automake_project);
    tcase_add_test(tc_standalone, test_regex_replace_basic);
    tcase_add_test(tc_standalone, test_regex_replace_capture_groups);
    tcase_add_test(tc_standalone, test_regex_replace_edge_cases);
    suite_add_tcase(s, tc_standalone);

    return s;
}



// Main function
int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = utils_suite();
    sr = srunner_create(s);

    // Run tests
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
