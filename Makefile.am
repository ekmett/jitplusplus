# include the macros from the m4 folder automatically as needed
REPOSITORY_HTTP = http://comonad.com/jit++/
REPOSITORY_SSH = slipwave@comonad.com:/home/slipwave/info/jit++

# autoconf macros are in m4
ACLOCAL_AMFLAGS = -I m4

# we need to include the boost c++ flags and link flags
AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(PTHREAD_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(PTHREAD_CPPFLAGS) -L/usr/local/lib

# scripts for maintainer use
noinst_SCRIPTS=bin/autogen.sh

distdir=$(top_srcdir)/dist/jit++-$(VERSION)

# install the documentation
docdir = $(prefix)/share/doc/jit++-$(VERSION)
dist_doc_DATA = COPYING ChangeLog INSTALL README

# install the headers 
nobase_noinst_HEADERS = \
	jit++/config.h \
	jit++/common.h \
	jit++/decoder.h \
	jit++/exceptions.h \
	jit++/flags.h \
	jit++/group_1.h
	jit++/group_2.h \
	jit++/group_3.h \
	jit++/group_4.h \
	jit++/interpreter.h \
	jit++/interpreter_internal.h \
	jit++/memory.h \
	jit++/mutex.h \
	jit++/semantics.h \
	jit++/tracer.h

nobase_include_HEADERS = \
        jit++.h \
	jit++/application.h 

# build the runtime system
lib_LTLIBRARIES = libjit++.la
libjit___la_LIBADD = $(LIBOBJS)
libjit___la_LDFLAGS = -release $(PACKAGE_VERSION) -version-info $(VERSION_INFO)
libjit___la_SOURCES = \
	jit++/flags.cc \
	jit++/interpreter.cc \
	jit++/application.cc \
	jit++/decoder.cc \
	jit++/exceptions.cc \
	jit++/interpreter_internal.cc \
	jit++/tracer.cc \
	jit++/tracer_start.S \
	jit++/memory.cc \
	jit++/mutex.cc \
	jit++/group_1.cc \ 
	jit++/locked_opcodes.cc \
	jit++/opcodes.cc


# test suite
noinst_PROGRAMS = test_suite test_interpreter

test_suite_DEPENDENCIES = libjit++.la
test_suite_SOURCES = test/options.cc test/t_suite.cc
test_suite_LDADD = libjit++.la $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(PTHREAD_LIBS) $(LIB_OBJS)

test_interpreter_DEPENDENCIES = libjit++.la
test_interpreter_SOURCES = test/t_interpreter.cc
test_interpreter_LDADD = libjit++.la $(PTHREAD_LIBS) $(LIB_OBJS)

# clear out caches
maintainer-clean-local:
	rm -rf autom4te.cache aclocal.m4

# clean up after ourselves to the point where there is nothing left
super-clean: maintainer-clean-local
	rm -rf ./configure config.log config/config.h config.status Makefile Makefile.in ChangeLog

# darcs support
EXTRA_DIST = _darcs bin/autogen.sh

ChangeLog: _darcs
	echo "Automatically generated from 'darcs changes' by 'make ChangeLog'" > ChangeLog
	echo >> ChangeLog
	$(DARCS) changes >> ChangeLog

push:
	darcs push -v -a $(REPOSITORY_SSH)

pull:
	darcs pull $(REPOSITORY_SSH)

pull-http:
	darcs pull $(REPOSITORY_HTTP)

record: 
	darcs record

tests: test_interpreter test_suite
	./test_interpreter -logtostderr -v 1
