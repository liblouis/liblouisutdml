# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([liblouisutdml],[2.13.0],[liblouis-liblouisxml@freelists.org],[liblouisutdml],[http://www.liblouis.io])
AC_CONFIG_SRCDIR([liblouisutdml/semantics.c])
AC_CONFIG_HEADERS([liblouisutdml/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4 gnulib/m4])
AM_INIT_AUTOMAKE([-Wall dist-zip])

# increment if the interface has additions, changes, removals.
LIBLOUISUTDML_CURRENT=11

# increment any time the source changes; set to
# 0 if you increment CURRENT
LIBLOUISUTDML_REVISION=0

# increment if any interfaces have been added; set to 0
# if any interfaces have been changed or removed. removal has
# precedence over adding, so set to 0 if both happened.
LIBLOUISUTDML_AGE=2

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
AM_PROG_AR
LT_INIT
# see https://www.gnu.org/software/libtool/manual/html_node/LT_005fINIT.html
LT_INIT([win32-dll])

# GNU help2man creates man pages from --help output; in many cases,
# this is sufficient, and obviates the need to maintain man pages
# separately. However, some developers do not have it so we do not
# make its use mandatory.
AC_CHECK_PROG([HELP2MAN], [help2man], [help2man])
AM_CONDITIONAL([HAVE_HELP2MAN], [test x$HELP2MAN = xhelp2man])

PKG_CHECK_MODULES(BASE_DEPENDENCIES, [liblouis >= 3.28.0, libxml-2.0])

# Checks for libraries.

