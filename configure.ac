#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([aloserv], [0.0], [/dev/null])
AC_CONFIG_SRCDIR([src/server.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([subdir-objects])

AC_USE_SYSTEM_EXTENSIONS
# Checks for programs.
AC_PROG_CC

AC_PROG_CC_STDC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netdb.h stddef.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([asprintf],,
               AC_DEFINE([_GNU_SOURCE], [], [Linux requires _GNU_SOURCE for
                          asprintf]))
AC_CHECK_FUNCS([memset socket asprintf],,
               AC_MSG_ERROR([some needed function(s) not found]))

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
