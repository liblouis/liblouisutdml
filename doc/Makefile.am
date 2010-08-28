# docdir is only needed if you have Autoconf older than 2.60
docdir = ${datadir}/doc/${PACKAGE}

doc_DATA = \
	copyright-notice \
	liblouisutdml.html \
	liblouisutdml.txt

EXTRA_DIST = \
	copyright-notice \
	liblouisutdml.html \
	liblouisutdml.txt

CLEANFILES = liblouisutdml.html liblouisutdml.txt

info_TEXINFOS = liblouisutdml.texi

# generate one big html file
AM_MAKEINFOHTMLFLAGS = --no-headers --no-split

SUFFIXES                = .txt

.texi.txt:
	$(MAKEINFO) --plaintext $< -o $@

