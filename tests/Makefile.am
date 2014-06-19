SUBDIRS = test_suite

AM_CPPFLAGS =					\
	-I$(top_srcdir)/liblouisutdml		\
	-I$(top_srcdir)/gnulib

LDADD = \
	$(top_builddir)/liblouisutdml/liblouisutdml.la \
	$(top_builddir)/gnulib/libgnu.la \
	$(BASE_DEPENDENCIES_LIBS) \
	$(LTLIBINTL)

AM_CFLAGS =	\
	$(BASE_DEPENDENCIES_CFLAGS)

AM_LDFLAGS =	\
	$(BASE_DEPENDENCIES_LDFLAGS)

check_PROGRAMS = \
	testTranslateString\
	tripleTrans

dist_check_SCRIPTS = run_test_suite.sh

testTranslateString_SOURCES = testTranslateString.c
tripleTrans_SOURCES = tripleTrans.c

XFAIL_TESTS =					\
	testTranslateString

TESTS =                   \
	$(check_PROGRAMS)     \
	$(dist_check_SCRIPTS)

TESTS_ENVIRONMENT =									\
	TESTS_DIR=$(top_srcdir)								\
	LD_LIBRARY_PATH=$(abs_top_builddir)/liblouisutdml/.libs:$$LD_LIBRARY_PATH	\
	PATH=$(abs_top_builddir)/tools:$$PATH
