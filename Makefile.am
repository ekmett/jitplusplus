# include the macros from the m4 folder automatically as needed
REPOSITORY_HTTP = http://comonad.com/jit++/
REPOSITORY_SSH = slipwave@comonad.com:/home/slipwave/info/jit++

# autoconf macros are in m4
ACLOCAL_AMFLAGS = -I m4

# AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(PTHREAD_CPPFLAGS)
# AM_LDFLAGS = $(BOOST_LDFLAGS) $(PTHREAD_CPPFLAGS) -L/usr/local/lib
AM_CPPFLAGS = $(PTHREAD_CPPFLAGS)
AM_LDFLAGS = $(PTHREAD_CPPFLAGS) -L/usr/local/lib

# scripts for maintainer use
noinst_SCRIPTS=bin/autogen.sh

distdir=$(top_srcdir)/dist/jit++-$(VERSION)

# install the documentation
docdir = $(prefix)/share/doc/jit++-$(VERSION)
dist_doc_DATA = COPYING INSTALL README

# install the headers 
nobase_noinst_HEADERS = \
	jit++/config.h \
	jit++/common.h \
	jit++/exceptions.h \
	jit++/interpreter.h \
	jit++/memory.h \
	jit++/mutex.h \
	jit++/semantics.h \
	jit++/interpreting/decoder.h \
	jit++/interpreting/flags.h \
	jit++/interpreting/group_1.h \
	jit++/interpreting/group_2.h \
	jit++/interpreting/group_3.h \
	jit++/interpreting/group_4.h \
	jit++/interpreting/group_5.h \
	jit++/interpreting/impl.h \
	jit++/interpreting/locked.h \
	jit++/interpreting/misc.h \
	jit++/interpreting/tracer.h

nobase_include_HEADERS = \
        jit++.h \
	jit++/application.h 

# build the runtime system
lib_LTLIBRARIES = libjit++.la
libjit___la_LIBADD = $(LIBOBJS)
libjit___la_LDFLAGS = -release $(PACKAGE_VERSION) -version-info $(VERSION_INFO)
libjit___la_SOURCES = \
	jit++/interpreter.cc \
	jit++/application.cc \
	jit++/exceptions.cc \
	jit++/memory.cc \
	jit++/mutex.cc \
	jit++/interpreting/base.cc \
	jit++/interpreting/decoder.cc \
	jit++/interpreting/flags.cc \
	jit++/interpreting/group_1.cc \
	jit++/interpreting/impl.cc \
	jit++/interpreting/tracer.cc \
	jit++/interpreting/misc.cc \
	jit++/interpreting/tracer_start.S

# test suite
# noinst_PROGRAMS = test_suite test_interpreter
noinst_PROGRAMS = test_interpreter

# test_suite_DEPENDENCIES = libjit++.la
# test_suite_SOURCES = test/options.cc 
# test/t_suite.cc
# test_suite_LDADD = libjit++.la $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(PTHREAD_LIBS) $(LIB_OBJS)

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
EXTRA_DIST = bin/autogen.sh

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

tests: test_interpreter
	./test_interpreter -logtostderr -v 1
