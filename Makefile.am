# include the macros from the m4 folder automatically as needed
REPOSITORY_HTTP = http://comonad.com/jitney/
REPOSITORY_SSH = slipwave@comonad.com:/home/slipwave/info/jitney

# autoconf macros are in m4
ACLOCAL_AMFLAGS = -I m4

# we need to include the boost cpp flags and link flags
AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(PTHREAD_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(PTHREAD_CPPFLAGS) -L/usr/local/lib

# scripts for maintainer use
noinst_SCRIPTS=autogen.sh

distdir=$(top_srcdir)/dist/$(PACKAGE)-$(VERSION)

# install the documentation
docdir = $(prefix)/share/doc/$(PACKAGE)-$(VERSION)
dist_doc_DATA = COPYING ChangeLog INSTALL README

# install the headers 
#nobase_inst_HEADERS once i make an instdir
nobase_noinst_HEADERS = jitney/interpreter.hpp jitney/memory.hpp jitney/tracer.hpp

# build the runtime system
lib_LIBRARIES = libjitney.a
libjitney_a_SOURCES = jitney/interpreter.cpp jitney/memory.cpp jitney/tracer.cpp
libjitney_a_LIBADD = $(LIBOBJS)

# test suite
noinst_PROGRAMS = tests
tests_DEPENDENCIES = libjitney.a
tests_SOURCES = test/t_main.cpp test/t_branch.cpp
tests_LDADD = libjitney.a \
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
