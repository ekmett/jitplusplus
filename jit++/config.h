/* jit++/config.h.  Generated from config.h.in by configure.  */
/* jit++/config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef INCLUDED_JITPP_CONFIG
#define INCLUDED_JITPP_CONFIG
// #include <boost/config.hpp>

/* Simplified version checking for GCC */
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif


/* Encapsulate a number of optional GNU attributes */
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


/* define if your compiler has __attribute__((cold)) */
/* #undef HAVE_ATTRIBUTE_COLD */

/* define if the Boost library is available */
#define HAVE_BOOST 

/* define if the Boost::Unit_Test_Framework library is available */
#define HAVE_BOOST_UNIT_TEST_FRAMEWORK 

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <gflags/gflags.h> header file. */
#define HAVE_GFLAGS_GFLAGS_H 1

/* Define to 1 if you have the <glog/logging.h> header file. */
#define HAVE_GLOG_LOGGING_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `gflags' library (-lgflags). */
#define HAVE_LIBGFLAGS 1

/* Define to 1 if you have the `udis86' library (-ludis86). */
/* #undef HAVE_LIBUDIS86 */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* define if the compiler implements namespaces */
#define HAVE_NAMESPACES 1

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <udis86.h> header file. */
#define HAVE_UDIS86_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Name of package */
#define PACKAGE "jit--"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "ekmett@gmail.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "jit++"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "jit++ 0.0.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "jit--"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.0"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* If the compiler supports a TLS storage class define it to that here */
#define TLS __thread

/* Version number of package */
#define VERSION "0.0.0"

/* Define if using udis86 as a disassembler. */
#define WITH_UDIS86 1

#endif // INCLUDED_JITPP_CONFIG
