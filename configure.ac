# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([jitney],[0.0.1],[ekmett@gmail.com])
AC_SUBST(JITNEY_COPYRIGHT,["Copyright (c) 2008 Edward Kmett. All Rights Reserved."])
AC_COPYRIGHT([Copyright (c) 2008 Edward Kmett. All Rights Reserved.])

AC_CONFIG_AUX_DIR([bin])
AC_CONFIG_HEADER([jitney/config.h])
AC_CONFIG_SRCDIR([.])

# add include guards to make jitney/config.hpp idempotent
AH_TOP([#ifndef INCLUDED_JITNEY_CONFIG
#define INCLUDED_JITNEY_CONFIG
// #include <boost/config.hpp>])
AH_BOTTOM([#endif // INCLUDED_JITNEY_CONFIG])

# find darcs version control system (autogenerates ChangeLog)
AC_ARG_VAR([DARCS],[darcs executable])
AC_CHECK_PROG([DARCS],[darcs],[/usr/bin/darcs])

# use C++
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_LIBTOOL

# check to see if we support gcc style attributes
AX_C_HAVE_ATTRIBUTE

# do we support the cold attribute added in 4.3?
AX_C_HAVE_ATTRIBUTE_COLD

AM_INIT_AUTOMAKE([foreign subdir-objects])

# libraries:
AX_BOOST_BASE([1.36])

# used for the unit tests
AX_BOOST_UNIT_TEST_FRAMEWORK

# pthreads are used pretty much everywhere
AC_CHECK_HEADERS_ONCE(pthread.h)
AX_CXX_PTHREAD

# udis86 used for disassembly on x86 and x86-64 for debug purposes
AC_CHECK_HEADERS_ONCE(udis86.h)
AX_UDIS86

# google gflags used for all runtime system command line options
AC_CHECK_HEADERS(gflags/gflags.h)
AC_CHECK_LIB(gflags, main) # _ZN6google7GetArgvEv)

# google logging framework
AC_CHECK_HEADERS_ONCE(glog/logging.h)
AC_CHECK_LIB(glog, ZN6google10LogMessage4FailEv)

# tune to current architecture
# AX_GCC_ARCHFLAG([no],[CPPFLAGS="$CPPFLAGS $ax_cv_gcc_archflag"])

# do we have namespaces? and what namespace is the STL in?
AC_CXX_NAMESPACES

if test "$ac_cv_cxx_namespaces" = no; then
  AC_ERROR([Namespace support required])
fi

# figure out how to handle TLS
AX_TLS 

AH_VERBATIM([GCC_VERSION],
[/* Simplified version checking for GCC */
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif
])

# abuse GNU attributes. NB. the __attribute__ macro below could encompass all of these uses.
AH_VERBATIM([HAVE_ATTRIBUTE],
[/* Encapsulate a number of optional GNU attributes */
#ifndef HAVE_ATTRIBUTE
# define __attribute__(x)
#endif

# define NORETURN __attribute__ ((noreturn))
# define ALWAYS_INLINE __attribute__ ((always_inline)) inline
# define UNUSED __attribute__ ((unused))
# define ALIGNED(x) __attribute__ ((aligned(x))

# ifdef HAVE_ATTRIBUTE_COLD
#  define NORETURN_COLD __attribute__ ((noreturn,cold))
#  define COLD __attribute__ ((cold))
# else 
#  define NORETURN_COLD NORETURN
#  define COLD
# endif
])

AC_OUTPUT([Makefile])
