# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.68)
AC_INIT([liblouisutdml], [2.7.0], [liblouis-liblouisxml@freelists.org], [liblouisutdml], [http://www.liblouis.org])
AC_CONFIG_SRCDIR([liblouisutdml/semantics.c])
AC_CONFIG_HEADER([liblouisutdml/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

# increment if the interface has additions, changes, removals.
LIBLOUISUTDML_CURRENT=8

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

AC_PATH_PROG(PKG_CONFIG, pkg-config)

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
	tests/test_suite/Makefile
	tests/test_suite/test_01/Makefile
	tests/test_suite/test_02/Makefile
	tests/test_suite/test_03/Makefile
	tests/test_suite/test_04/Makefile
	tests/test_suite/test_05/Makefile
	tests/test_suite/test_06/Makefile
	tests/test_suite/test_07/Makefile
	tests/test_suite/test_08/Makefile
	tests/test_suite/test_09/Makefile
	tests/test_suite/test_10/Makefile
	tests/test_suite/test_11/Makefile
	tests/test_suite/test_12/Makefile
	tests/test_suite/test_13/Makefile
	tests/test_suite/test_14/Makefile
	tests/test_suite/test_mathml_woluwe/Makefile
	tests/test_suite/test_mathml_woluwe/test_001/Makefile
	tests/test_suite/test_mathml_woluwe/test_002/Makefile
	tests/test_suite/test_mathml_woluwe/test_003/Makefile
	tests/test_suite/test_mathml_woluwe/test_004/Makefile
	tests/test_suite/test_mathml_woluwe/test_005/Makefile
	tests/test_suite/test_mathml_woluwe/test_006/Makefile
	tests/test_suite/test_mathml_woluwe/test_007/Makefile
	tests/test_suite/test_mathml_woluwe/test_008/Makefile
	tests/test_suite/test_mathml_woluwe/test_009/Makefile
	tests/test_suite/test_mathml_woluwe/test_010/Makefile
	tests/test_suite/test_mathml_woluwe/test_011/Makefile
	tests/test_suite/test_mathml_woluwe/test_012/Makefile
	tests/test_suite/test_mathml_woluwe/test_013/Makefile
	tests/test_suite/test_mathml_woluwe/test_014/Makefile
	tests/test_suite/test_mathml_woluwe/test_015/Makefile
	tests/test_suite/test_mathml_woluwe/test_016/Makefile
	tests/test_suite/test_mathml_woluwe/test_017/Makefile
	tests/test_suite/test_mathml_woluwe/test_018/Makefile
	tests/test_suite/test_mathml_woluwe/test_019/Makefile
	tests/test_suite/test_mathml_woluwe/test_020/Makefile
	tests/test_suite/test_mathml_woluwe/test_021/Makefile
	tests/test_suite/test_mathml_woluwe/test_022/Makefile
	tests/test_suite/test_mathml_woluwe/test_023/Makefile
	tests/test_suite/test_mathml_woluwe/test_024/Makefile
	tests/test_suite/test_mathml_woluwe/test_025/Makefile
	tests/test_suite/test_mathml_woluwe/test_026/Makefile
	tests/test_suite/test_mathml_woluwe/test_027/Makefile
	tests/test_suite/test_mathml_woluwe/test_028/Makefile
	tests/test_suite/test_mathml_woluwe/test_029/Makefile
	tests/test_suite/test_mathml_woluwe/test_030/Makefile
	tests/test_suite/test_mathml_woluwe/test_031/Makefile
	tests/test_suite/test_mathml_woluwe/test_032/Makefile
	tests/test_suite/test_mathml_woluwe/test_033/Makefile
	tests/test_suite/test_mathml_woluwe/test_034/Makefile
	tests/test_suite/test_mathml_woluwe/test_035/Makefile
	tests/test_suite/test_mathml_woluwe/test_036/Makefile
	tests/test_suite/test_mathml_woluwe/test_037/Makefile
	tests/test_suite/test_mathml_woluwe/test_038/Makefile
	tests/test_suite/test_mathml_woluwe/test_039/Makefile
	tests/test_suite/test_mathml_woluwe/test_040/Makefile
	tests/test_suite/test_mathml_woluwe/test_041/Makefile
	tests/test_suite/test_mathml_woluwe/test_042/Makefile
	tests/test_suite/test_mathml_woluwe/test_043/Makefile
	tests/test_suite/test_mathml_woluwe/test_044/Makefile
	tests/test_suite/test_mathml_woluwe/test_045/Makefile
	tests/test_suite/test_mathml_woluwe/test_046/Makefile
	tests/test_suite/test_mathml_woluwe/test_047/Makefile
	tests/test_suite/test_mathml_woluwe/test_048/Makefile
	tests/test_suite/test_mathml_woluwe/test_049/Makefile
	tests/test_suite/test_mathml_woluwe/test_050/Makefile
	tests/test_suite/test_mathml_woluwe/test_051/Makefile
	tests/test_suite/test_mathml_woluwe/test_052/Makefile
	tests/test_suite/test_mathml_woluwe/test_053/Makefile
	tests/test_suite/test_mathml_woluwe/test_054/Makefile
	tests/test_suite/test_mathml_woluwe/test_055/Makefile
	tests/test_suite/test_mathml_woluwe/test_056/Makefile
	tests/test_suite/test_mathml_woluwe/test_057/Makefile
	tests/test_suite/test_mathml_woluwe/test_058/Makefile
	tests/test_suite/test_mathml_woluwe/test_059/Makefile
	tests/test_suite/test_mathml_woluwe/test_060/Makefile
	tests/test_suite/test_mathml_woluwe/test_061/Makefile
	tests/test_suite/test_mathml_woluwe/test_062/Makefile
	tests/test_suite/test_mathml_woluwe/test_063/Makefile
	tests/test_suite/test_mathml_woluwe/test_064/Makefile
	tests/test_suite/test_mathml_woluwe/test_065/Makefile
	tests/test_suite/test_mathml_woluwe/test_066/Makefile
	tests/test_suite/test_mathml_woluwe/test_067/Makefile
	tests/test_suite/test_mathml_woluwe/test_068/Makefile
	tests/test_suite/test_mathml_woluwe/test_069/Makefile
	tests/test_suite/test_mathml_woluwe/test_070/Makefile
	tests/test_suite/test_mathml_woluwe/test_071/Makefile
	tests/test_suite/test_mathml_woluwe/test_072/Makefile
	tests/test_suite/test_mathml_woluwe/test_073/Makefile
	tests/test_suite/test_mathml_woluwe/test_074/Makefile
	tests/test_suite/test_mathml_woluwe/test_075/Makefile
	tests/test_suite/test_mathml_woluwe/test_076/Makefile
	tests/test_suite/test_mathml_woluwe/test_077/Makefile
	tests/test_suite/test_mathml_woluwe/test_078/Makefile
	tests/test_suite/test_mathml_woluwe/test_079/Makefile
	tests/test_suite/test_mathml_woluwe/test_080/Makefile
	tests/test_suite/test_mathml_woluwe/test_081/Makefile
	tests/test_suite/test_mathml_woluwe/test_082/Makefile
	tests/test_suite/test_mathml_woluwe/test_083/Makefile
	tests/test_suite/test_mathml_woluwe/test_084/Makefile
	tests/test_suite/test_mathml_woluwe/test_085/Makefile
	tests/test_suite/test_mathml_woluwe/test_086/Makefile
	tests/test_suite/test_mathml_woluwe/test_087/Makefile
	tests/test_suite/test_mathml_woluwe/test_088/Makefile
	tests/test_suite/test_mathml_woluwe/test_089/Makefile
	tests/test_suite/test_mathml_woluwe/test_090/Makefile
	tests/test_suite/test_mathml_woluwe/test_091/Makefile
	tests/test_suite/test_mathml_woluwe/test_092/Makefile
	tests/test_suite/test_mathml_woluwe/test_093/Makefile
	tests/test_suite/test_mathml_woluwe/test_094/Makefile
	tests/test_suite/test_mathml_woluwe/test_095/Makefile
	tests/test_suite/test_mathml_woluwe/test_096/Makefile
	tests/test_suite/test_mathml_woluwe/test_097/Makefile
	tests/test_suite/test_mathml_woluwe/test_098/Makefile
	tests/test_suite/test_mathml_woluwe/test_099/Makefile
	tests/test_suite/test_mathml_woluwe/test_100/Makefile
	tests/test_suite/test_mathml_woluwe/test_101/Makefile
	tests/test_suite/test_mathml_woluwe/test_102/Makefile
	tests/test_suite/test_mathml_woluwe/test_103/Makefile
	tests/test_suite/test_mathml_woluwe/test_104/Makefile
	tests/test_suite/test_mathml_woluwe/test_105/Makefile
	tests/test_suite/test_mathml_woluwe/test_106/Makefile
	tests/test_suite/test_mathml_woluwe/test_107/Makefile
	tests/test_suite/test_mathml_woluwe/test_108/Makefile
	tests/test_suite/test_mathml_woluwe/test_109/Makefile
	tests/test_suite/test_mathml_woluwe/test_110/Makefile
	tests/test_suite/test_mathml_woluwe/test_111/Makefile
	tests/test_suite/test_mathml_woluwe/test_112/Makefile
	tests/test_suite/test_mathml_woluwe/test_113/Makefile
	tests/test_suite/test_mathml_woluwe/test_114/Makefile
	tests/test_suite/test_mathml_woluwe/test_115/Makefile
	tests/test_suite/test_mathml_woluwe/test_116/Makefile
	tests/test_suite/test_mathml_woluwe/test_117/Makefile
	tests/test_suite/test_mathml_woluwe/test_118/Makefile
	tests/test_suite/test_mathml_woluwe/test_119/Makefile
	tools/Makefile
	windows/Makefile
	windows/include/Makefile
	m4/Makefile])

# generate the shell script to run the test suite and make sure it is executable
AC_CONFIG_FILES([tests/run_test_suite.sh], [chmod +x tests/run_test_suite.sh])

AC_OUTPUT

