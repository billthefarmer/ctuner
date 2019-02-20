#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([tuner], [1.06])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([tuner.cpp])

# Checks for programs.
AC_LANG(C++)
AC_PROG_CXX
PKG_PROG_PKG_CONFIG
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([pow])
AC_CHECK_HEADER_STDBOOL

# Checks for libraries.
PKG_CHECK_MODULES([GTK],[gtk+-3.0])
PKG_CHECK_MODULES([ALSA],[alsa])

# Checks for header files.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
