SUBDIRS =					\
	boxline.test				\
	endNotes.test				\
	firstLineIndent.test			\
	footer_01.test				\
	footer_02.test				\
	hyphenation.test			\
	mathml_nemeth				\
	minSyllableWidth.test			\
	mixed_content.test			\
	orphanControl_01.test			\
	orphanControl_02.test			\
	page_numbering.test			\
	pagenum.test				\
	prehyphenation.test			\
	printPageNumberRange_01.test		\
	printPageNumberRange_02.test		\
	printPageNumberRange_03.test		\
	test_mathml_woluwe

AM_CPPFLAGS =					\
	-I$(top_srcdir)/liblouisutdml		\
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

check_PROGRAMS = \
	testTranslateString\
	tripleTrans

testTranslateString_SOURCES = testTranslateString.c
tripleTrans_SOURCES = tripleTrans.c

XFAIL_TESTS =					\
	testTranslateString

dist_suite_TESTS =				\
	boxline.test				\
	endNotes.test				\
	firstLineIndent.test			\
	footer_01.test				\
	footer_02.test				\
	hyphenation.test			\
	mathml_nemeth/general_001.test		\
	mathml_nemeth/general_002.test		\
	mathml_nemeth/general_003.test		\
	mathml_nemeth/mfrac_01.test		\
	mathml_nemeth/mfrac_02.test		\
	mathml_nemeth/mfrac_03.test		\
	mathml_nemeth/mfrac_04.test		\
	mathml_nemeth/mfrac_05.test		\
	mathml_nemeth/mfrac_06.test		\
	mathml_nemeth/mfrac_07.test		\
	mathml_nemeth/mixed_scripts_01.test	\
	mathml_nemeth/mixed_scripts_02.test	\
	mathml_nemeth/mixed_scripts_03.test	\
	mathml_nemeth/mixed_scripts_04.test	\
	mathml_nemeth/mixed_scripts_05.test	\
	mathml_nemeth/mixed_scripts_06.test	\
	mathml_nemeth/mixed_scripts_07.test	\
	mathml_nemeth/mmultiscripts_01.test	\
	mathml_nemeth/mmultiscripts_02.test	\
	mathml_nemeth/mmultiscripts_03.test	\
	mathml_nemeth/mmultiscripts_04.test	\
	mathml_nemeth/mmultiscripts_05.test	\
	mathml_nemeth/mmultiscripts_06.test	\
	mathml_nemeth/mmultiscripts_07.test	\
	mathml_nemeth/mover_01.test		\
	mathml_nemeth/mover_02.test		\
	mathml_nemeth/mover_03.test		\
	mathml_nemeth/mover_04.test		\
	mathml_nemeth/mover_05.test		\
	mathml_nemeth/mover_06.test		\
	mathml_nemeth/mover_07.test		\
	mathml_nemeth/mover_08.test		\
	mathml_nemeth/mover_09.test		\
	mathml_nemeth/mover_10.test		\
	mathml_nemeth/mover_11.test		\
	mathml_nemeth/mover_12.test		\
	mathml_nemeth/mover_13.test		\
	mathml_nemeth/mover_14.test		\
	mathml_nemeth/mover_15.test		\
	mathml_nemeth/mover_16.test		\
	mathml_nemeth/mroot_01.test		\
	mathml_nemeth/msqrt_01.test		\
	mathml_nemeth/msqrt_02.test		\
	mathml_nemeth/msqrt_03.test		\
	mathml_nemeth/msub_01.test		\
	mathml_nemeth/msub_02.test		\
	mathml_nemeth/msub_03.test		\
	mathml_nemeth/msub_04.test		\
	mathml_nemeth/msub_05.test		\
	mathml_nemeth/msub_06.test		\
	mathml_nemeth/msub_07.test		\
	mathml_nemeth/msub_08.test		\
	mathml_nemeth/msub_09.test		\
	mathml_nemeth/msub_10.test		\
	mathml_nemeth/msubsup_01.test		\
	mathml_nemeth/msubsup_02.test		\
	mathml_nemeth/msubsup_03.test		\
	mathml_nemeth/msubsup_04.test		\
	mathml_nemeth/msubsup_05.test		\
	mathml_nemeth/msubsup_06.test		\
	mathml_nemeth/msubsup_07.test		\
	mathml_nemeth/msup_01.test		\
	mathml_nemeth/msup_02.test		\
	mathml_nemeth/msup_03.test		\
	mathml_nemeth/msup_04.test		\
	mathml_nemeth/msup_05.test		\
	mathml_nemeth/msup_06.test		\
	mathml_nemeth/msup_07.test		\
	mathml_nemeth/msup_08.test		\
	mathml_nemeth/msup_09.test		\
	mathml_nemeth/msup_10.test		\
	mathml_nemeth/msup_11.test		\
	mathml_nemeth/munder_01.test		\
	mathml_nemeth/munder_02.test		\
	mathml_nemeth/munder_03.test		\
	mathml_nemeth/munder_04.test		\
	mathml_nemeth/munder_05.test		\
	mathml_nemeth/munder_06.test		\
	mathml_nemeth/munder_07.test		\
	mathml_nemeth/munderover_01.test	\
	mathml_nemeth/munderover_02.test	\
	mathml_nemeth/munderover_03.test	\
	minSyllableWidth.test			\
	mixed_content.test			\
	orphanControl_01.test			\
	orphanControl_02.test			\
	page_numbering.test			\
	pagenum.test				\
	prehyphenation.test			\
	printPageNumberRange_01.test		\
	printPageNumberRange_02.test		\
	printPageNumberRange_03.test		\
	test_mathml_woluwe/test_001.test	\
	test_mathml_woluwe/test_002.test	\
	test_mathml_woluwe/test_003.test	\
	test_mathml_woluwe/test_004.test	\
	test_mathml_woluwe/test_005.test	\
	test_mathml_woluwe/test_006.test	\
	test_mathml_woluwe/test_007.test	\
	test_mathml_woluwe/test_008.test	\
	test_mathml_woluwe/test_009.test	\
	test_mathml_woluwe/test_010.test	\
	test_mathml_woluwe/test_011.test	\
	test_mathml_woluwe/test_012.test	\
	test_mathml_woluwe/test_013.test	\
	test_mathml_woluwe/test_014.test	\
	test_mathml_woluwe/test_015.test	\
	test_mathml_woluwe/test_016.test	\
	test_mathml_woluwe/test_017.test	\
	test_mathml_woluwe/test_018.test	\
	test_mathml_woluwe/test_019.test	\
	test_mathml_woluwe/test_020.test	\
	test_mathml_woluwe/test_021.test	\
	test_mathml_woluwe/test_022.test	\
	test_mathml_woluwe/test_023.test	\
	test_mathml_woluwe/test_024.test	\
	test_mathml_woluwe/test_025.test	\
	test_mathml_woluwe/test_026.test	\
	test_mathml_woluwe/test_027.test	\
	test_mathml_woluwe/test_028.test	\
	test_mathml_woluwe/test_029.test	\
	test_mathml_woluwe/test_030.test	\
	test_mathml_woluwe/test_031.test	\
	test_mathml_woluwe/test_032.test	\
	test_mathml_woluwe/test_033.test	\
	test_mathml_woluwe/test_034.test	\
	test_mathml_woluwe/test_035.test	\
	test_mathml_woluwe/test_036.test	\
	test_mathml_woluwe/test_037.test	\
	test_mathml_woluwe/test_038.test	\
	test_mathml_woluwe/test_039.test	\
	test_mathml_woluwe/test_040.test	\
	test_mathml_woluwe/test_041.test	\
	test_mathml_woluwe/test_042.test	\
	test_mathml_woluwe/test_043.test	\
	test_mathml_woluwe/test_044.test	\
	test_mathml_woluwe/test_045.test	\
	test_mathml_woluwe/test_046.test	\
	test_mathml_woluwe/test_047.test	\
	test_mathml_woluwe/test_048.test	\
	test_mathml_woluwe/test_049.test	\
	test_mathml_woluwe/test_050.test	\
	test_mathml_woluwe/test_051.test	\
	test_mathml_woluwe/test_052.test	\
	test_mathml_woluwe/test_053.test	\
	test_mathml_woluwe/test_054.test	\
	test_mathml_woluwe/test_055.test	\
	test_mathml_woluwe/test_056.test	\
	test_mathml_woluwe/test_057.test	\
	test_mathml_woluwe/test_058.test	\
	test_mathml_woluwe/test_059.test	\
	test_mathml_woluwe/test_060.test	\
	test_mathml_woluwe/test_061.test	\
	test_mathml_woluwe/test_062.test	\
	test_mathml_woluwe/test_063.test	\
	test_mathml_woluwe/test_064.test	\
	test_mathml_woluwe/test_065.test	\
	test_mathml_woluwe/test_066.test	\
	test_mathml_woluwe/test_067.test	\
	test_mathml_woluwe/test_068.test	\
	test_mathml_woluwe/test_069.test	\
	test_mathml_woluwe/test_070.test	\
	test_mathml_woluwe/test_071.test	\
	test_mathml_woluwe/test_072.test	\
	test_mathml_woluwe/test_073.test	\
	test_mathml_woluwe/test_074.test	\
	test_mathml_woluwe/test_075.test	\
	test_mathml_woluwe/test_076.test	\
	test_mathml_woluwe/test_077.test	\
	test_mathml_woluwe/test_078.test	\
	test_mathml_woluwe/test_079.test	\
	test_mathml_woluwe/test_080.test	\
	test_mathml_woluwe/test_081.test	\
	test_mathml_woluwe/test_082.test	\
	test_mathml_woluwe/test_083.test	\
	test_mathml_woluwe/test_084.test	\
	test_mathml_woluwe/test_085.test	\
	test_mathml_woluwe/test_086.test	\
	test_mathml_woluwe/test_087.test	\
	test_mathml_woluwe/test_088.test	\
	test_mathml_woluwe/test_089.test	\
	test_mathml_woluwe/test_090.test	\
	test_mathml_woluwe/test_091.test	\
	test_mathml_woluwe/test_092.test	\
	test_mathml_woluwe/test_093.test	\
	test_mathml_woluwe/test_094.test	\
	test_mathml_woluwe/test_095.test	\
	test_mathml_woluwe/test_096.test	\
	test_mathml_woluwe/test_097.test	\
	test_mathml_woluwe/test_098.test	\
	test_mathml_woluwe/test_099.test	\
	test_mathml_woluwe/test_100.test	\
	test_mathml_woluwe/test_101.test	\
	test_mathml_woluwe/test_102.test	\
	test_mathml_woluwe/test_103.test	\
	test_mathml_woluwe/test_104.test	\
	test_mathml_woluwe/test_105.test	\
	test_mathml_woluwe/test_106.test	\
	test_mathml_woluwe/test_107.test	\
	test_mathml_woluwe/test_108.test	\
	test_mathml_woluwe/test_109.test	\
	test_mathml_woluwe/test_110.test	\
	test_mathml_woluwe/test_111.test	\
	test_mathml_woluwe/test_112.test	\
	test_mathml_woluwe/test_113.test	\
	test_mathml_woluwe/test_114.test	\
	test_mathml_woluwe/test_115.test	\
	test_mathml_woluwe/test_116.test	\
	test_mathml_woluwe/test_117.test	\
	test_mathml_woluwe/test_118.test	\
	test_mathml_woluwe/test_119.test	

