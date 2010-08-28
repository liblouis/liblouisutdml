INCLUDES =		\
	-I$(top_srcdir)/liblouisutdml \
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
	file2brl \
	lbu_devonly

bin_SCRIPTS = \
	msword2brl \
	rtf2brl \
	pdf2brl

CLEANFILES = $(bin_SCRIPTS)

file2brl_SOURCES = file2brl.c

lbu_devonly_SOURCES = lbu_devonly.c
