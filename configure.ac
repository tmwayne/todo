#
# ------------------------------------------------------------------------------
# configure.ac
# ------------------------------------------------------------------------------
#
# Tyler Wayne (c) 2022
#

AC_PREREQ([2.69])
AC_INIT([todo], [0.1], [tylerwayne3@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])

LT_PREREQ([2.2.])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/todo/todo.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_YACC

# Checks for libraries.
AC_SEARCH_LIBS([dlopen], [dl])
AC_SEARCH_LIBS([initscr], [cursesw curses])
AC_SEARCH_LIBS([stdscr], [cursesw curses term])
AC_SEARCH_LIBS([sqlite3_open_v2], [sqlite3])

# Checks for header files.
AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([])

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  src/common/Makefile
  src/backend/Makefile
  src/delim-reader/Makefile
  src/todo/Makefile
  test/Makefile
  test/unit/Makefile])

AC_OUTPUT
