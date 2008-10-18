# include the macros from the m4 folder automatically as needed
REPOSITORY_HTTP = http://comonad.com/jit++/
REPOSITORY_SSH = slipwave@comonad.com:/home/slipwave/info/jit++

# autoconf macros are in m4
ACLOCAL_AMFLAGS = -I m4

# we need to include the boost c++ flags and link flags
AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(PTHREAD_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(PTHREAD_CPPFLAGS) -L/usr/local/lib

# scripts for maintainer use
noinst_SCRIPTS=autogen.sh

distdir=$(top_srcdir)/dist/$(PACKAGE)-$(VERSION)

# install the documentation
docdir = $(prefix)/share/doc/$(PACKAGE)-$(VERSION)
dist_doc_DATA = COPYING ChangeLog INSTALL README

# install the headers 
nobase_noinst_HEADERS = jit++/internal.h
nobase_include_HEADERS = jit++.h jit++/config.h jit++/exception.h jit++/interpreter.h jit++/memory.h jit++/mutex.h jit++/options.h jit++/semantics.h jit++/tracer.h

# build the runtime system
lib_LTLIBRARIES = libjit++.la
libjit___la_SOURCES = jit++/exception.cc jit++/interpreter.cc jit++/memory.cc jit++/mutex.cc jit++/options.cc jit++/tracer.cc
libjit___la_LIBADD = $(LIBOBJS)

# test suite
noinst_PROGRAMS = tests
tests_DEPENDENCIES = libjit++.la
tests_SOURCES = test/options.cc test/suite.cc
tests_LDADD = libjit++.la \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	$(PTHREAD_LIBS) \
	$(LIB_OBJS)

# clear out caches
maintainer-clean-local:
	rm -rf autom4te.cache aclocal.m4

# clean up after ourselves to the point where there is nothing left
super-clean: maintainer-clean-local
	rm -rf ./configure config.log config/config.h config.status Makefile Makefile.in ChangeLog libkata.a 

# darcs support
EXTRA_DIST = _darcs autogen.sh

ChangeLog: _darcs
	echo "Automatically generated from 'darcs changes' by 'make ChangeLog'" > ChangeLog
	echo >> ChangeLog
	$(DARCS) changes >> ChangeLog

push:
	darcs push -v -a $(REPOSITORY_SSH)

pull:
	darcs pull $(REPOSITORY_HTTP)

record: 
	darcs record
