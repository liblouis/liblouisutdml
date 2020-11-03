SUBDIRS = gnulib doc lbu_files liblouisutdml tools tests man m4 java windows

ACLOCAL_AMFLAGS = -I m4 -I gnulib/m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = liblouisutdml.pc

EXTRA_DIST = liblouisutdml.pc README.windows README.mac README

WINDIST_FILES = liblouisutdml-$(VERSION)-win32.zip liblouisutdml-$(VERSION)-win64.zip

.PHONY: distwin
distwin: $(WINDIST_FILES)

liblouisutdml-$(VERSION)-win32.zip: Dockerfile.win32
	docker build -f $< -t liblouisutdml/win32 .
	time=$$(date +'%Y%m%d-%H%M%S') && \
	docker create --name="tmp_$$time" liblouisutdml/win32 && \
	docker cp "tmp_$$time":/usr/src/liblouisutdml/liblouisutdml.zip $@ && \
	docker rm tmp_$$time

liblouisutdml-$(VERSION)-win64.zip: Dockerfile.win64
	docker build -f $< -t liblouisutdml/win64 .
	time=$$(date +'%Y%m%d-%H%M%S') && \
	docker create --name="tmp_$$time" liblouisutdml/win64 && \
	docker cp "tmp_$$time":/usr/src/liblouisutdml/liblouisutdml.zip $@ && \
	docker rm tmp_$$time

clean-local:
	rm -rf $(WINDIST_FILES)

