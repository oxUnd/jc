if ENABLE_TESTS

# Check framework based tests
check_PROGRAMS = test_jc

test_jc_SOURCES = \
    test_utils.c \
    ../src/utils.c

test_jc_CFLAGS = -I$(top_srcdir)/src $(CHECK_CFLAGS) -Wall -Wextra -g
test_jc_LDADD = $(CHECK_LIBS)

TESTS = test_jc

endif