XFAIL_TESTS +=					\
	pagenum.test				\
	footer_01.test				\
	footer_02.test				\
	orphanControl_02.test			\
	firstLineIndent.test			\
	mixed_content.test			\
	printPageNumberRange_03.test		\
	mathml_nemeth/general_001.test		\
	mathml_nemeth/general_002.test		\
	mathml_nemeth/general_003.test		\
	mathml_nemeth/mfrac_02.test		\
	mathml_nemeth/mfrac_03.test		\
	mathml_nemeth/mfrac_04.test		\
	mathml_nemeth/mfrac_06.test		\
	mathml_nemeth/mfrac_07.test		\
	mathml_nemeth/mixed_scripts_01.test	\
	mathml_nemeth/mixed_scripts_02.test	\
	mathml_nemeth/mixed_scripts_03.test	\
	mathml_nemeth/mixed_scripts_04.test	\
	mathml_nemeth/mixed_scripts_05.test	\
	mathml_nemeth/mixed_scripts_06.test	\
	mathml_nemeth/mixed_scripts_07.test	\
	mathml_nemeth/msub_02.test		\
	mathml_nemeth/msub_03.test		\
	mathml_nemeth/msub_04.test		\
	mathml_nemeth/msub_05.test		\
	mathml_nemeth/msub_06.test		\
	mathml_nemeth/msub_07.test		\
	mathml_nemeth/msub_08.test		\
	mathml_nemeth/msub_09.test		\
	mathml_nemeth/msub_10.test		\
	mathml_nemeth/msubsup_02.test		\
	mathml_nemeth/msubsup_03.test		\
	mathml_nemeth/msubsup_04.test		\
	mathml_nemeth/msubsup_05.test		\
	mathml_nemeth/msubsup_06.test		\
	mathml_nemeth/msubsup_07.test		\
	mathml_nemeth/msup_02.test		\
	mathml_nemeth/msup_06.test		\
	mathml_nemeth/msup_09.test		\
	mathml_nemeth/msup_10.test		\
	mathml_nemeth/msqrt_03.test		\
	mathml_nemeth/mmultiscripts_01.test	\
	mathml_nemeth/mmultiscripts_02.test	\
	mathml_nemeth/mmultiscripts_03.test	\
	mathml_nemeth/mmultiscripts_04.test	\
	mathml_nemeth/mmultiscripts_05.test	\
	mathml_nemeth/mmultiscripts_06.test	\
	mathml_nemeth/mmultiscripts_07.test	\
	mathml_nemeth/mover_01.test		\
	mathml_nemeth/mover_02.test		\
	mathml_nemeth/mover_03.test		\
	mathml_nemeth/mover_04.test		\
	mathml_nemeth/mover_05.test		\
	mathml_nemeth/mover_06.test		\
	mathml_nemeth/mover_07.test		\
	mathml_nemeth/mover_08.test		\
	mathml_nemeth/mover_13.test		\
	mathml_nemeth/mover_14.test		\
	mathml_nemeth/mover_15.test		\
	mathml_nemeth/mover_16.test		\
	mathml_nemeth/munder_01.test		\
	mathml_nemeth/munder_02.test		\
	mathml_nemeth/munder_03.test		\
	mathml_nemeth/munder_05.test		\
	mathml_nemeth/munder_06.test		\
	mathml_nemeth/munder_07.test		\
	mathml_nemeth/munderover_01.test	\
	mathml_nemeth/munderover_02.test	\
	mathml_nemeth/munderover_03.test	\
	test_mathml_woluwe/test_044.test	\
	test_mathml_woluwe/test_060.test	\
	test_mathml_woluwe/test_061.test	\
	test_mathml_woluwe/test_062.test	\
	test_mathml_woluwe/test_063.test	\
	test_mathml_woluwe/test_075.test	\
	test_mathml_woluwe/test_076.test	\
	test_mathml_woluwe/test_078.test	\
	test_mathml_woluwe/test_093.test	\
	test_mathml_woluwe/test_096.test	\
	test_mathml_woluwe/test_099.test

TESTS =						\
	$(check_PROGRAMS)			\
	$(dist_suite_TESTS)

TEST_LOG_COMPILER_SCRIPT = run_test.sh
TEST_EXTENSIONS = .test
TEST_LOG_COMPILER = $(top_srcdir)/tests/$(TEST_LOG_COMPILER_SCRIPT)

EXTRA_DIST = README $(TEST_LOG_COMPILER_SCRIPT)

TESTS_ENVIRONMENT =									\
	LD_LIBRARY_PATH=$(abs_top_builddir)/liblouisutdml/.libs:$$LD_LIBRARY_PATH	\
	PATH=$(abs_top_builddir)/tools:$$PATH
