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

bin_PROGRAMS = \
	file2brl

file2brl_SOURCES = file2brl.c

