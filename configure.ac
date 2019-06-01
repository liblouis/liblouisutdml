# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.68)
AC_INIT([liblouisutdml], [2.7.1], [liblouis-liblouisxml@freelists.org], [liblouisutdml], [http://www.liblouis.org])
AC_CONFIG_SRCDIR([liblouisutdml/semantics.c])
AC_CONFIG_HEADER([liblouisutdml/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

# increment if the interface has additions, changes, removals.
LIBLOUISUTDML_CURRENT=9

# increment any time the source changes; set to
# 0 if you increment CURRENT
LIBLOUISUTDML_REVISION=0

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

PKG_CHECK_MODULES(BASE_DEPENDENCIES, [liblouis >= 3.0.0, libxml-2.0])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

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

# Disable building the Java bindings on request
AC_ARG_ENABLE(
	[java_bindings], 
	[  --disable-java-bindings do not build the Java bindings],
	[case "${enableval}" in
	      yes) java_bindings=yes ;;
	      no)  java_bindings=no ;;
	      *) AC_MSG_ERROR([bad value ${enableval} for --disable-java-bindings]) ;;
	esac],[java_bindings=yes])
AM_CONDITIONAL([JAVA_BINDINGS], [test x$java_bindings == xyes])
AC_MSG_CHECKING([whether Java bindings should be built])
AC_MSG_RESULT($java_bindings)

if test x$java_bindings == xyes ; then

   # make sure jni include dirs are found
   # using a macro from the autoconf archive
   AX_JNI_INCLUDE_DIR

   for JNI_INCLUDE_DIR in $JNI_INCLUDE_DIRS
       do
           CPPFLAGS="$CPPFLAGS -I$JNI_INCLUDE_DIR"
	   found_jni_h=yes
       done
fi

if test x$java_bindings == xyes -a x$found_jni_h != xyes ; then
    AC_MSG_ERROR([Please install jni properly or disable the Java bindings with --disable-java-bindings.])
fi

AC_CONFIG_FILES([Makefile
	gnulib/Makefile
	doc/Makefile
	java/Makefile
	java/lib/Makefile
	java/src/Makefile
	java/src/org/Makefile
	java/src/org/liblouis/Makefile
	java/tests/Makefile
	java/tests/org/Makefile
	java/tests/org/liblouis/Makefile
	java/testdata/Makefile
	java/testdata/liblouis/Makefile
	java/testdata/liblouis/tables/Makefile
	java/testdata/liblouisutdml/Makefile
	java/testdata/liblouisutdml/lbu_files/Makefile
	man/Makefile
	lbu_files/Makefile
	liblouisutdml/Makefile
	liblouisutdml.pc
	tests/Makefile
	tests/firstLineIndent.test/Makefile
	tests/footer_01.test/Makefile
	tests/footer_02.test/Makefile
	tests/hyphenation.test/Makefile
	tests/mixed_content.test/Makefile
	tests/orphanControl_01.test/Makefile
	tests/orphanControl_02.test/Makefile
	tests/page_numbering.test/Makefile
	tests/pagenum.test/Makefile
	tests/prehyphenation.test/Makefile
	tests/printPageNumberRange_01.test/Makefile
	tests/printPageNumberRange_02.test/Makefile
	tests/printPageNumberRange_03.test/Makefile
	tests/endNotes.test/Makefile
	tests/test_mathml_woluwe/Makefile
	tests/test_mathml_woluwe/test_001.test/Makefile
	tests/test_mathml_woluwe/test_002.test/Makefile
	tests/test_mathml_woluwe/test_003.test/Makefile
	tests/test_mathml_woluwe/test_004.test/Makefile
	tests/test_mathml_woluwe/test_005.test/Makefile
	tests/test_mathml_woluwe/test_006.test/Makefile
	tests/test_mathml_woluwe/test_007.test/Makefile
	tests/test_mathml_woluwe/test_008.test/Makefile
	tests/test_mathml_woluwe/test_009.test/Makefile
	tests/test_mathml_woluwe/test_010.test/Makefile
	tests/test_mathml_woluwe/test_011.test/Makefile
	tests/test_mathml_woluwe/test_012.test/Makefile
	tests/test_mathml_woluwe/test_013.test/Makefile
	tests/test_mathml_woluwe/test_014.test/Makefile
	tests/test_mathml_woluwe/test_015.test/Makefile
	tests/test_mathml_woluwe/test_016.test/Makefile
	tests/test_mathml_woluwe/test_017.test/Makefile
	tests/test_mathml_woluwe/test_018.test/Makefile
	tests/test_mathml_woluwe/test_019.test/Makefile
	tests/test_mathml_woluwe/test_020.test/Makefile
	tests/test_mathml_woluwe/test_021.test/Makefile
	tests/test_mathml_woluwe/test_022.test/Makefile
	tests/test_mathml_woluwe/test_023.test/Makefile
	tests/test_mathml_woluwe/test_024.test/Makefile
	tests/test_mathml_woluwe/test_025.test/Makefile
	tests/test_mathml_woluwe/test_026.test/Makefile
	tests/test_mathml_woluwe/test_027.test/Makefile
	tests/test_mathml_woluwe/test_028.test/Makefile
	tests/test_mathml_woluwe/test_029.test/Makefile
	tests/test_mathml_woluwe/test_030.test/Makefile
	tests/test_mathml_woluwe/test_031.test/Makefile
	tests/test_mathml_woluwe/test_032.test/Makefile
	tests/test_mathml_woluwe/test_033.test/Makefile
	tests/test_mathml_woluwe/test_034.test/Makefile
	tests/test_mathml_woluwe/test_035.test/Makefile
	tests/test_mathml_woluwe/test_036.test/Makefile
	tests/test_mathml_woluwe/test_037.test/Makefile
	tests/test_mathml_woluwe/test_038.test/Makefile
	tests/test_mathml_woluwe/test_039.test/Makefile
	tests/test_mathml_woluwe/test_040.test/Makefile
	tests/test_mathml_woluwe/test_041.test/Makefile
	tests/test_mathml_woluwe/test_042.test/Makefile
	tests/test_mathml_woluwe/test_043.test/Makefile
	tests/test_mathml_woluwe/test_044.test/Makefile
	tests/test_mathml_woluwe/test_045.test/Makefile
	tests/test_mathml_woluwe/test_046.test/Makefile
	tests/test_mathml_woluwe/test_047.test/Makefile
	tests/test_mathml_woluwe/test_048.test/Makefile
	tests/test_mathml_woluwe/test_049.test/Makefile
	tests/test_mathml_woluwe/test_050.test/Makefile
	tests/test_mathml_woluwe/test_051.test/Makefile
	tests/test_mathml_woluwe/test_052.test/Makefile
	tests/test_mathml_woluwe/test_053.test/Makefile
	tests/test_mathml_woluwe/test_054.test/Makefile
	tests/test_mathml_woluwe/test_055.test/Makefile
	tests/test_mathml_woluwe/test_056.test/Makefile
	tests/test_mathml_woluwe/test_057.test/Makefile
	tests/test_mathml_woluwe/test_058.test/Makefile
	tests/test_mathml_woluwe/test_059.test/Makefile
	tests/test_mathml_woluwe/test_060.test/Makefile
	tests/test_mathml_woluwe/test_061.test/Makefile
	tests/test_mathml_woluwe/test_062.test/Makefile
	tests/test_mathml_woluwe/test_063.test/Makefile
	tests/test_mathml_woluwe/test_064.test/Makefile
	tests/test_mathml_woluwe/test_065.test/Makefile
	tests/test_mathml_woluwe/test_066.test/Makefile
	tests/test_mathml_woluwe/test_067.test/Makefile
	tests/test_mathml_woluwe/test_068.test/Makefile
	tests/test_mathml_woluwe/test_069.test/Makefile
	tests/test_mathml_woluwe/test_070.test/Makefile
	tests/test_mathml_woluwe/test_071.test/Makefile
	tests/test_mathml_woluwe/test_072.test/Makefile
	tests/test_mathml_woluwe/test_073.test/Makefile
	tests/test_mathml_woluwe/test_074.test/Makefile
	tests/test_mathml_woluwe/test_075.test/Makefile
	tests/test_mathml_woluwe/test_076.test/Makefile
	tests/test_mathml_woluwe/test_077.test/Makefile
	tests/test_mathml_woluwe/test_078.test/Makefile
	tests/test_mathml_woluwe/test_079.test/Makefile
	tests/test_mathml_woluwe/test_080.test/Makefile
	tests/test_mathml_woluwe/test_081.test/Makefile
	tests/test_mathml_woluwe/test_082.test/Makefile
	tests/test_mathml_woluwe/test_083.test/Makefile
	tests/test_mathml_woluwe/test_084.test/Makefile
	tests/test_mathml_woluwe/test_085.test/Makefile
	tests/test_mathml_woluwe/test_086.test/Makefile
	tests/test_mathml_woluwe/test_087.test/Makefile
	tests/test_mathml_woluwe/test_088.test/Makefile
	tests/test_mathml_woluwe/test_089.test/Makefile
	tests/test_mathml_woluwe/test_090.test/Makefile
	tests/test_mathml_woluwe/test_091.test/Makefile
	tests/test_mathml_woluwe/test_092.test/Makefile
	tests/test_mathml_woluwe/test_093.test/Makefile
	tests/test_mathml_woluwe/test_094.test/Makefile
	tests/test_mathml_woluwe/test_095.test/Makefile
	tests/test_mathml_woluwe/test_096.test/Makefile
	tests/test_mathml_woluwe/test_097.test/Makefile
	tests/test_mathml_woluwe/test_098.test/Makefile
	tests/test_mathml_woluwe/test_099.test/Makefile
	tests/test_mathml_woluwe/test_100.test/Makefile
	tests/test_mathml_woluwe/test_101.test/Makefile
	tests/test_mathml_woluwe/test_102.test/Makefile
	tests/test_mathml_woluwe/test_103.test/Makefile
	tests/test_mathml_woluwe/test_104.test/Makefile
	tests/test_mathml_woluwe/test_105.test/Makefile
	tests/test_mathml_woluwe/test_106.test/Makefile
	tests/test_mathml_woluwe/test_107.test/Makefile
	tests/test_mathml_woluwe/test_108.test/Makefile
	tests/test_mathml_woluwe/test_109.test/Makefile
	tests/test_mathml_woluwe/test_110.test/Makefile
	tests/test_mathml_woluwe/test_111.test/Makefile
	tests/test_mathml_woluwe/test_112.test/Makefile
	tests/test_mathml_woluwe/test_113.test/Makefile
	tests/test_mathml_woluwe/test_114.test/Makefile
	tests/test_mathml_woluwe/test_115.test/Makefile
	tests/test_mathml_woluwe/test_116.test/Makefile
	tests/test_mathml_woluwe/test_117.test/Makefile
	tests/test_mathml_woluwe/test_118.test/Makefile
	tests/test_mathml_woluwe/test_119.test/Makefile
	tools/Makefile
	windows/Makefile
	windows/include/Makefile
	m4/Makefile])

AC_OUTPUT

