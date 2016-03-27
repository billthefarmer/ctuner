#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([tuner], [1.04])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([tuner.c])

# Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([GTK],[gtk+-2.0])
PKG_CHECK_MODULES([ALSA],[alsa])

# Checks for header files.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
