/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   JJB Software, Inc. www.jjb-software.com
   All rights reserved

   This file is free software; you can redistribute it and/or modify it
   under the terms of the Lesser or Library GNU General Public License 
   as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   Library GNU General Public License for more details.

   You should have received a copy of the Library GNU General Public 
   License along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Maintained by John J. Boyer john.boyer@jjb-software.com
   */

#ifndef liblouisutdml_h
#define liblouisutdml_h
#include <libxml/parser.h>
#include <louis.h>
#include "liblouisutdml.h"
#include "sem_enum.h"

typedef enum
{
  firstCall = 0,
  otherCall = 1,
  skipChoicesBefore = 2,
  skipChoicesAfter = 3
} NodeAction;

typedef enum
{
  leftJustified = 0,
  rightJustified = 1,
  centered = 2,
  alignColumnsLeft = 3,
  alignColumnsRight = 4,
  listColumns = 5,
  listLines = 6,
  computerCoded = 7,
  contents = 8
} StyleFormat;

typedef enum
{
  normal = 0,
  blank = 1,
  p = 2,
  roman = 3
} BrlPageNumFormat;

typedef struct
{				/*Paragraph formatting instructions */
  sem_act action;
  int lines_before;
  int lines_after;
  int left_margin;
  int first_line_indent;	/* At true margin if negative */
  sem_act translate;
  int skip_number_lines;	/*Don't write on lines with page numbers */
  StyleFormat format;
  BrlPageNumFormat brlNumFormat;
  int newpage_before;
  int newpage_after;
  int righthand_page;
} StyleType;

#ifdef _WIN32
#define strcasecmp _strnicmp
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#ifndef CHARSIZE
#define CHARSIZE sizeof (widechar)
#endif

#define CELLWIDTH 5
#define NORMALLINE 8
#define WIDELINE 10
#define BUFSIZE 8192
#define MAX_LENGTH BUFSIZE - 4
#define MAX_TRANS_LENGTH 2 * BUFSIZE - 4
#define MAXNAMELEN 256
#define MAXNUMLEN 32
#define STACKSIZE 100

typedef enum
{
  utf8 = 0,
  utf16,
  utf32,
  ascii8
} Encoding;

typedef enum
{
  plain = 0,
  html
} TextFormat;

typedef enum
{
  textDevice = 0,
  browser,
  utd
} FormatFor;

typedef enum
{
  error,
  beforeBody,
  startBody,
  resumeBody,
  bodyInterrupted,
  afterBody
} StyleStatus;

typedef struct
{
  StyleType *style;
  xmlNode *node;
  StyleStatus status;
  BrlPageNumFormat curBrlNumFormat;
} StyleRecord;

typedef struct
{				/*user data */
  FILE *inFile;
  FILE *outFile;
  xmlDoc *doc;
  xmlNode *head_node;
  int text_length;
  int old_text_length;
  int translated_length;
  int string_buf_len;
  int has_math;
  int has_comp_code;
  int has_chem;
  int has_graphics;
  int has_music;
  int has_cdata;
  Encoding input_encoding;
  Encoding output_encoding;
  Encoding input_text_encoding;
  TextFormat back_text;
  int back_line_length;
  FormatFor format_for;
  int contents;
  int has_contentsheader;
  unsigned int mode;
  translationModes louis_mode;
  int debug;
  int top_margin;
  int left_margin;
  int contains_utd;
  int cells_per_line;
  int lines_per_page;
  int beginning_braille_page_number;
  int interpoint;
  int print_page_number_at;
  int braille_page_number_at;
  int hyphenate;
  int internet_access;
  int new_entries;
  int doc_new_entries;
  const char *main_braille_table;
  const char *inbuf;
  int inlen;
  widechar *outbuf;
  int outlen;
  int outlen_so_far;
  int lines_on_page;
  int braille_page_number;
  int prelim_pages;
  int paragraphs;
  int braille_pages;
  int print_pages;
  char path_list[4 * MAXNAMELEN];
  const char *writeable_path;
  char string_escape;
  char file_separator;
  char line_fill;
  char lit_hyphen[5];
  char comp_hyphen[5];
  widechar running_head[MAXNAMELEN / 2];
  widechar footer[MAXNAMELEN / 2];
  int running_head_length;
  int footer_length;
  const char *contracted_table_name;
  const char *uncontracted_table_name;
  const char *compbrl_table_name;
  const char *mathtext_table_name;
  const char *mathexpr_table_name;
  const char *edit_table_name;
  const char *semantic_files;
  const char *volume_sem;
  widechar print_page_number[MAXNUMLEN];
   widechar page_separator_number_first[MAXNUMLEN];
   widechar page_separator_number_last[MAXNUMLEN];
   widechar print_page_number_first[MAXNUMLEN];
   widechar print_page_number_last[MAXNUMLEN];
    int page_separator;
   int page_separator_number;
   int ignore_empty_pages;
   int continue_pages;
   int merge_unnumbered_pages;
   int print_page_number_range;
   int page_number_top_separate_line;
   int page_number_bottom_separate_line;
   int fill_pages;
   int after_contents;
   widechar *pagebuf;
   int pagelen;
   int pagelen_so_far;
  widechar braille_page_string[MAXNUMLEN];
  char lineEnd[8];
  char pageEnd[8];
  char fileEnd[8];
  int line_spacing;
  int top;
  sem_act stack[STACKSIZE];
  StyleRecord style_stack[STACKSIZE];
  int style_top;
  BrlPageNumFormat brl_page_num_format;
  char xml_header[BUFSIZE];
  widechar text_buffer[2 * BUFSIZE];
  widechar translated_buffer[2 * BUFSIZE];
  unsigned char typeform[2 * BUFSIZE];
  char string_buffer[2 * BUFSIZE];
} UserData;
extern UserData *ud;

