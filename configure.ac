AC_PREREQ([2.69])
AC_INIT([jc], [1.0.0], [oxUnd])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB

# Check for required programs
AC_CHECK_PROG([AUTOCONF], [autoconf], [autoconf], [no])
AC_CHECK_PROG([AUTOMAKE], [automake], [automake], [no])
AC_CHECK_PROG([MAKE], [make], [make], [no])

if test "$AUTOCONF" = "no"; then
    AC_MSG_ERROR([autoconf is required but not found])
fi

if test "$AUTOMAKE" = "no"; then
    AC_MSG_ERROR([automake is required but not found])
fi

# Check for debugger
AC_CHECK_PROG([LLDB], [lldb], [lldb], [no])
AC_CHECK_PROG([GDB], [gdb], [gdb], [no])

if test "$LLDB" != "no"; then
    AC_DEFINE([HAVE_LLDB], [1], [Define if lldb is available])
    AC_SUBST([DEBUGGER], [lldb])
elif test "$GDB" != "no"; then
    AC_DEFINE([HAVE_GDB], [1], [Define if gdb is available])
    AC_SUBST([DEBUGGER], [gdb])
fi

# Check for Check testing framework (optional for testing)
AC_ARG_ENABLE([tests],
    AS_HELP_STRING([--enable-tests], [Enable building tests with Check framework]),
    [enable_tests=$enableval],
    [enable_tests=no])

if test "x$enable_tests" = "xyes"; then
    PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])

# Checks for header files
AC_CHECK_HEADERS([stdlib.h string.h unistd.h sys/stat.h sys/types.h])

# Checks for typedefs, structures, and compiler characteristics
AC_TYPE_SIZE_T

# Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([mkdir strdup])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
])

AC_OUTPUT