# Checks for header files.
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
	tests/boxline.test/Makefile
	tests/firstLineIndent.test/Makefile
	tests/footer_01.test/Makefile
	tests/footer_02.test/Makefile
	tests/hyphenation.test/Makefile
	tests/minSyllableWidth.test/Makefile
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
	tests/mathml_nemeth/Makefile
	tests/mathml_nemeth/general_001.test/Makefile
	tests/mathml_nemeth/general_002.test/Makefile
	tests/mathml_nemeth/general_003.test/Makefile
	tests/mathml_nemeth/mfrac_01.test/Makefile
	tests/mathml_nemeth/mfrac_02.test/Makefile
	tests/mathml_nemeth/mfrac_03.test/Makefile
	tests/mathml_nemeth/mfrac_04.test/Makefile
	tests/mathml_nemeth/mfrac_05.test/Makefile
	tests/mathml_nemeth/mfrac_06.test/Makefile
	tests/mathml_nemeth/mfrac_07.test/Makefile
	tests/mathml_nemeth/mixed_scripts_01.test/Makefile
	tests/mathml_nemeth/mixed_scripts_02.test/Makefile
	tests/mathml_nemeth/mixed_scripts_03.test/Makefile
	tests/mathml_nemeth/mixed_scripts_04.test/Makefile
	tests/mathml_nemeth/mixed_scripts_05.test/Makefile
	tests/mathml_nemeth/mixed_scripts_06.test/Makefile
	tests/mathml_nemeth/mixed_scripts_07.test/Makefile
	tests/mathml_nemeth/mmultiscripts_01.test/Makefile
	tests/mathml_nemeth/mmultiscripts_02.test/Makefile
	tests/mathml_nemeth/mmultiscripts_03.test/Makefile
	tests/mathml_nemeth/mmultiscripts_04.test/Makefile
	tests/mathml_nemeth/mmultiscripts_05.test/Makefile
	tests/mathml_nemeth/mmultiscripts_06.test/Makefile
	tests/mathml_nemeth/mmultiscripts_07.test/Makefile
	tests/mathml_nemeth/mover_01.test/Makefile
	tests/mathml_nemeth/mover_02.test/Makefile
	tests/mathml_nemeth/mover_03.test/Makefile
	tests/mathml_nemeth/mover_04.test/Makefile
	tests/mathml_nemeth/mover_05.test/Makefile
	tests/mathml_nemeth/mover_06.test/Makefile
	tests/mathml_nemeth/mover_07.test/Makefile
	tests/mathml_nemeth/mover_08.test/Makefile
	tests/mathml_nemeth/mover_09.test/Makefile
	tests/mathml_nemeth/mover_10.test/Makefile
	tests/mathml_nemeth/mover_11.test/Makefile
	tests/mathml_nemeth/mover_12.test/Makefile
	tests/mathml_nemeth/mover_13.test/Makefile
	tests/mathml_nemeth/mover_14.test/Makefile
	tests/mathml_nemeth/mover_15.test/Makefile
	tests/mathml_nemeth/mover_16.test/Makefile
	tests/mathml_nemeth/mroot_01.test/Makefile
	tests/mathml_nemeth/msqrt_01.test/Makefile
	tests/mathml_nemeth/msqrt_02.test/Makefile
	tests/mathml_nemeth/msqrt_03.test/Makefile
	tests/mathml_nemeth/msub_01.test/Makefile
	tests/mathml_nemeth/msub_02.test/Makefile
	tests/mathml_nemeth/msub_03.test/Makefile
	tests/mathml_nemeth/msub_04.test/Makefile
	tests/mathml_nemeth/msub_05.test/Makefile
	tests/mathml_nemeth/msub_06.test/Makefile
	tests/mathml_nemeth/msub_07.test/Makefile
	tests/mathml_nemeth/msub_08.test/Makefile
	tests/mathml_nemeth/msub_09.test/Makefile
	tests/mathml_nemeth/msub_10.test/Makefile
	tests/mathml_nemeth/msubsup_01.test/Makefile
	tests/mathml_nemeth/msubsup_02.test/Makefile
	tests/mathml_nemeth/msubsup_03.test/Makefile
	tests/mathml_nemeth/msubsup_04.test/Makefile
	tests/mathml_nemeth/msubsup_05.test/Makefile
	tests/mathml_nemeth/msubsup_06.test/Makefile
	tests/mathml_nemeth/msubsup_07.test/Makefile
	tests/mathml_nemeth/msup_01.test/Makefile
	tests/mathml_nemeth/msup_02.test/Makefile
	tests/mathml_nemeth/msup_03.test/Makefile
	tests/mathml_nemeth/msup_04.test/Makefile
	tests/mathml_nemeth/msup_05.test/Makefile
	tests/mathml_nemeth/msup_06.test/Makefile
	tests/mathml_nemeth/msup_07.test/Makefile
	tests/mathml_nemeth/msup_08.test/Makefile
	tests/mathml_nemeth/msup_09.test/Makefile
	tests/mathml_nemeth/msup_10.test/Makefile
	tests/mathml_nemeth/msup_11.test/Makefile
	tests/mathml_nemeth/munder_01.test/Makefile
	tests/mathml_nemeth/munder_02.test/Makefile
	tests/mathml_nemeth/munder_03.test/Makefile
	tests/mathml_nemeth/munder_04.test/Makefile
	tests/mathml_nemeth/munder_05.test/Makefile
	tests/mathml_nemeth/munder_06.test/Makefile
	tests/mathml_nemeth/munder_07.test/Makefile
	tests/mathml_nemeth/munderover_01.test/Makefile
	tests/mathml_nemeth/munderover_02.test/Makefile
	tests/mathml_nemeth/munderover_03.test/Makefile
	tests/nemeth_MathCAT/Makefile
	tests/nemeth_MathCAT/above_and_below_88_2.test/Makefile
	tests/nemeth_MathCAT/arrow_96_1.test/Makefile
	tests/nemeth_MathCAT/arrow_96_4.test/Makefile
	tests/nemeth_MathCAT/as_multiscript_nested_sub_sup_74_c_5.test/Makefile
	tests/nemeth_MathCAT/bar_97_b_1.test/Makefile
	tests/nemeth_MathCAT/bar_above_and_below_88_1.test/Makefile
	tests/nemeth_MathCAT/baseline_80_a_1.test/Makefile
	tests/nemeth_MathCAT/beveled_frac_62_b_1.test/Makefile
	tests/nemeth_MathCAT/binomial_90_1.test/Makefile
	tests/nemeth_MathCAT/boldface_32_b_3.test/Makefile
	tests/nemeth_MathCAT/boldface_32_b_6.test/Makefile
	tests/nemeth_MathCAT/cap_22_a_1.test/Makefile
	tests/nemeth_MathCAT/cap_roman_numeral_18_a_3.test/Makefile
	tests/nemeth_MathCAT/carrot_98_1.test/Makefile
	tests/nemeth_MathCAT/comma_78_2.test/Makefile
	tests/nemeth_MathCAT/comma_78_2_invisible.test/Makefile
	tests/nemeth_MathCAT/comma_78_3.test/Makefile
	tests/nemeth_MathCAT/comma_78_6.test/Makefile
	tests/nemeth_MathCAT/comma_ellipsis_in_sub_79_b_5.test/Makefile
	tests/nemeth_MathCAT/comma_in_number_in_sup_79_b_3.test/Makefile
	tests/nemeth_MathCAT/comma_in_sup_79_b_4.test/Makefile
	tests/nemeth_MathCAT/comma_number_77_4_20.test/Makefile
	tests/nemeth_MathCAT/comma_space_78_1.test/Makefile
	tests/nemeth_MathCAT/comparison_79_g_2.test/Makefile
	tests/nemeth_MathCAT/complex_frac_66_1.test/Makefile
	tests/nemeth_MathCAT/dash_42_4.test/Makefile
	tests/nemeth_MathCAT/dash_42_6.test/Makefile
	tests/nemeth_MathCAT/dots_99_a_1.test/Makefile
	tests/nemeth_MathCAT/dots_99_a_2.test/Makefile
	tests/nemeth_MathCAT/dots_99_a_3.test/Makefile
	tests/nemeth_MathCAT/ellipsis_43_b_3.test/Makefile
	tests/nemeth_MathCAT/ellipsis_level_79_f_1.test/Makefile
	tests/nemeth_MathCAT/extension_field_not_ratio_151_11.test/Makefile
	tests/nemeth_MathCAT/full_binomial.test/Makefile
	tests/nemeth_MathCAT/function_space_119_c_3.test/Makefile
	tests/nemeth_MathCAT/german_base_77_4_3.test/Makefile
	tests/nemeth_MathCAT/hyper_complex_frac_68_a_1.test/Makefile
	tests/nemeth_MathCAT/identity_matrix_126.test/Makefile
	tests/nemeth_MathCAT/integral_77_4_26.test/Makefile
	tests/nemeth_MathCAT/left_sup_75_1.test/Makefile
	tests/nemeth_MathCAT/left_sup_75_12.test/Makefile
	tests/nemeth_MathCAT/left_sup_75_4.test/Makefile
	tests/nemeth_MathCAT/left_sup_75_7.test/Makefile
	tests/nemeth_MathCAT/left_sup_75_8.test/Makefile
	tests/nemeth_MathCAT/lesson_11_24_1.test/Makefile
	tests/nemeth_MathCAT/letter_26_b_19.test/Makefile
	tests/nemeth_MathCAT/lim_86_a_3.test/Makefile
	tests/nemeth_MathCAT/list_num_ind_11_a_1.test/Makefile
	tests/nemeth_MathCAT/list_num_ind_11_a_2.test/Makefile
	tests/nemeth_MathCAT/list_num_ind_11_a_7.test/Makefile
	tests/nemeth_MathCAT/list_num_ind_11_c_1.test/Makefile
	tests/nemeth_MathCAT/list_num_ind_11_d_1.test/Makefile
	tests/nemeth_MathCAT/list_num_ind__11_a_3.test/Makefile
	tests/nemeth_MathCAT/log_77_4_8.test/Makefile
	tests/nemeth_MathCAT/lower_roman_numeral_18_b_4.test/Makefile
	tests/nemeth_MathCAT/mathml_spec_example_86_a.test/Makefile
	tests/nemeth_MathCAT/mathml_spec_example_alt_char_86_a.test/Makefile
	tests/nemeth_MathCAT/menclose_111_a_4.test/Makefile
	tests/nemeth_MathCAT/menclose_115_1.test/Makefile
	tests/nemeth_MathCAT/menclose_86_a_1.test/Makefile
	tests/nemeth_MathCAT/menclose_86_a_4.test/Makefile
	tests/nemeth_MathCAT/menclose_86_b_1.test/Makefile
	tests/nemeth_MathCAT/menclose_86_b_11.test/Makefile
	tests/nemeth_MathCAT/menclose_bar_97_b_1.test/Makefile
	tests/nemeth_MathCAT/menclose_bar_97_b_3.test/Makefile
	tests/nemeth_MathCAT/menclose_lesson_12_5_5_5.test/Makefile
	tests/nemeth_MathCAT/menclose_primed_86_b_6.test/Makefile
	tests/nemeth_MathCAT/menclose_top_bottom_88_1.test/Makefile
	tests/nemeth_MathCAT/mixed_frac_63_a_1.test/Makefile
	tests/nemeth_MathCAT/mixed_frac_64_2.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_77_4_10.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_77_4_18.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_a_1.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_a_2.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_a_3.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_1.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_2.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_3.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_4.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_5.test/Makefile
	tests/nemeth_MathCAT/mmultiscripts_82_b_6.test/Makefile
	tests/nemeth_MathCAT/modifier_in_script_91_1.test/Makefile
	tests/nemeth_MathCAT/ms.test/Makefile
	tests/nemeth_MathCAT/ms_38_4_8.test/Makefile
	tests/nemeth_MathCAT/msubsup_82_a_1.test/Makefile
	tests/nemeth_MathCAT/msubsup_82_a_3.test/Makefile
	tests/nemeth_MathCAT/multipurpose_177_2_1.test/Makefile
	tests/nemeth_MathCAT/multipurpose_177_2_2.test/Makefile
	tests/nemeth_MathCAT/multipurpose_177_3_1.test/Makefile
	tests/nemeth_MathCAT/multipurpose_177_5_1.test/Makefile
	tests/nemeth_MathCAT/multipurpose_177_7_1.test/Makefile
	tests/nemeth_MathCAT/munder_lesson_12_5_5_5.test/Makefile
	tests/nemeth_MathCAT/nested_root_105_2.test/Makefile
	tests/nemeth_MathCAT/nested_sqrt_105_1.test/Makefile
	tests/nemeth_MathCAT/nested_sqrt_105_3.test/Makefile
	tests/nemeth_MathCAT/nested_sqrt_105_4.test/Makefile
	tests/nemeth_MathCAT/nested_sub_sup_74_c_5.test/Makefile
	tests/nemeth_MathCAT/nested_sup_74_b_1.test/Makefile
	tests/nemeth_MathCAT/nested_sup_74_b_4.test/Makefile
	tests/nemeth_MathCAT/nested_sup_mmultiscripts_74_b_1.test/Makefile
	tests/nemeth_MathCAT/nested_sup_sup_space_79_d_9.test/Makefile
	tests/nemeth_MathCAT/nested_super_79_a_2.test/Makefile
	tests/nemeth_MathCAT/nested_super_79_a_3.test/Makefile
	tests/nemeth_MathCAT/nested_super_space_79_d_3.test/Makefile
	tests/nemeth_MathCAT/nested_super_space_79_d_7.test/Makefile
	tests/nemeth_MathCAT/non_hyper_complex_frac_67_1.test/Makefile
	tests/nemeth_MathCAT/non_list_10_4.test/Makefile
	tests/nemeth_MathCAT/number_space_after.test/Makefile
	tests/nemeth_MathCAT/number_space_before.test/Makefile
	tests/nemeth_MathCAT/number_space_before_and_after.test/Makefile
	tests/nemeth_MathCAT/numeric_sub_81_a_1.test/Makefile
	tests/nemeth_MathCAT/order2_overbar_87_a_1.test/Makefile
	tests/nemeth_MathCAT/overbar_86_a_4.test/Makefile
	tests/nemeth_MathCAT/overbar_86_a_5.test/Makefile
	tests/nemeth_MathCAT/overbar_86_b_1.test/Makefile
	tests/nemeth_MathCAT/overbar_86_b_10.test/Makefile
	tests/nemeth_MathCAT/overbar_86_b_11.test/Makefile
	tests/nemeth_MathCAT/prescript_77_4_18.test/Makefile
	tests/nemeth_MathCAT/prescript_77_4_6.test/Makefile
	tests/nemeth_MathCAT/prescript_77_4_7.test/Makefile
	tests/nemeth_MathCAT/prime_172_5.test/Makefile
	tests/nemeth_MathCAT/prime_172_6.test/Makefile
	tests/nemeth_MathCAT/prime_172_8.test/Makefile
	tests/nemeth_MathCAT/prime_172_9.test/Makefile
	tests/nemeth_MathCAT/prime_77_4_4.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_1.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_3.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_4.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_5.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_6.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_7.test/Makefile
	tests/nemeth_MathCAT/prime_83_b_8.test/Makefile
	tests/nemeth_MathCAT/prime_mathjax_83_b_2.test/Makefile
	tests/nemeth_MathCAT/prime_mmultiscripts_83_b_2.test/Makefile
	tests/nemeth_MathCAT/prime_mmultiscripts_83_b_4.test/Makefile
	tests/nemeth_MathCAT/prime_wiris_83_b_2.test/Makefile
	tests/nemeth_MathCAT/primed_86_b_6.test/Makefile
	tests/nemeth_MathCAT/product_77_4_24.test/Makefile
	tests/nemeth_MathCAT/punct_37_1_2.test/Makefile
	tests/nemeth_MathCAT/punct_37_2_2.test/Makefile
	tests/nemeth_MathCAT/punct_37_7_1.test/Makefile
	tests/nemeth_MathCAT/punctuation_after_sup_79_b_2.test/Makefile
	tests/nemeth_MathCAT/ratio_151_11.test/Makefile
	tests/nemeth_MathCAT/root_104_iii_1.test/Makefile
	tests/nemeth_MathCAT/root_104_iii_4.test/Makefile
	tests/nemeth_MathCAT/set_vertical_bar_145_1.test/Makefile
	tests/nemeth_MathCAT/shape_110_1.test/Makefile
	tests/nemeth_MathCAT/simple_frac_62_a_3.test/Makefile
	tests/nemeth_MathCAT/sqrt_103_a_2.test/Makefile
	tests/nemeth_MathCAT/sqrt_103_a_4.test/Makefile
	tests/nemeth_MathCAT/sqrt_103_b_2.test/Makefile
	tests/nemeth_MathCAT/sub_ind_79_g_4.test/Makefile
	tests/nemeth_MathCAT/sub_ind_80_b_3.test/Makefile
	tests/nemeth_MathCAT/sub_ind_80_b_4.test/Makefile
	tests/nemeth_MathCAT/sub_ind_mmultiscripts_80_b_3.test/Makefile
	tests/nemeth_MathCAT/sub_sup_82_b_1.test/Makefile
	tests/nemeth_MathCAT/sub_sup_82_b_2.test/Makefile
	tests/nemeth_MathCAT/sum_77_4_23.test/Makefile
	tests/nemeth_MathCAT/superscript_80_a_2.test/Makefile
	tests/nemeth_MathCAT/table_entry_after_sup_79_c_4.test/Makefile
	tests/nemeth_MathCAT/tensor_from_mathml_spec.test/Makefile
	tests/nemeth_MathCAT/test_9_d_2.test/Makefile
	tests/nemeth_MathCAT/test_9_d_3.test/Makefile
	tests/nemeth_MathCAT/text_after_sup_79_c_3.test/Makefile
	tests/nemeth_MathCAT/trilinear_not_ratio_151_11.test/Makefile
	tests/nemeth_MathCAT/underbar_86_a_1.test/Makefile
	tests/nemeth_MathCAT/vertical_bar_145_4.test/Makefile
	tests/nemeth_MathCAT/whitespace_in_sup_79_e_1.test/Makefile
	tests/nemeth_MathCAT/word_77_4_12.test/Makefile	
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

