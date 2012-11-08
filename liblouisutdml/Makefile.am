INCLUDES =		\
	-I$(top_srcdir) \
	-DLBU_PATH=\"$(datadir)/liblouisutdml/lbu_files/\" \
	-DLIBLOUIS_TABLES_PATH=\"$(datadir)/liblouis/tables/\"
liblouisutdmlincludedir = $(includedir)/liblouisutdml

liblouisutdmlinclude_HEADERS = \
	liblouisutdml.h \
	louisutdml.h

lib_LTLIBRARIES = liblouisutdml.la

liblouisutdml_la_CFLAGS =	\
	$(BASE_DEPENDENCIES_CFLAGS)

liblouisutdml_la_LDFLAGS =	\
	-version-info \
	$(LIBLOUISUTDML_CURRENT):$(LIBLOUISUTDML_REVISION):$(LIBLOUISUTDML_AGE) \
	-no-undefined \
	$(BASE_DEPENDENCIES_LIBS) \
	$(LTLIBINTL) 

liblouisutdml_la_SOURCES =			\
	../java/Jliblouisutdml.h \
	../java/Jliblouisutdml.c \
	change_table.c \
	makeContents.c \
	examine_document.c \
	liblouisutdml.c \
	liblouisutdml.h \
	louisutdml.h \
	paths.c \
	readconfig.c \
	semantics.c \
	sem_enum.h \
	sem_names.h \
	transcribe_cdataSection.c \
	transcribe_chemistry.c \
	transcribe_computerCode.c \
	transcribe_document.c \
	transcribe_graphic.c \
	transcribe_math.c \
	transcribe_music.c \
	transcribe_paragraph.c \
	convert_utd.c \
	utd2transinxml.c \
	utd2dsbible.c \
	utd2brf.c \
	utd2pef.c \
	utd2volumes.c \
	transcriber.c

EXTRA_DIST = \
	README \
	make_sem_names.c \
	Makesemnames

