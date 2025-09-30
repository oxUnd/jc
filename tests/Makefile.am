if ENABLE_TESTS

# Google Test based tests
check_PROGRAMS = test_jc

test_jc_SOURCES = \
    test_main.cpp \
    test_utils.cpp \
    ../src/utils.c

test_jc_CPPFLAGS = -I$(top_srcdir)/src -std=c++11
test_jc_CXXFLAGS = -Wall -Wextra -g
test_jc_LDFLAGS = -lgtest -lgtest_main -pthread

TESTS = test_jc

endif
