#include <gtest/gtest.h>

// Main function is provided by gtest_main library
// This file exists for potential global test setup if needed

class JCTestEnvironment : public ::testing::Environment {
public:
    virtual ~JCTestEnvironment() {}
    
    virtual void SetUp() override {
        // Global test setup
    }
    
    virtual void TearDown() override {
        // Global test cleanup
    }
};

// Register the environment
::testing::Environment* const jc_env = 
    ::testing::AddGlobalTestEnvironment(new JCTestEnvironment);