/* Function definitions */
sem_act find_semantic_number (const char *semName);
int file_exists (const char *completePath);
int find_file (const char *fileName, char *filePath);
int set_paths (const char *configPath);
int read_configuration_file (const char *configFileName,
			     const char *logFileName, const char
			     *settingsString, unsigned int mode);
int config_compileSettings (const char *fileName);
int do_xpath_expr ();
int examine_document (xmlNode * node);
int transcribe_document (xmlNode * node);
int transcribe_math (xmlNode * node, int action);
int transcribe_computerCode (xmlNode * node, int action);
int transcribe_cdataSection (xmlNode * node);
int transcribe_paragraph (xmlNode * node, int action);
int transcribe_chemistry (xmlNode * node, int action);
int transcribe_graphic (xmlNode * node, int action);
int transcribe_music (xmlNode * node, int action);
int interpret_utd (xmlNode *node, NodeAction action);
int compile_semantic_table (xmlNode * rootElement);
sem_act set_sem_attr (xmlNode * node);
sem_act get_sem_attr (xmlNode * node);
sem_act push_sem_stack (xmlNode * node);
sem_act pop_sem_stack ();
void destroy_semantic_table (void);
void append_new_entries (void);
int insert_code (xmlNode * node, int which);
xmlChar *get_attr_value (xmlNode * node);
int change_table (xmlNode * node);
int initialize_contents (void);
int start_heading (sem_act action, widechar * translatedBuffer, int
		   translatedLength);
int finish_heading (sem_act action);
int make_contents (void);
void do_reverse (xmlNode * node);
int do_boxline (xmlNode * node);
void do_linespacing (xmlNode *node);
int do_newpage (void);
int do_blankline (void);
int do_softreturn (void);
int do_righthandpage (void);
void libxml_errors (void *ctx ATTRIBUTE_UNUSED, const char *msg, ...);
int do_configstring (xmlNode * node);
StyleType *new_style (xmlChar * name);
StyleType *lookup_style (xmlChar * name);
StyleType *is_style (xmlNode * node);
StyleType *action_to_style (sem_act action);
int insert_utf8 (const unsigned char *intext);
int insert_utf16 (widechar * intext, int length);
int insert_translation (const char *table);
int write_paragraph (sem_act action, xmlNode *node);
int start_document (void);
int end_document (void);
int transcribe_text_string (void);
int transcribe_text_file (void);
int back_translate_file (void);
StyleType *find_current_style (void);
void insert_text (xmlNode * node);
int insert_linkOrTarget (xmlNode * node, int which);
int start_style (StyleType * curStyle, xmlNode *node);
int end_style ();
int find_action (const char **actions, const char *action);
int find_group_length (const char groupSym[2], const char *groupStart);
char *alloc_string (const char *inString);
int write_outbuf (void);
int link_brl_node (xmlNode *node);
void clean_semantic_table (void);
int back_translate_braille_string (void);
#endif /*louisutdml_h */
