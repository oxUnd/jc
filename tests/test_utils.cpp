#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
#include "utils.h"
}

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        strcpy(test_dir, "/tmp/jc_test_XXXXXX");
        if (mkdtemp(test_dir) == nullptr) {
            FAIL() << "Failed to create temporary directory";
        }
    }

    void TearDown() override {
        // Clean up test directory
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", test_dir);
        system(cmd);
    }

    char test_dir[256];
};

TEST_F(UtilsTest, CreateDirectory) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testdir", test_dir);
    
    EXPECT_EQ(create_directory(path), 0);
    EXPECT_TRUE(directory_exists(path));
    
    // Creating again should succeed (idempotent)
    EXPECT_EQ(create_directory(path), 0);
}

TEST_F(UtilsTest, FileExists) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testfile.txt", test_dir);
    
    EXPECT_FALSE(file_exists(path));
    
    // Create the file
    FILE *f = fopen(path, "w");
    ASSERT_NE(f, nullptr);
    fprintf(f, "test content");
    fclose(f);
    
    EXPECT_TRUE(file_exists(path));
}

TEST_F(UtilsTest, WriteAndReadFile) {
    char path[512];
    snprintf(path, sizeof(path), "%s/testfile.txt", test_dir);
    
    const char *content = "Hello, World!";
    EXPECT_EQ(write_file(path, content), 0);
    EXPECT_TRUE(file_exists(path));
    
    char *read_content = read_file(path);
    ASSERT_NE(read_content, nullptr);
    EXPECT_STREQ(read_content, content);
    
    free(read_content);
}

TEST_F(UtilsTest, CopyFile) {
    char src_path[512];
    char dst_path[512];
    snprintf(src_path, sizeof(src_path), "%s/source.txt", test_dir);
    snprintf(dst_path, sizeof(dst_path), "%s/dest.txt", test_dir);
    
    const char *content = "Test content for copy";
    EXPECT_EQ(write_file(src_path, content), 0);
    
    EXPECT_EQ(copy_file(src_path, dst_path), 0);
    EXPECT_TRUE(file_exists(dst_path));
    
    char *copied_content = read_file(dst_path);
    ASSERT_NE(copied_content, nullptr);
    EXPECT_STREQ(copied_content, content);
    
    free(copied_content);
}

TEST_F(UtilsTest, DirectoryExists) {
    EXPECT_TRUE(directory_exists(test_dir));
    EXPECT_FALSE(directory_exists("/nonexistent/directory/path"));
}

TEST_F(UtilsTest, ExecuteCommandQuiet) {
    // Test successful command
    EXPECT_EQ(execute_command_quiet("true"), 0);
    
    // Test failing command
    EXPECT_NE(execute_command_quiet("false"), 0);
}

TEST(UtilsTestStandalone, IsAutomakeProject) {
    // Save current directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    
    // Create a temporary directory
    char test_dir[256];
    strcpy(test_dir, "/tmp/jc_automake_test_XXXXXX");
    if (mkdtemp(test_dir) == nullptr) {
        FAIL() << "Failed to create temporary directory";
    }
    
    // Change to test directory
    chdir(test_dir);
    
    // Should not be an automake project
    EXPECT_FALSE(is_automake_project());
    
    // Create configure.ac
    FILE *f = fopen("configure.ac", "w");
    ASSERT_NE(f, nullptr);
    fprintf(f, "AC_INIT([test], [1.0])\n");
    fclose(f);
    
    // Now it should be an automake project
    EXPECT_TRUE(is_automake_project());
    
    // Restore directory and cleanup
    chdir(cwd);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", test_dir);
    system(cmd);
}

// Test main (using gtest_main)
// No need to define main() as we link with -lgtest_main
