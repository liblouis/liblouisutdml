# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(liblouisutdml, 2.3.0, john.boyer@abilitiessoft.com)
AC_CONFIG_SRCDIR([liblouisutdml/semantics.c])
AC_CONFIG_HEADER([liblouisutdml/config.h])
# AM_MAINTAINER_MODE

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

# increment if the interface has additions, changes, removals.
LIBLOUISUTDML_CURRENT=6

# increment any time the source changes; set to
# 0 if you increment CURRENT
LIBLOUISUTDML_REVISION=5

# increment if any interfaces have been added; set to 0
# if any interfaces have been changed or removed. removal has
# precedence over adding, so set to 0 if both happened.
LIBLOUISUTDML_AGE=0

AC_SUBST(LIBLOUISUTDML_CURRENT)
AC_SUBST(LIBLOUISUTDML_REVISION)
AC_SUBST(LIBLOUISUTDML_AGE)

# Checks for programs.
AC_PROG_CC

# gnulib
gl_EARLY
gl_INIT

# Checks for more programs.
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
# AC_PROG_RANLIB
AC_ISC_POSIX
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

# GNU help2man creates man pages from --help output; in many cases,
# this is sufficient, and obviates the need to maintain man pages
# separately. However, some developers do not have it so we do not
# make its use mandatory.
AC_CHECK_PROG([HELP2MAN], [help2man], [help2man])
AM_CONDITIONAL([HAVE_HELP2MAN], [test x$HELP2MAN = xhelp2man])

AC_PATH_PROG(PKG_CONFIG, pkg-config)

PKG_CHECK_MODULES(BASE_DEPENDENCIES, [liblouis, libxml-2.0])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h jni.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset])

case $host in
 *mingw* | *cygwin*)
 LDFLAGS="$LDFLAGS -avoid-version"
 CFLAGS="$CFLAGS -Wl,--add-stdcall-alias"
 ;;
esac

AC_CONFIG_FILES([Makefile
	 gnulib/Makefile
	doc/Makefile
	java/Makefile
	java/src/Makefile
	java/src/org/Makefile
	java/src/org/liblouis/Makefile
	man/Makefile
	lbu_files/Makefile
	liblouisutdml/Makefile
	 liblouisutdml.pc
	tests/Makefile
	tools/Makefile
	windows/Makefile
	windows/include/Makefile
	 m4/Makefile])

AC_OUTPUT

