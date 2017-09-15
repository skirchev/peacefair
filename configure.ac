#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([energy], [1.0], [stefan.kirchev@gmail.com])
AM_INIT_AUTOMAKE
LT_INIT

AC_PREFIX_PROGRAM(energy)
AC_CONFIG_SRCDIR([system/energy_com_classes.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LIBTOOL
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memset pow strchr strerror strrchr strstr])
################################################################################

AC_CONFIG_FILES([ \
	Makefile \
	system/Makefile \
	tools/Makefile \
	scripts/Makefile \
	])

AC_OUTPUT
