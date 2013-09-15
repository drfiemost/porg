#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#

AC_INIT([porg], [svn], [http://porg.sourceforge.net])

RELEASEDATE="28 June 2010"

AC_CONFIG_SRCDIR([porg/main.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])

AM_INIT_AUTOMAKE([-Wall foreign no-installinfo no-texinfo.tex])
AM_MAINTAINER_MODE

AC_CONFIG_FILES([ \
	Makefile \
	config-bot.h \
	lib/Makefile \
	lib/porg/Makefile \
	lib/porg-log/Makefile \
	porg/Makefile \
	grop/Makefile \
	scripts/Makefile \
	doc/Makefile \
	doc/porgrc \
	doc/porgrc.5 \
	doc/porg.8 \
	doc/porgball.8 \
	doc/superporg.8 \
])

AC_GNU_SOURCE

#=======================
# Set configure options
#=======================

if test "$PACKAGE_VERSION" = svn; then
	RELEASEDATE="`date +'%d %B %Y'`"
else
	AC_DEFINE([NDEBUG], [1], [Disable libc assertions])
	AC_DEFINE([G_DISABLE_ASSERT], [1], [Disable glib assertions])
fi

AC_DEFINE_UNQUOTED([RELEASEDATE], ["$RELEASEDATE"], [Release date])
AC_SUBST([RELEASEDATE])

INCLUDE="/"
AC_DEFINE_UNQUOTED([INCLUDE], ["$INCLUDE"], [List of paths to scan])
AC_SUBST([INCLUDE])

EXCLUDE="/dev:/tmp"
for dir in /proc /selinux /sys /usr/share/info/dir; do
	test -e "$dir" && EXCLUDE="$EXCLUDE:$dir"
done
AC_DEFINE_UNQUOTED([EXCLUDE], ["$EXCLUDE"], [List of paths to skip])
AC_SUBST([EXCLUDE])

AC_ARG_WITH([porg-logdir],
	[AC_HELP_STRING([--with-porg-logdir=DIR],
		[porg log directory [default=/var/log/porg]])],
	[LOGDIR=$withval], [LOGDIR=/var/log/porg]
)
AC_DEFINE_UNQUOTED([LOGDIR], ["$LOGDIR"], [Default porg logs directory])
AC_SUBST([LOGDIR])

AC_ARG_ENABLE([grop],
	AC_HELP_STRING([--disable-grop], [do not install grop (the GUI of porg)]),
	[enable_grop=$enableval], [enable_grop=yes])
AM_CONDITIONAL([ENABLE_GROP], [test "$enable_grop" = yes])

AC_ARG_ENABLE([scripts],
	[AC_HELP_STRING([--disable-scripts],
		[disable installation of auxiliary scripts (see README)])],
	[enable_scripts=$enableval], [enable_scripts=yes])
AM_CONDITIONAL([ENABLE_SCRIPTS], [test "$enable_scripts" = yes])

#================
# Check programs
#================

AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX

if test "$enable_grop" = yes; then
	AC_CHECK_PROG([has_gzip], [gzip], [yes], [no])
	if test "$has_gzip" = no; then
		AC_MSG_ERROR([*** gzip program not found ***])
	fi
fi

#============
# Check libs
#============

AC_SEARCH_LIBS([dlopen], [dl], [], AC_MSG_ERROR([*** dlopen not found ***]))

if test "$enable_grop" = yes; then
	PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0 >= 3.4.0])
fi

#===============
# Check headers
#===============

AC_CHECK_HEADERS_ONCE([regex.h])
AM_CONDITIONAL([HAVE_REGEX_H], [test "$ac_cv_header_regex_h" = yes])

#=============
# Check types
#=============

AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

AC_CHECK_TYPE([off64_t], [unsigned long])
AC_CHECK_TYPE([uint], [unsigned int])
AC_CHECK_TYPE([ulong], [unsigned long])

#================
# Check compiler
#================

MY_CFLAGS=-W
MY_CXXFLAGS=-W

if test "$GCC" = yes; then
	MY_CFLAGS="$MY_CFLAGS -ansi -Wshadow -Wmissing-declarations -Wall"
fi

if test "$GXX" = yes; then
	MY_CXXFLAGS="$MY_CXXFLAGS -ansi -pedantic -Wno-long-long -Wmissing-declarations -fno-operator-names -Wall"
fi
	
AC_SUBST([MY_CXXFLAGS])
AC_SUBST([MY_CFLAGS])

#=============
# Check funcs
#=============

AC_CHECK_FUNCS_ONCE([ \
	getopt_long \
	open64 \
	creat64 \
	truncate64 \
	fopen64 \
	freopen64 \
	setenv
])
AM_CONDITIONAL([HAVE_GETOPT_LONG], [test "$ac_cv_func_getopt_long" = yes])

#========	
# Output
#========

AH_TOP([
#ifndef PORG_CONFIG_H
#define PORG_CONFIG_H 1
])

AH_BOTTOM([
#include "config-bot.h"
#endif
])

# for config-bot.h:
AC_SUBST([ac_includes_default])

AC_OUTPUT

