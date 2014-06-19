# Only build man pages if configure found the HELP2MAN tool
if HAVE_HELP2MAN
man_MANS = file2brl.1
endif

CLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# We are not distributing the man pages otherwise we would need rules
# such as below. The user will have to install help2man if they want
# man pages.
# file2brl.1: $(top_srcdir)/tools/file2brl.c $(common_mandeps)
# 	 cd ../liblouisutd && $(MAKE) $(AM_MAKEFLAGS) liblouisutd.la
# 	 cd ../gnulib && $(MAKE) $(AM_MAKEFLAGS) libgnu.la
# 	 cd ../tools && $(MAKE) $(AM_MAKEFLAGS) file2brl$(EXEEXT)
# 	$(HELP2MAN) ../tools/file2brl$(EXEEXT) --info-page=$(PACKAGE) --output=$@

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
file2brl.1: $(top_srcdir)/tools/file2brl.c $(common_mandeps)
	$(HELP2MAN) ../tools/file2brl$(EXEEXT) --info-page=$(PACKAGE) --output=$@
