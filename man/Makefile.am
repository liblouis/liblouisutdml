# Only build man pages if configure found the HELP2MAN tool
if HAVE_HELP2MAN
man_MANS = utdml2brl.1 msword2brl.1 rtf2brl.1 pdf2brl.1
endif

CLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# We are not distributing the man pages otherwise we would need rules
# such as below. The user will have to install help2man if they want
# man pages.
# utdml2brl.1: $(top_srcdir)/tools/utdml2brl.c $(common_mandeps)
# 	 cd ../liblouisutd && $(MAKE) $(AM_MAKEFLAGS) liblouisutd.la
# 	 cd ../gnulib && $(MAKE) $(AM_MAKEFLAGS) libgnu.la
# 	 cd ../tools && $(MAKE) $(AM_MAKEFLAGS) utdml2brl$(EXEEXT)
# 	$(HELP2MAN) ../tools/utdml2brl$(EXEEXT) --info-page=$(PACKAGE) --output=$@

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
utdml2brl.1: $(top_srcdir)/tools/utdml2brl.c $(common_mandeps)
	$(HELP2MAN) ../tools/utdml2brl$(EXEEXT) --info-page=$(PACKAGE) --output=$@

msword2brl.1: $(top_srcdir)/tools/msword2brl $(common_mandeps)
	$(HELP2MAN) ../tools/msword2brl --info-page=$(PACKAGE) --output=$@

rtf2brl.1: $(top_srcdir)/tools/rtf2brl $(common_mandeps)
	$(HELP2MAN) ../tools/rtf2brl --info-page=$(PACKAGE) --output=$@

pdf2brl.1: $(top_srcdir)/tools/pdf2brl $(common_mandeps)
	$(HELP2MAN) ../tools/pdf2brl --info-page=$(PACKAGE) --output=$@

