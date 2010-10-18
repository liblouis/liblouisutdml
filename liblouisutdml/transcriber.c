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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"

#define LETSIGN "\\_"
static StyleRecord *styleSpec;
/* Note that the following is an actual data area, not a pointer*/
static StyleRecord prevStyleSpec;
static StyleType *style;
static StyleType *prevStyle;
static int styleBody (void);

static int
fineFormat (void)
{
  if (ud->text_length == 0 && ud->translated_length == 0)
    return 1;
  else
    {
      insert_translation (ud->main_braille_table);
      if (styleSpec == NULL)
	write_paragraph (para, NULL);
      else
	styleBody ();
      styleSpec->status = resumeBody;
    }
  return 1;
}

StyleType *
find_current_style (void)
{
  StyleRecord *sr = &ud->style_stack[ud->style_top];
  return sr->style;
}

static int doLeftJustify (void);
static widechar pageNumberString[MAXNUMLEN];
static int pageNumberLength;
static char *litHyphen = "-";
static char *compHyphen = "_&";
static char *blanks =
  "                                                                      ";
static int fillPage (void);
static int insertCharacters (char *chars, int length);
static BrlPageNumFormat currentBraillePageNumFormat;
static int writePagebuf (void);
static int writeOutbufDirect (void);
static char *makeRomanNumber (int n);
static int utd_start ();
static int utd_finish ();
static int utd_transcribe_text_string (void);
static int utd_transcribe_text_file (void);
static int utd_insert_translation (const char *table);
static void utd_insert_text (xmlNode * node, int length);
static int utd_makeBlankLines (int number, int beforeAfter);
static int utd_startStyle (void);
static int utd_styleBody (void);
static int utd_finishStyle (void);

int
start_document (void)
{
  ud->head_node = NULL;
  if (ud->has_math)
    ud->main_braille_table = ud->mathtext_table_name;
  else
    ud->main_braille_table = ud->contracted_table_name;
  if (!lou_getTable (ud->main_braille_table))
    return 0;
  if (ud->has_contentsheader)
    ud->braille_page_number = 1;
  else
    ud->braille_page_number = ud->beginning_braille_page_number;
  ud->louis_mode = 0;
  ud->outlen_so_far = 0;
  styleSpec = &prevStyleSpec;
  style = prevStyle = lookup_style ("document");
  prevStyleSpec.style = prevStyle;
  if (ud->format_for != utd)
    {
      if (ud->outFile && ud->output_encoding == utf16)
	{
	  /*Little Endian indicator */
	  fputc (0xff, ud->outFile);
	  fputc (0xfe, ud->outFile);
	}
      switch (ud->format_for)
	{
	default:
	case textDevice:
	  break;
	case browser:
	  if (!insertCharacters
	      ("<html><head><title>HTML Document</title></head><body><pre>",
	       58))
	    return 0;
	  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	    return 0;
	  write_outbuf ();
	  break;
	}
    }
  if (ud->contents && !ud->has_contentsheader)
    initialize_contents ();
  if (ud->format_for == utd)
    return (utd_start ());
  return 1;
}

int
end_document (void)
{
  if (ud->format_for == utd)
    return (utd_finish ());
  if (ud->style_top < 0)
    ud->style_top = 0;
  if (ud->contains_utd)
    return 1;
  if (ud->text_length != 0)
    insert_translation (ud->main_braille_table);
  if (ud->translated_length != 0)
    write_paragraph (para, NULL);
  if (ud->braille_pages)
    {
      fillPage ();
      write_outbuf ();
    }
  if (ud->contents)
    make_contents ();
  switch (ud->format_for)
    {
    default:
    case textDevice:
      break;
    case browser:
      if (!insertCharacters ("</pre></body></html>", 20))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      write_outbuf ();
      break;
    }
  return 1;
}

static int
isLineend (int *c)
{
  if (c[0] == 10 && c[1] == 13)
    return 2;
  else if (c[0] == 10 || c[0] == 13)
    return 1;
  else
    return 0;
}

int
transcribe_text_string (void)
{
  int charsProcessed = 0;
  int charsInParagraph = 0;
  int ch[2];
  int lineend[2];
  FormatFor oldFormat;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  StyleType *docStyle;
  StyleType *paraStyle;
  if (ud->format_for == utd)
    return utd_transcribe_text_string ();
  oldFormat = ud->format_for;
  ud->format_for = textDevice;
  docStyle = lookup_style ("document");
  paraStyle = lookup_style ("para");
  if (!start_document ())
    return 0;
  ud->input_encoding = ud->input_text_encoding;
  start_style (docStyle, NULL);
  while (1)
    {
      while (charsProcessed < ud->inlen)
	{
	  start_style (paraStyle, NULL);
	  ch[0] = ch[1];
	  ch[1] = ud->inbuf[charsProcessed++];
	  lineend[0] = lineend[1];
	  lineend[1] = isLineend (ch);
	  if (lineend[0] && lineend[1])
	    break;
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  paragraphBuffer[charsInParagraph++] = ch[1];
	}
      if (charsInParagraph == 0)
	break;
      ch[1] = ud->inbuf[charsProcessed++];
      paragraphBuffer[charsInParagraph] = 0;
      if (!insert_utf8 (paragraphBuffer))
	return 0;
      if (!insert_translation (ud->main_braille_table))
	return 0;
      if (ch[1] == 10)
	do_blankline ();
      end_style ();
      charsInParagraph = 0;
      paragraphBuffer[charsInParagraph++] = ch[1];
    }
  ud->input_encoding = utf8;
  end_style ();
  end_document ();
  ud->format_for = oldFormat;
  return 1;
}


int
transcribe_text_file (void)
{
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  FormatFor oldFormat;
  StyleType *docStyle;
  StyleType *paraStyle;
  if (ud->format_for == utd)
    return utd_transcribe_text_file ();
  oldFormat = ud->format_for;
  ud->format_for = textDevice;
  docStyle = lookup_style ("document");
  paraStyle = lookup_style ("para");
  if (!start_document ())
    return 0;
  start_style (docStyle, NULL);
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      start_style (paraStyle, NULL);
      while ((ch = fgetc (ud->inFile)) != EOF)
	{
	  if (ch == 0 || ch == 13)
	    continue;
	  if (pch == 10 && ch == 10)
	    break;
	  if (charsInParagraph == 0 && ch <= 32)
	    continue;
	  pch = ch;
	  if (ch < 32)
	    ch = ' ';
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  paragraphBuffer[charsInParagraph++] = ch;
	}
      if (charsInParagraph == 0)
	break;
      ch = fgetc (ud->inFile);
      paragraphBuffer[charsInParagraph] = 0;
      if (!insert_utf8 (paragraphBuffer))
	return 0;
      if (!insert_translation (ud->main_braille_table))
	return 0;
      if (ch == 10)
	do_blankline ();
      end_style ();
      charsInParagraph = 0;
      pch = 0;
      if (ch > 32)
	paragraphBuffer[charsInParagraph++] = ch;
    }
  ud->input_encoding = utf8;
  end_style ();
  end_document ();
  ud->format_for = oldFormat;
  return 1;
}

#define MAXBYTES 7
static int first0Bit[MAXBYTES] = { 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0XFE };

static int
utf8ToWc (const unsigned char *utf8str, int *inSize, widechar *
	  utfwcstr, int *outSize)
{
  int in = 0;
  int out = 0;
  int lastInSize = 0;
  int lastOutSize = 0;
  unsigned int ch = 0;
  int numBytes = 0;
  unsigned int utf32 = 0;
  int k;
  while (in < *inSize)
    {
      ch = utf8str[in++] & 0xff;
      if (ch < 128 || ud->input_encoding == ascii8)
	{
	  utfwcstr[out++] = (widechar) ch;
	  if (out >= *outSize)
	    {
	      *inSize = in;
	      *outSize = out;
	      return 1;
	    }
	  continue;
	}
      lastInSize = in;
      lastOutSize = out;
      for (numBytes = MAXBYTES - 1; numBytes >= 0; numBytes--)
	if (ch >= first0Bit[numBytes])
	  break;
      utf32 = ch & (0XFF - first0Bit[numBytes]);
      for (k = 0; k < numBytes; k++)
	{
	  if (in >= *inSize)
	    break;
	  utf32 = (utf32 << 6) + (utf8str[in++] & 0x3f);
	}
      if (CHARSIZE == 2 && utf32 > 0xffff)
	utf32 = 0xffff;
      utfwcstr[out++] = (widechar) utf32;
      if (out >= *outSize)
	{
	  *inSize = lastInSize;
	  *outSize = lastOutSize;
	  return 1;
	}
    }
  *inSize = in;
  *outSize = out;
  return 1;
}

static unsigned char *
utfwcto8 (widechar utfwcChar)
{
  static unsigned char utf8Str[10];
  unsigned int utf8Bytes[MAXBYTES] = { 0, 0, 0, 0, 0, 0, 0 };
  int numBytes;
  int k;
  unsigned int utf32;
  if (utfwcChar < 128)
    {
      utf8Str[0] = utfwcChar;
      utf8Str[1] = 0;
      return utf8Str;
    }
  utf32 = utfwcChar;
  for (numBytes = 0; numBytes < MAXBYTES - 1; numBytes++)
    {
      utf8Bytes[numBytes] = utf32 & 0x3f;
      utf32 >>= 6;
      if (utf32 == 0)
	break;
    }
  utf8Str[0] = first0Bit[numBytes] | utf8Bytes[numBytes];
  numBytes--;
  k = 1;
  while (numBytes >= 0)
    utf8Str[k++] = utf8Bytes[numBytes--] | 0x80;
  utf8Str[k] = 0;
  return utf8Str;
}

static int
minimum (int x, int y)
{
  if (x <= y)
    return x;
  return y;
}

int
insert_utf8 (const unsigned char *text)
{
  int length = strlen ((char *) text);
  int charsToDo = 0;
  int maxSize = 0;
  int charsDone = length;
  int outSize = MAX_LENGTH - ud->text_length;
  ud->old_text_length = ud->text_length;
  utf8ToWc (text, &charsDone, &ud->text_buffer[ud->text_length], &outSize);
  ud->text_length += outSize;
  while (charsDone < length)
    {
      /*Handle buffer overflow */
      StyleType *style = find_current_style ();
      const char *table;
      if (style == NULL)
	style = lookup_style ("para");
      switch (style->action)
	{
	case code:
	  table = ud->compbrl_table_name;
	  memset (ud->typeform, computer_braille, ud->text_length);
	  break;
	default:
	  table = ud->main_braille_table;
	  break;
	}
      if (!insert_translation (table))
	return 0;
      if (!write_paragraph (style->action, NULL))
	return 0;
      charsToDo = minimum (MAX_LENGTH, length - charsDone);
      while (text[charsDone + charsToDo] > 32)
	charsToDo--;
      if (charsToDo <= 0)
	charsToDo = minimum (MAX_LENGTH, length - charsDone);
      maxSize = MAX_LENGTH;
      utf8ToWc (&text[charsDone], &charsToDo, &ud->text_buffer[0], &maxSize);
      charsDone += charsToDo;
    }
  return outSize;
}

int
insert_utfwc (widechar * text, int length)
{
  if (length < 0)
    return 0;
  if ((ud->text_length + length) > MAX_LENGTH)
    return 0;
  memcpy (&ud->text_buffer[ud->text_length], text, CHARSIZE * length);
  ud->text_length += length;
  return length;
}

int
insert_translation (const char *table)
{
  int translationLength;
  int translatedLength;
  int k;
  if (ud->contains_utd)
    return 1;
  if (table == NULL)
    {
      memset (ud->typeform, 0, sizeof (ud->typeform));
      ud->text_length = 0;
      return 0;
    }
  if (ud->text_length == 0)
    return 1;
  for (k = 0; k < ud->text_length && ud->text_buffer[k] <= 32; k++);
  if (k == ud->text_length)
    {
      ud->text_length = 0;
      return 1;
    }
  if (styleSpec != NULL && styleSpec->status == resumeBody)
    styleSpec->status = bodyInterrupted;
  if (ud->format_for == utd)
    return (utd_insert_translation (table));
  if (ud->translated_length > 0 && ud->translated_length <
      MAX_TRANS_LENGTH &&
      ud->translated_buffer[ud->translated_length - 1] > 32)
    ud->translated_buffer[ud->translated_length++] = 32;
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  ud->text_buffer[ud->text_length++] = 32;
  ud->text_buffer[ud->text_length++] = 32;
  k = lou_translateString (table,
			   &ud->text_buffer[0], &translationLength,
			   &ud->translated_buffer[ud->translated_length],
			   &translatedLength, (char *)
			   &ud->typeform[0], NULL, 0);
  memset (ud->typeform, 0, sizeof (ud->typeform));
  ud->text_length = 0;
  if (!k)
    {
      table = NULL;
      return 0;
    }
  if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
    ud->translated_length += translatedLength;
  else
    {
      ud->translated_length = MAX_TRANS_LENGTH;
      if (!write_paragraph (para, NULL))
	return 0;
    }
  return 1;
}

static int cellsOnLine;

static int
insertCharacters (char *chars, int length)
{
/* Put chars in outbuf, checking for overflow.*/
  int k;
  if (chars == NULL || length < 0)
    return 0;
  if (length == 0)
    return 1;
  if ((ud->outlen_so_far + length) >= ud->outlen)
    return 0;
  for (k = 0; k < length; k++)
    ud->outbuf[ud->outlen_so_far++] = (widechar) chars[k];
  cellsOnLine += length;
  return 1;
}

static int
insertDubChars (char *chars, int length)
{
/* Put chars in outbuf, checking for overflow.*/
  int k;
  if (chars == NULL || length < 0)
    return 0;
  while (length > 0 && chars[length - 1] == ' ')
    length--;
  cellsOnLine += length;
  if (length == 0)
    return 1;
  if ((ud->outlen_so_far + length) >= ud->outlen)
    return 0;
  switch (ud->format_for)
    {
    case textDevice:
      for (k = 0; k < length; k++)
	ud->outbuf[ud->outlen_so_far++] = (widechar) chars[k];
      break;
    case browser:
      for (k = 0; k < length; k++)
	{
	  if (chars[k] == '<')
	    {
	      if (!insertCharacters ("&lt;", 4))
		return 0;
	    }
	  else if (chars[k] == '&')
	    {
	      if (!insertCharacters ("&amp;", 5))
		return 0;
	    }
	  else
	    ud->outbuf[ud->outlen_so_far++] = (widechar) chars[k];
	}
      break;
    default:
      break;
    }
  return 1;
}

static int
insertWidechars (widechar * chars, int length)
{
/* Put chars in outbuf, checking for overflow.*/
  int k;
  if (chars == NULL || length < 0)
    return 0;
  while (length > 0 && chars[length - 1] == ' ')
    length--;
  cellsOnLine += length;
  if (length == 0)
    return 1;
  if ((ud->outlen_so_far + length) >= ud->outlen)
    return 0;
  switch (ud->format_for)
    {
    case textDevice:
      memcpy (&ud->outbuf[ud->outlen_so_far], chars, length * CHARSIZE);
      ud->outlen_so_far += length;
      break;
    case browser:
      for (k = 0; k < length; k++)
	{
	  if (chars[k] == '<')
	    {
	      if (!insertCharacters ("&lt;", 4))
		return 0;
	    }
	  else if (chars[k] == '&')
	    {
	      if (!insertCharacters ("&amp;", 5))
		return 0;
	    }
	  else
	    ud->outbuf[ud->outlen_so_far++] = chars[k];
	}
      break;
    default:
      break;
    }
  return 1;
}

static int startLine (void);
static int finishLine (void);

static int
makeBlankLines (int number, int beforeAfter)
{
  int availableCells;
  int k;
  if (number == 0)
    return 1;
  if (ud->braille_pages)
    {
      if (beforeAfter == 0 && (ud->lines_on_page == 0 ||
			       prevStyle->lines_after > 0
			       || prevStyle->action == document))
	return 1;
      else if (beforeAfter == 1 && (ud->lines_per_page - ud->lines_on_page -
				    number) < 2)
	return 1;
    }
  else
    {
      if (beforeAfter == 0 && (prevStyle->lines_after || prevStyle->action ==
			       document))
	return 1;
    }
  for (k = 0; k < number; k++)
    {
      availableCells = startLine ();
      if (!finishLine ())
	return 0;
    }
  return 1;
}

static int
fillPage (void)
{
  if (!ud->braille_pages)
    return 1;
  ud->fill_pages++;
  startLine ();
  write_outbuf ();
  return 1;
}

static int makePageSeparator (xmlChar * printPageNumber, int length);

static int
handlePagenum (xmlChar * printPageNumber, int length)
{
  int k;
  int kk;
  widechar translationBuffer[MAXNUMLEN];
  int translationLength = MAXNUMLEN - 1;
  widechar translatedBuffer[MAXNUMLEN];
  int translatedLength = MAXNUMLEN;
  widechar separatorLine[128];
  char setup[MAXNAMELEN];
  if (!*printPageNumber)
    {
      if (!ud->merge_unnumbered_pages)
	{
	  ud->print_page_number[0] = '_';
	  ud->print_page_number[1] = 0;
	  if (!ud->page_separator_number_first[0] || ud->ignore_empty_pages)
	    {
	      ud->page_separator_number_first[0] = '_';
	      ud->page_separator_number_first[1] = 0;
	    }
	}
      return 1;
    }
  strcpy (setup, " ");
  if (!(printPageNumber[0] >= '0' && printPageNumber[0] <= '9'))
    strcat (setup, LETSIGN);
  strcat (setup, printPageNumber);
  length = strlen (setup);
  utf8ToWc (setup, &length, &translationBuffer[0], &translationLength);
  if (!lou_translateString (ud->main_braille_table, translationBuffer,
			    &translationLength, translatedBuffer,
			    &translatedLength, NULL, NULL, 0))
    return 0;
  ud->print_page_number[0] = ' ';
  for (k = 1; k < translatedLength; k++)
    ud->print_page_number[k] = translatedBuffer[k];
  ud->print_page_number[k] = 0;
  if (!ud->page_separator_number_first[0] ||
      ud->page_separator_number_first[0] == '_' || ud->ignore_empty_pages)
    {
      for (k = 0; ud->print_page_number[k]; k++)
	ud->page_separator_number_first[k] = ud->print_page_number[k];
      ud->page_separator_number_first[k] = 0;
    }
  else
    {
      for (k = 0; ud->print_page_number[k]; k++)
	ud->page_separator_number_last[k] = ud->print_page_number[k];
      ud->page_separator_number_last[k] = 0;
    }
  return 1;
}

static void
getPrintPageString (void)
{
  int k;
  if (ud->print_page_number_first[0] != '_')
    {
      if (ud->print_page_number_first[0] != ' '
	  && ud->print_page_number_first[0] != '+')
	pageNumberString[pageNumberLength++] = ud->print_page_number_first[0];
    }
  for (k = 1; ud->print_page_number_first[k]; k++)
    pageNumberString[pageNumberLength++] = ud->print_page_number_first[k];
  if (ud->print_page_number_last[0])
    {
      pageNumberString[pageNumberLength++] = '-';
      for (k = 1; ud->print_page_number_last[k]; k++)
	pageNumberString[pageNumberLength++] = ud->print_page_number_last[k];
    }
}

static int getBraillePageString (void);

static int
getPageNumber (void)
{
  int k;
  int braillePageNumber = 0;
  int printPageNumber = 0;
  pageNumberLength = 0;
  if (ud->lines_on_page == 1)
    {
      if (ud->print_pages && ud->print_page_number_at
	  && ud->print_page_number_first[0] != '_')
	printPageNumber = 1;
      if (ud->braille_pages && !ud->braille_page_number_at
	  && currentBraillePageNumFormat != blank)
	braillePageNumber = 1;
    }
  else if (ud->lines_on_page == ud->lines_per_page)
    {
      if (ud->print_pages && !ud->print_page_number_at
	  && ud->print_page_number_first[0] != '_')
	printPageNumber = 1;
      if (ud->braille_pages && ud->braille_page_number_at
	  && currentBraillePageNumFormat != blank)
	braillePageNumber = 1;
    }
  if (printPageNumber || braillePageNumber)
    {
      pageNumberString[pageNumberLength++] = ' ';
      pageNumberString[pageNumberLength++] = ' ';
      if (printPageNumber)
	{
	  pageNumberString[pageNumberLength++] = ' ';
	  getPrintPageString ();
	}
      if (braillePageNumber)
	{
	  pageNumberString[pageNumberLength++] = ' ';
	  getBraillePageString ();
	}
    }
  return 1;
}

static void
addPagesToPrintPageNumber ()
{
  int k;
  if (ud->braille_pages && ud->page_separator_number_first[0])
    {
      if ((ud->lines_on_page == 0
	   && (ud->ignore_empty_pages
	       || ud->print_page_number_first[0] != ' '))
	  || (ud->lines_on_page == ud->lines_per_page)
	  || (ud->print_page_number_range
	      && ud->print_page_number_first[0] == '_'))
	{
	  for (k = 0; ud->page_separator_number_first[k]; k++)
	    ud->print_page_number_first[k] =
	      ud->page_separator_number_first[k];
	  ud->print_page_number_first[k] = 0;
	}
      else if (ud->page_separator_number_first[0] != '_'
	       && (ud->print_page_number_range
		   || (ud->lines_on_page == 0 && !ud->ignore_empty_pages)))
	{
	  for (k = 0; ud->page_separator_number_first[k]; k++)
	    ud->print_page_number_last[k] =
	      ud->page_separator_number_first[k];
	  ud->print_page_number_last[k] = 0;
	}
      if (ud->page_separator_number_last[0]
	  && (ud->print_page_number_range || ud->lines_on_page == 0))
	{
	  for (k = 0; ud->page_separator_number_last[k]; k++)
	    ud->print_page_number_last[k] = ud->page_separator_number_last[k];
	  ud->print_page_number_last[k] = 0;
	}
    }
  ud->page_separator_number_first[0] = 0;
  ud->page_separator_number_last[0] = 0;
}

static void
nextPrintPage (void)
{
  int k;
  int kk;
  widechar separatorLine[128];
  int pageSeparatorNumberFirstLength = 0;
  int pageSeparatorNumberLastLength = 0;
  if (ud->page_separator_number_first[0])
    {
      if (ud->braille_pages && ud->lines_on_page == 0)
	{

	}
      else if (!ud->page_separator)
	{

	}
      else if (ud->braille_pages &&
	       (ud->lines_on_page == ud->lines_per_page - 1))
	{
	  ud->lines_on_page++;
	  cellsOnLine = 0;
	  getPageNumber ();
	  finishLine ();
	}
      else if (ud->braille_pages &&
	       (ud->lines_on_page == ud->lines_per_page - 2) &&
	       (ud->footer_length > 0
		|| (ud->page_number_bottom_separate_line &&
		    ((ud->print_pages && !ud->print_page_number_at
		      && ud->print_page_number_first[0] != '_')
		     || (ud->braille_page_number_at
			 && currentBraillePageNumFormat != blank)))))
	{
	  ud->lines_on_page++;
	  insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
	  ud->lines_on_page++;
	  getPageNumber ();
	  finishLine ();
	}
      else if (ud->fill_pages > 0)
	{
	}
      else
	{
	  if (!ud->page_separator_number)
	    {
	      for (k = 0; k < ud->cells_per_line; k++)
		separatorLine[k] = '-';
	    }
	  else
	    {
	      for (k = 0; ud->page_separator_number_first[k] != 0; k++)
		pageSeparatorNumberFirstLength++;
	      for (k = 0; ud->page_separator_number_last[k] != 0; k++)
		pageSeparatorNumberLastLength++;
	      if (ud->ignore_empty_pages)
		{
		  pageSeparatorNumberLastLength = 0;
		}

	      k = 0;
	      while (k <
		     (ud->cells_per_line - pageSeparatorNumberFirstLength -
		      pageSeparatorNumberLastLength + 1))
		separatorLine[k++] = '-';
	      kk = 1;
	      while (k < (ud->cells_per_line - pageSeparatorNumberLastLength))
		separatorLine[k++] = ud->page_separator_number_first[kk++];
	      if (pageSeparatorNumberLastLength > 0)
		{
		  separatorLine[k++] = '-';
		  kk = 1;
		  while (k < (ud->cells_per_line))
		    separatorLine[k++] = ud->page_separator_number_last[kk++];
		}
	    }
	  insertWidechars (separatorLine, ud->cells_per_line);
	  insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
	  if (ud->braille_pages)
	    ud->lines_on_page++;
	  write_outbuf ();
	}
      addPagesToPrintPageNumber ();
    }
}

static void
continuePrintPageNumber (void)
{
  int k;
  if (ud->print_page_number[0] == '_')
    {
    }
  else if (!ud->continue_pages)
    ud->print_page_number[0] = '+';
  else if (ud->print_page_number[0] == ' ')
    ud->print_page_number[0] = 'a';
  else if (ud->print_page_number[0] == 'z')
    {
      ud->print_page_number[0] = '_';
      ud->print_page_number[1] = 0;
    }
  else
    ud->print_page_number[0]++;
  for (k = 0; ud->print_page_number[k]; k++)
    ud->print_page_number_first[k] = ud->print_page_number[k];
  ud->print_page_number_first[k] = 0;
  ud->print_page_number_last[0] = 0;
}

static int
nextBraillePage (void)
{
  if (ud->braille_pages)
    {
      if (ud->print_page_number_range && ud->print_pages
	  && ud->print_page_number_at)
	{
	  write_outbuf ();
	  ud->lines_on_page = 1;
	  cellsOnLine = 0;
	  getPageNumber ();
	  finishLine ();
	  writeOutbufDirect ();
	  writePagebuf ();
	}
      else
	write_outbuf ();
      if (!insertCharacters (ud->pageEnd, strlen (ud->pageEnd)))
	return 0;
      writeOutbufDirect ();
      ud->lines_on_page = 0;
      ud->braille_page_number++;
      continuePrintPageNumber ();
    }
  return 1;
}

static int
startLine (void)
{
  int availableCells = 0;
  while (availableCells == 0 || ud->fill_pages > 0)
    {
      ud->after_contents = 0;
      if (ud->page_separator_number_first[0])
	nextPrintPage ();
      if (!ud->braille_pages)
	return ud->cells_per_line;
      cellsOnLine = 0;
      ud->lines_on_page++;
      if (ud->lines_on_page == 1)
	{
	  currentBraillePageNumFormat = ud->brl_page_num_format;
	  getBraillePageString ();
	  if ((ud->print_page_number_range && ud->print_pages
	       && ud->print_page_number_at))
	    {
	      pageNumberLength = 0;
	      ud->lines_on_page++;
	      availableCells = ud->cells_per_line;
	    }
	  else
	    {
	      getPageNumber ();
	      if (ud->running_head_length > 0 ||
		  (style->skip_number_lines && pageNumberLength > 0) ||
		  (ud->page_number_top_separate_line && pageNumberLength > 0))
		availableCells = 0;
	      else
		availableCells = ud->cells_per_line - pageNumberLength;
	    }
	}
      else if (ud->lines_on_page == ud->lines_per_page)
	{
	  getPageNumber ();
	  if (ud->footer_length > 0 ||
	      (style->skip_number_lines && pageNumberLength > 0) ||
	      (ud->page_number_bottom_separate_line && pageNumberLength > 0))
	    availableCells = 0;
	  else
	    availableCells = ud->cells_per_line - pageNumberLength;
	}
      else
	{
	  pageNumberLength = 0;
	  availableCells = ud->cells_per_line;
	}
      if (availableCells == 0 || ud->fill_pages > 0)
	finishLine ();
      if (ud->fill_pages > 0 && ud->lines_on_page == 0)
	{
	  ud->fill_pages--;
	  if (ud->fill_pages == 0)
	    availableCells = ud->cells_per_line;
	  else
	    availableCells = 0;
	}
    }
  return availableCells;
}

static int
finishLine (void)
{
  int cellsToWrite = 0;
  int leaveBlank;
  for (leaveBlank = -1; leaveBlank < ud->line_spacing; leaveBlank++)
    {
      if (leaveBlank != -1)
	startLine ();
      if (ud->braille_pages)
	{
	  if (cellsOnLine > 0 && pageNumberLength > 0)
	    {
	      cellsToWrite =
		ud->cells_per_line - pageNumberLength - cellsOnLine;
	      if (!insertCharacters (blanks, cellsToWrite))
		return 0;
	      if (!insertWidechars (pageNumberString, pageNumberLength))
		return 0;
	    }
	  else if (ud->lines_on_page == 1)
	    {
	      if (ud->running_head_length > 0)
		{
		  cellsToWrite =
		    minimum (ud->running_head_length,
			     ud->cells_per_line - pageNumberLength);
		  if (!insertWidechars (ud->running_head, cellsToWrite))
		    return 0;
		  if (pageNumberLength)
		    {
		      cellsToWrite =
			ud->cells_per_line - pageNumberLength - cellsToWrite;
		      if (!insertCharacters (blanks, cellsToWrite))
			return 0;
		      if (!insertWidechars
			  (pageNumberString, pageNumberLength))
			return 0;
		    }
		}
	      else
		{
		  if (pageNumberLength)
		    {
		      cellsToWrite = ud->cells_per_line - pageNumberLength;
		      if (!insertCharacters (blanks, cellsToWrite))
			return 0;
		      if (!insertWidechars
			  (pageNumberString, pageNumberLength))
			return 0;
		    }
		}
	    }
	  else if (ud->lines_on_page == ud->lines_per_page)
	    {
	      if (ud->footer_length > 0)
		{
		  cellsToWrite =
		    minimum (ud->footer_length,
			     ud->cells_per_line - pageNumberLength);
		  if (!insertWidechars (ud->footer, cellsToWrite))
		    return 0;
		  if (pageNumberLength)
		    {
		      cellsToWrite =
			ud->cells_per_line - pageNumberLength - cellsToWrite;
		      if (!insertCharacters (blanks, cellsToWrite))
			return 0;
		      if (!insertWidechars
			  (pageNumberString, pageNumberLength))
			return 0;
		    }
		}
	      else
		{
		  if (pageNumberLength)
		    {
		      cellsToWrite = ud->cells_per_line - pageNumberLength;
		      if (!insertCharacters (blanks, cellsToWrite))
			return 0;
		      if (!insertWidechars
			  (pageNumberString, pageNumberLength))
			return 0;
		    }
		}
	    }
	}
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      if (ud->braille_pages && ud->lines_on_page == ud->lines_per_page)
	{
	  if (!nextBraillePage ())
	    return 0;
	}
    }
  return 1;
}

static int
writePagebuf (void)
{
  int k;
  unsigned char *utf8Str;
  if (ud->print_page_number_range && ud->print_page_number_at)
    {
      if (ud->pagelen_so_far > 0 && ud->outFile != NULL)
	{
	  switch (ud->output_encoding)
	    {
	    case utf8:
	      for (k = 0; k < ud->pagelen_so_far; k++)
		{
		  utf8Str = utfwcto8 (ud->pagebuf[k]);
		  fwrite (utf8Str, strlen ((char *) utf8Str), 1, ud->outFile);
		}
	      break;
	    case utf16:
	      for (k = 0; k < ud->pagelen_so_far; k++)
		{
		  unsigned short uc16 = (unsigned short) ud->pagebuf[k];
		  fwrite (&uc16, 1, sizeof (uc16), ud->outFile);
		}
	      break;
	    case utf32:
	      for (k = 0; k < ud->pagelen_so_far; k++)
		{
		  unsigned int uc32 = (unsigned int) ud->pagebuf[k];
		  fwrite (&uc32, 1, sizeof (uc32), ud->outFile);
		}
	      break;
	    case ascii8:
	      for (k = 0; k < ud->pagelen_so_far; k++)
		{
		  fputc ((char) ud->pagebuf[k], ud->outFile);
		}
	      break;
	    default:
	      break;
	    }
	  ud->pagelen_so_far = 0;
	}
    }
  return 1;
}

static int
writeOutbufDirect (void)
{
  /* outbuf not appended to pagebuf, but written directly */
  int k;
  unsigned char *utf8Str;
  if (ud->outlen_so_far == 0 || ud->outFile == NULL)
    return 1;			/*output stays in ud->outbuf */
  switch (ud->output_encoding)
    {
    case utf8:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  utf8Str = utfwcto8 (ud->outbuf[k]);
	  fwrite (utf8Str, strlen ((char *) utf8Str), 1, ud->outFile);
	}
      break;
    case utf16:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  unsigned short uc16 = (unsigned short) ud->outbuf[k];
	  fwrite (&uc16, 1, sizeof (uc16), ud->outFile);
	}
      break;
    case utf32:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  unsigned int uc32 = (unsigned int) ud->outbuf[k];
	  fwrite (&uc32, 1, sizeof (uc32), ud->outFile);
	}
      break;
    case ascii8:
      for (k = 0; k < ud->outlen_so_far; k++)
	fputc ((char) ud->outbuf[k], ud->outFile);
      break;
    default:
      break;
    }
  ud->outlen_so_far = 0;
  return 1;
}

void
insert_text (xmlNode * node)
{
  int length;
  int wcLength;
  if (ud->contains_utd)
    return;
  for (length = strlen ((char *) node->content); length > 0 &&
       node->content[length - 1] <= 32; length--);
  if (length <= 0)
    return;
  if (ud->format_for == utd)
    return (utd_insert_text (node, length));
  switch (ud->stack[ud->top])
    {
    case notranslate:
      insert_translation (ud->main_braille_table);
      insert_utf8 (node->content);
      if ((ud->translated_length + ud->text_length) > MAX_TRANS_LENGTH)
	ud->text_length = MAX_TRANS_LENGTH - ud->translated_length;
      memcpy (&ud->translated_buffer[ud->translated_length], ud->text_buffer,
	      ud->text_length * CHARSIZE);
      ud->translated_length += ud->text_length;
      ud->text_length = 0;
      return;
    case pagenum:
      if (!ud->print_pages)
	return;
      fineFormat ();
      makePageSeparator (node->content, length);
      return;
    default:
      break;
    }
  wcLength = insert_utf8 (node->content);
  switch (ud->stack[ud->top])
    {
    case italicx:
      if (!(ud->emphasis & italic))
	break;
      memset (&ud->typeform[ud->old_text_length], italic, wcLength);
      break;
    case underlinex:
      if (!(ud->emphasis & underline))
	break;
      memset (&ud->typeform[ud->old_text_length], underline, wcLength);
      break;
    case boldx:
      if (!(ud->emphasis & bold))
	break;
      memset (&ud->typeform[ud->old_text_length], bold, wcLength);
      break;
    case compbrl:

      if (!(ud->emphasis & computer_braille))
	break;
      memset (&ud->typeform[ud->old_text_length], computer_braille, wcLength);
      break;
    default:
      break;
    }
}

static int
getBraillePageString (void)
{
  int k;
  char brlPageString[MAXNUMLEN];
  widechar translationBuffer[MAXNUMLEN];
  int translationLength;
  int translatedLength = MAXNUMLEN;
  switch (ud->brl_page_num_format)
    {
    case none:
      pageNumberLength = 0;
      return 1;
    default:
    case normal:
      translationLength =
	sprintf (brlPageString, "%d", ud->braille_page_number);
      break;
    case p:
      translationLength = sprintf (brlPageString, "p%d",
				   ud->braille_page_number);
      break;
    case roman:
      strcpy (brlPageString, LETSIGN);
      strcat (brlPageString, makeRomanNumber (ud->braille_page_number));
      translationLength = strlen (brlPageString);
      break;
    }
  for (k = 0; k < translationLength; k++)
    translationBuffer[k] = brlPageString[k];
  if (!lou_translateString (ud->main_braille_table, translationBuffer,
			    &translationLength, ud->braille_page_string,
			    &translatedLength, NULL, NULL, 0))
    return 0;
  ud->braille_page_string[translatedLength] = 0;
  for (pageNumberLength = 0; pageNumberLength < 3; pageNumberLength++)
    pageNumberString[pageNumberLength] = ' ';
  for (k = 0; k < translatedLength; k++)
    pageNumberString[pageNumberLength++] = ud->braille_page_string[k];
  return 1;
}

static char *
makeRomanNumber (int n)
{
  static char romNum[40];
  static const char *hundreds[] = {
    "",
    "c",
    "cc",
    "ccc",
    "cd",
    "d",
    "dc",
    "dcc",
    "dccc",
    "cm",
    "m"
  };
  static const char *tens[] = {
    "",
    "x",
    "xx",
    "xxx",
    "xl",
    "l",
    "lx",
    "lxx",
    "lxxx",
    "xc"
  };
  static const char *units[] = {
    "",
    "i",
    "ii",
    "iii",
    "iv",
    "v",
    "vi",
    "vii",
    "viii",
    "ix"
  };
  if (n <= 0 || n > 1000)
    return NULL;
  romNum[0] = 0;
  strcat (romNum, hundreds[n / 100]);
  strcat (romNum, tens[(n / 10) % 10]);
  strcat (romNum, units[n % 10]);
  return romNum;
}

static int
makeBlankPage (void)
{
  if (!ud->braille_pages)
    return 1;
  if (ud->format_for != utd)
    {
      if (!makeBlankLines (ud->lines_per_page, 2))
	return 0;
    }
  else
    {
      if (!utd_makeBlankLines (ud->lines_per_page, 2))
	return 0;
    }
  return 1;
}

int
write_outbuf (void)
{
  int k;
  unsigned char *utf8Str;
  if (ud->outlen_so_far == 0 || ud->outFile == NULL)
    return 1;			/*output stays in ud->outbuf */
  switch (ud->output_encoding)
    {
    case utf8:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  utf8Str = utfwcto8 (ud->outbuf[k]);
	  fwrite (utf8Str, strlen ((char *) utf8Str), 1, ud->outFile);
	}
      break;
    case utf16:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  unsigned short uc16 = (unsigned short) ud->outbuf[k];
	  fwrite (&uc16, 1, sizeof (uc16), ud->outFile);
	}
      break;
    case utf32:
      for (k = 0; k < ud->outlen_so_far; k++)
	{
	  unsigned int uc32 = (unsigned int) ud->outbuf[k];
	  fwrite (&uc32, 1, sizeof (uc32), ud->outFile);
	}
      break;
    default:
    case ascii8:
      for (k = 0; k < ud->outlen_so_far; k++)
	fputc ((char) ud->outbuf[k], ud->outFile);
      break;
    }
  ud->outlen_so_far = 0;
  return 1;
}

static widechar *translatedBuffer;
static int translationLength;
static int translatedLength;

static int
hyphenatex (int lastBlank, int lineEnd)
{
  char hyphens[MAXNAMELEN];
  int k;
  int wordStart = lastBlank + 1;
  int wordLength;
  int breakAt = 0;
  int hyphenFound = 0;
  if ((translatedLength - wordStart) < 12)
    return 0;
  for (wordLength = wordStart; wordLength < translatedLength; wordLength++)
    if (translatedBuffer[wordLength] == ' ')
      break;
  wordLength -= wordStart;
  if (wordLength < 5 || wordLength > ud->cells_per_line)
    return 0;
  for (k = wordLength - 2; k >= 0; k--)
    if ((wordStart + k) < lineEnd && translatedBuffer[wordStart + k] ==
	*litHyphen && !hyphenFound)
      {
	hyphens[k + 1] = '1';
	hyphenFound = 1;
      }
    else
      hyphens[k + 1] = '0';
  hyphens[wordLength] = 0;
  if (!hyphenFound)
    {
      if (!lou_hyphenate (ud->main_braille_table,
			  &translatedBuffer[wordStart], wordLength,
			  hyphens, 1))
	return 0;
    }
  for (k = strlen (hyphens) - 2; k > 0; k--)
    {
      breakAt = wordStart + k;
      if (hyphens[k] == '1' && breakAt < lineEnd)
	break;
    }
  if (k < 2)
    return 0;
  return breakAt;
}

#define escapeChar 0x1b
#define MAXCOLS 100
#define MAXROWSIZE 400
#define COLSPACING 2

static int
doAlignColumns ()
{
  int numRows = 0;
  int rowNum = 0;
  int numCols = 0;
  int colNum = 0;
  int colLength = 0;
  int rowLength;
  int colSize[MAXCOLS];
  widechar rowBuf[MAXROWSIZE];
  int bufPos;
  int k;
  unsigned int ch;
  int rowEnd = 0;
  for (bufPos = 0; bufPos < translatedLength; bufPos++)
    if (translatedBuffer[bufPos] == escapeChar)
      break;
  if (bufPos >= translatedLength)
    {
      doLeftJustify ();
      return 1;
    }
  for (k = 0; k < MAXCOLS; k++)
    colSize[k] = 0;

  /*Calculate number of columns and column sizes */
  while (bufPos < translatedLength)
    {
      ch = translatedBuffer[bufPos++];
      if (ch == escapeChar)
	{
	  unsigned int nch = translatedBuffer[bufPos];
	  if (nch == 'r')	/*End of row */
	    {
	      numRows++;
	      if (rowEnd == 0)
		rowEnd = colLength;
	      colLength = 0;
	      colNum = 0;
	      bufPos++;
	    }
	  else if (nch == 'c')
	    {
	      if (numRows == 0)
		numCols++;
	      if (colSize[colNum] < colLength)
		colSize[colNum] = colLength;
	      colNum++;
	      colLength = 0;
	      bufPos++;
	    }
	  else if (nch == 'e')
	    break;
	}
      else
	colLength++;
    }
  colSize[numCols - 1] += rowEnd;
  if (style->format == alignColumnsLeft)
    {
      /*Calculate starting points of columns in output */
      int colStart = 0;
      for (colNum = 0; colNum < numCols; colNum++)
	{
	  k = colSize[colNum];
	  colSize[colNum] = colStart;
	  colStart += k;
	  if (colNum != (numCols - 1))
	    colStart += COLSPACING;
	}
    }
  else
    {
      /*Calculate ending points of columns in output */
      int colEnd = colSize[0];
      for (colNum = 1; colNum < numCols; colNum++)
	{
	  colEnd += colSize[colNum] + COLSPACING;
	  colSize[colNum] = colEnd;
	}
    }

/*Now output the stuff.*/
  if ((ud->lines_per_page - ud->lines_on_page) < numRows)
    fillPage ();
  bufPos = 0;
  for (rowNum = 0; rowNum < numRows; rowNum++)
    {
      int charactersWritten = 0;
      int cellsToWrite = 0;
      int availableCells = 0;
      rowLength = 0;
      if (style->format == alignColumnsLeft)
	{
	  for (colNum = 0; colNum < numCols; colNum++)
	    {
	      while (rowLength < MAXROWSIZE
		     && translatedBuffer[bufPos] != escapeChar)
		rowBuf[rowLength++] = translatedBuffer[bufPos++];
	      bufPos += 2;
	      if (colNum < (numCols - 1))
		{
		  while (rowLength < MAXROWSIZE && rowLength <
			 colSize[colNum + 1])
		    rowBuf[rowLength++] = ' ';
		}
	      else
		{
		  while (rowLength < MAXROWSIZE
			 && translatedBuffer[bufPos] != escapeChar)
		    rowBuf[rowLength++] = translatedBuffer[bufPos++];
		  bufPos += 2;	/*actual end of row */
		}
	    }
	}
      else
	{
	  int prevBufPos = bufPos;
	  int prevCol = 0;
	  for (colNum = 0; colNum < numCols; colNum++)
	    {
	      while (translatedBuffer[bufPos] != escapeChar)
		bufPos++;
	      for (k = bufPos - 1; k >= prevBufPos; k--)
		rowBuf[k + prevCol] = translatedBuffer[k];
	      for (; k >= prevCol; k--)
		rowBuf[k + prevCol] = ' ';
	      prevBufPos = bufPos + 2;
	      prevCol = colSize[colNum];
	      rowLength += colSize[colNum];
	      if (rowLength > MAXROWSIZE)
		break;
	    }
	  while (rowLength < MAXROWSIZE && translatedBuffer[bufPos] !=
		 escapeChar)
	    rowBuf[rowLength++] = translatedBuffer[bufPos++];
	  bufPos += 2;
	}
      while (charactersWritten < rowLength)
	{
	  int rowTooLong = 0;
	  availableCells = startLine ();
	  if ((charactersWritten + availableCells) >= rowLength)
	    cellsToWrite = rowLength - charactersWritten;
	  else
	    {
	      for (cellsToWrite = availableCells; cellsToWrite > 0;
		   cellsToWrite--)
		if (rowBuf[charactersWritten + cellsToWrite] == ' ')
		  break;
	      if (cellsToWrite == 0)
		{
		  cellsToWrite = availableCells - 1;
		  rowTooLong = 1;
		}
	    }
	  while (rowBuf[charactersWritten + cellsToWrite] == ' ')
	    cellsToWrite--;
	  if (cellsToWrite == 0)
	    break;
	  for (k = charactersWritten;
	       k < (charactersWritten + cellsToWrite); k++)
	    if (rowBuf[k] == 0xa0)	/*unbreakable space */
	      rowBuf[k] = 0x20;	/*space */
	  if (!insertWidechars (&rowBuf[charactersWritten], cellsToWrite))
	    return 0;
	  charactersWritten += cellsToWrite;
	  if (rowTooLong)
	    {
	      if (!insertDubChars (litHyphen, strlen (litHyphen)))
		return 0;
	    }
	  finishLine ();
	}
    }
  return 1;
}

static int
doListColumns (void)
{
  widechar *thisRow;
  int rowLength;
  int bufPos;
  int prevPos = 0;
  for (bufPos = 0; bufPos < translatedLength; bufPos++)
    if (translatedBuffer[bufPos] == escapeChar)
      break;
  if (bufPos >= translatedLength)
    {
      doLeftJustify ();
      return 1;
    }
  for (; bufPos < translatedLength; bufPos++)
    {
      if (translatedBuffer[bufPos] == escapeChar &&
	  translatedBuffer[bufPos + 1] == escapeChar)
	{
	  int charactersWritten = 0;
	  int cellsToWrite = 0;
	  int availableCells = 0;
	  int k;
	  thisRow = &translatedBuffer[prevPos];
	  rowLength = bufPos - prevPos - 1;
	  prevPos = bufPos + 2;
	  while (charactersWritten < rowLength)
	    {
	      int wordTooLong = 0;
	      int breakAt = 0;
	      int leadingBlanks = 0;
	      availableCells = startLine ();
	      if (styleSpec->status == startBody)
		{
		  if (style->first_line_indent < 0)
		    leadingBlanks = 0;
		  else
		    leadingBlanks =
		      style->left_margin + style->first_line_indent;
		  styleSpec->status = resumeBody;
		}
	      else
		leadingBlanks = style->left_margin;
	      if (!insertCharacters (blanks, leadingBlanks))
		return 0;
	      availableCells -= leadingBlanks;
	      if ((charactersWritten + availableCells) >= rowLength)
		cellsToWrite = rowLength - charactersWritten;
	      else
		{
		  for (cellsToWrite = availableCells; cellsToWrite > 0;
		       cellsToWrite--)
		    if (thisRow[charactersWritten + cellsToWrite] == ' ')
		      break;
		  if (cellsToWrite == 0)
		    {
		      cellsToWrite = availableCells - 1;
		      wordTooLong = 1;
		    }
		  else
		    {
		      if (ud->hyphenate)
			breakAt =
			  hyphenatex (charactersWritten + cellsToWrite,
				      charactersWritten + availableCells);
		      if (breakAt)
			cellsToWrite = breakAt - charactersWritten;
		    }
		}
	      for (k = charactersWritten;
		   k < (charactersWritten + cellsToWrite); k++)
		if (thisRow[k] == 0xa0)	/*unbreakable space */
		  thisRow[k] = 0x20;	/*space */
	      if (!insertWidechars
		  (&thisRow[charactersWritten], cellsToWrite))
		return 0;
	      charactersWritten += cellsToWrite;
	      if (thisRow[charactersWritten] == ' ')
		charactersWritten++;
	      if ((breakAt && thisRow[breakAt - 1] != *litHyphen)
		  || wordTooLong)
		{
		  if (!insertDubChars (litHyphen, strlen (litHyphen)))
		    return 0;
		}
	      finishLine ();
	    }
	}
      else if (translatedBuffer[bufPos - 1] !=
	       escapeChar && translatedBuffer[bufPos] == escapeChar)
	translatedBuffer[bufPos] = ' ';
    }
  return 1;
}

static int
doListLines (void)
{
  widechar *thisLine;
  int lineLength;
  int bufPos;
  int prevPos = 0;
  for (bufPos = 0; bufPos < translatedLength; bufPos++)
    if (translatedBuffer[bufPos] == escapeChar)
      break;
  if (bufPos >= translatedLength)
    {
      doLeftJustify ();
      return 1;
    }
  for (; bufPos < translatedLength; bufPos++)
    if (translatedBuffer[bufPos] == escapeChar && translatedBuffer[bufPos + 1]
	== escapeChar)
      {
	int charactersWritten = 0;
	int cellsToWrite = 0;
	int availableCells = 0;
	int k;
	thisLine = &translatedBuffer[prevPos];
	lineLength = bufPos - prevPos - 1;
	prevPos = bufPos + 2;
	while (charactersWritten < lineLength)
	  {
	    int wordTooLong = 0;
	    int breakAt = 0;
	    int leadingBlanks = 0;
	    availableCells = startLine ();
	    if (styleSpec->status == startBody)
	      {
		if (style->first_line_indent < 0)
		  leadingBlanks = 0;
		else
		  leadingBlanks =
		    style->left_margin + style->first_line_indent;
		styleSpec->status = resumeBody;
	      }
	    else
	      leadingBlanks = style->left_margin;
	    if (!insertCharacters (blanks, leadingBlanks))
	      return 0;
	    availableCells -= leadingBlanks;
	    if ((charactersWritten + availableCells) >= lineLength)
	      cellsToWrite = lineLength - charactersWritten;
	    else
	      {
		for (cellsToWrite = availableCells; cellsToWrite > 0;
		     cellsToWrite--)
		  if (thisLine[charactersWritten + cellsToWrite] == ' ')
		    break;
		if (cellsToWrite == 0)
		  {
		    cellsToWrite = availableCells - 1;
		    wordTooLong = 1;
		  }
		else
		  {
		    if (ud->hyphenate)
		      breakAt =
			hyphenatex (charactersWritten + cellsToWrite,
				    charactersWritten + availableCells);
		    if (breakAt)
		      cellsToWrite = breakAt - charactersWritten;
		  }
	      }
	    for (k = charactersWritten;
		 k < (charactersWritten + cellsToWrite); k++)
	      if (thisLine[k] == 0xa0)	/*unbreakable space */
		thisLine[k] = 0x20;	/*space */
	    if (!insertWidechars (&thisLine[charactersWritten], cellsToWrite))
	      return 0;
	    charactersWritten += cellsToWrite;
	    if (thisLine[charactersWritten] == ' ')
	      charactersWritten++;
	    if ((breakAt && thisLine[breakAt - 1] != *litHyphen)
		|| wordTooLong)
	      {
		if (!insertDubChars (litHyphen, strlen (litHyphen)))
		  return 0;
	      }
	    finishLine ();
	  }
      }
  return 1;
}

static int
doComputerCode (void)
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (translatedBuffer[charactersWritten] == 0x0a)
    charactersWritten++;
  while (charactersWritten < translatedLength)
    {
      int lineTooLong = 0;
      availableCells = startLine ();
      for (cellsToWrite = 0; cellsToWrite < availableCells; cellsToWrite++)
	if ((charactersWritten + cellsToWrite) >= translatedLength
	    || translatedBuffer[charactersWritten + cellsToWrite] == 0x0a)
	  break;
      if ((charactersWritten + cellsToWrite) > translatedLength)
	cellsToWrite--;
      if (cellsToWrite <= 0 && translatedBuffer[charactersWritten] != 0x0a)
	break;
      if (cellsToWrite == availableCells &&
	  translatedBuffer[charactersWritten + cellsToWrite] != 0x0a)
	{
	  cellsToWrite = availableCells - strlen (compHyphen);
	  lineTooLong = 1;
	}
      if (translatedBuffer[charactersWritten + cellsToWrite] == 0x0a)
	translatedBuffer[charactersWritten + cellsToWrite] = ' ';
      for (k = charactersWritten; k < (charactersWritten + cellsToWrite); k++)
	if (translatedBuffer[k] == 0xa0)	/*unbreakable space */
	  translatedBuffer[k] = 0x20;	/*space */
      if (!insertWidechars
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == ' ')
	charactersWritten++;
      if (lineTooLong)
	{
	  if (!insertDubChars (compHyphen, strlen (compHyphen)))
	    return 0;
	}
      finishLine ();
    }
  return 1;
}

static int
doLeftJustify (void)
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (charactersWritten < translatedLength)
    {
      int wordTooLong = 0;
      int breakAt = 0;
      int leadingBlanks = 0;
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = style->left_margin + style->first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = style->left_margin;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      if ((charactersWritten + availableCells) >= translatedLength)
	cellsToWrite = translatedLength - charactersWritten;
      else
	{
	  for (cellsToWrite = availableCells; cellsToWrite > 0;
	       cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == ' ')
	      break;
	  if (cellsToWrite == 0)
	    {
	      cellsToWrite = availableCells - 1;
	      wordTooLong = 1;
	    }
	  else
	    {
	      if (ud->hyphenate)
		breakAt =
		  hyphenatex (charactersWritten + cellsToWrite,
			      charactersWritten + availableCells);
	      if (breakAt)
		cellsToWrite = breakAt - charactersWritten;
	    }
	}
      for (k = charactersWritten; k < (charactersWritten + cellsToWrite); k++)
	if (translatedBuffer[k] == 0xa0)	/*unbreakable space */
	  translatedBuffer[k] = 0x20;	/*space */
      if (!insertWidechars
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == ' ')
	charactersWritten++;
      if ((breakAt && translatedBuffer[breakAt - 1] != *litHyphen)
	  || wordTooLong)
	{
	  if (!insertDubChars (litHyphen, strlen (litHyphen)))
	    return 0;
	}
      finishLine ();
    }
  return 1;
}

static int
doContents (void)
{
  int lastWord;
  int lastWordLength;
  int untilLastWord;
  int numbersStart;
  int numbersLength;
  int leadingBlanks = 0;
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  if (translatedBuffer[translatedLength - 1] == 0xa0)
    {
      /* No page numbers */
      translatedLength--;
      doLeftJustify ();
      return 1;
    }
  for (k = translatedLength - 1; k > 0 && translatedBuffer[k] != 32; k--);
  if (k == 0)
    {
      doLeftJustify ();
      return 1;
    }
  numbersStart = k + 1;
  numbersLength = translatedLength - numbersStart;
  for (--k; k >= 0 && translatedBuffer[k] > 32; k--);
  lastWord = k + 1;
  lastWordLength = numbersStart - lastWord;
  for (k = numbersStart; k < translatedLength; k++)
    if (translatedBuffer[k] == 0xa0)
      translatedBuffer[k] = ' ';
  untilLastWord = lastWord - 1;
  while (charactersWritten < untilLastWord)
    {
      int wordTooLong = 0;
      int breakAt = 0;
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = style->left_margin + style->first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = style->left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      if ((charactersWritten + availableCells) >= untilLastWord)
	cellsToWrite = untilLastWord - charactersWritten;
      else
	{
	  for (cellsToWrite = availableCells - 2; cellsToWrite > 0;
	       cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == ' ')
	      break;
	  if (cellsToWrite == 0)
	    {
	      cellsToWrite = availableCells - 1;
	      wordTooLong = 1;
	    }
	  else
	    {
	      if (ud->hyphenate)
		breakAt =
		  hyphenatex (charactersWritten + cellsToWrite,
			      charactersWritten + availableCells);
	      if (breakAt)
		cellsToWrite = breakAt - charactersWritten;
	    }
	}
      for (k = charactersWritten; k < (charactersWritten + cellsToWrite); k++)
	if (translatedBuffer[k] == 0xa0)	/*unbreakable space */
	  translatedBuffer[k] = 0x20;	/*space */
      if (!insertWidechars
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == ' ')
	charactersWritten++;
      if ((breakAt && translatedBuffer[breakAt - 1] != *litHyphen)
	  || wordTooLong)
	{
	  if (!insertDubChars (litHyphen, strlen (litHyphen)))
	    return 0;
	}
      if (charactersWritten < untilLastWord)
	finishLine ();
      else
	{
	  availableCells -= cellsToWrite;
	  if (availableCells <= 0)
	    {
	      finishLine ();
	      availableCells = 0;
	    }
	}
    }
  if (availableCells == 0)
    {
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = style->left_margin + style->first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = style->left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
    }
  if ((lastWordLength + numbersLength + 2) < availableCells)
    {
      insertCharacters (blanks, 1);
      availableCells--;
      if (!insertWidechars (&translatedBuffer[lastWord], lastWordLength))
	return 0;
      availableCells -= lastWordLength;
      if ((availableCells - numbersLength) < 3)
	insertCharacters (blanks, availableCells - numbersLength);
      else
	{
	  insertCharacters (blanks, 1);
	  for (k = availableCells - (numbersLength + 1); k > 0; k--)
	    insertCharacters (&ud->line_fill, 1);
	  insertCharacters (blanks, 1);
	}
      if (!insertWidechars (&translatedBuffer[numbersStart], numbersLength))
	return 0;
      finishLine ();
    }
  else
    {
      finishLine ();
      availableCells = startLine ();
      leadingBlanks = style->left_margin;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      if (!insertWidechars (&translatedBuffer[lastWord], lastWordLength))
	return 0;
      availableCells -= lastWordLength;
      if ((availableCells - numbersLength) < 3)
	insertCharacters (blanks, availableCells - numbersLength);
      else
	{
	  insertCharacters (blanks, 1);
	  for (k = availableCells - (numbersLength + 1); k > 0; k--)
	    insertCharacters (&ud->line_fill, 1);
	  insertCharacters (blanks, 1);
	}
      if (!insertWidechars (&translatedBuffer[numbersStart], numbersLength))
	return 0;
      finishLine ();
    }
  return 1;
}

static int
doCenterRight (void)
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (charactersWritten < translatedLength)
    {
      int wordTooLong = 0;
      availableCells = startLine ();
      if ((translatedLength - charactersWritten) < availableCells)
	{
	  k = (availableCells - (translatedLength - charactersWritten));
	  if (style->format == centered)
	    k /= 2;
	  else if (style->format != rightJustified)
	    return 0;
	  if (!insertCharacters (blanks, k))
	    return 0;
	  if (!insertWidechars (&translatedBuffer[charactersWritten],
				translatedLength - charactersWritten))
	    return 0;
	  finishLine ();
	  break;
	}
      if ((charactersWritten + availableCells) > translatedLength)
	cellsToWrite = translatedLength - charactersWritten;
      else
	{
	  for (cellsToWrite = availableCells; cellsToWrite > 0;
	       cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == ' ')
	      break;
	  if (cellsToWrite == 0)
	    {
	      cellsToWrite = availableCells - 1;
	      wordTooLong = 1;
	    }
	}
      for (k = charactersWritten; k < (charactersWritten + cellsToWrite); k++)
	if (translatedBuffer[k] == 0xa0)	/*unbreakable space */
	  translatedBuffer[k] = 0x20;	/*space */
      if (!wordTooLong)
	{
	  k = availableCells - cellsToWrite;
	  if (style->format == centered)
	    k /= 2;
	  if (!insertCharacters (blanks, k))
	    return 0;
	}
      if (!insertWidechars
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == ' ')
	charactersWritten++;
      if (wordTooLong)
	{
	  if (!insertDubChars (litHyphen, strlen (litHyphen)))
	    return 0;
	}
      finishLine ();
    }
  return 1;
}

static int
editTrans (void)
{
  if (!(ud->contents == 2) && !(style->format == computerCoded) &&
      ud->edit_table_name && (ud->has_math || ud->has_chem || ud->has_music))
    {
      translationLength = ud->translated_length;
      translatedLength = MAX_TRANS_LENGTH;
      if (!lou_translateString (ud->edit_table_name,
				ud->translated_buffer,
				&translationLength, ud->text_buffer,
				&translatedLength, NULL, NULL, 0))
	{
	  ud->edit_table_name = NULL;
	  return 0;
	}
      translatedBuffer = ud->text_buffer;
    }
  else
    {
      translatedBuffer = ud->translated_buffer;
      translatedLength = ud->translated_length;
    }
  return 1;
}

static int
startStyle (void)
{
/*Line or page skipping before body*/
  styleSpec->status = startBody;
  if (ud->format_for == utd)
    return (utd_startStyle ());
  if (!ud->paragraphs)
    return 1;
  if (ud->braille_pages && prevStyle->action != document)
    {
      if (style->righthand_page)
	{
	  fillPage ();
	  if (ud->interpoint && !(ud->braille_page_number & 1))
	    makeBlankPage ();
	}
      else if (style->newpage_before)
	fillPage ();
      else if (style->lines_before > 0
	       && prevStyle->lines_after == 0 && ud->lines_on_page > 0)
	{
	  if ((ud->lines_per_page - ud->lines_on_page) < 2)
	    fillPage ();
	  else if (!makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  else
    {
      if (style->lines_before > 0 && prevStyle->lines_after == 0 &&
	  prevStyle->action != document)
	{
	  if (!makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  write_outbuf ();
  return 1;
}

static int
styleBody (void)
{
  sem_act action = style->action;
  if (ud->format_for == utd)
    return (utd_styleBody ());
  while (ud->translated_length > 0 &&
	 ud->translated_buffer[ud->translated_length - 1] <= 32)
    ud->translated_length--;
  if (ud->translated_length == 0)
    return 1;
  if (!editTrans ())
    return 0;
  if (style->format != computerCoded && action != document)
    {
      int realStart;
      for (realStart = 0; realStart < translatedLength &&
	   translatedBuffer[realStart] <= 32 &&
	   translatedBuffer[realStart] != escapeChar; realStart++);
      if (realStart > 0)
	{
	  translatedBuffer = &translatedBuffer[realStart];
	  translatedLength -= realStart;
	}
    }
  while (translatedLength > 0
	 && translatedBuffer[translatedLength - 1] <= 32 &&
	 translatedBuffer[translatedLength - 1] != escapeChar)
    translatedLength--;
  if (translatedLength <= 0)
    {
      ud->translated_length = 0;
      return 1;
    }
  if (!ud->paragraphs)
    {
      cellsOnLine = 0;
      if (!insertWidechars (translatedBuffer, translatedLength))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      write_outbuf ();
      ud->translated_length = 0;
      return 1;
    }
  if (action == contentsheader && ud->contents != 2)
    {
      initialize_contents ();
      start_heading (action, translatedBuffer, translatedLength);
      finish_heading (action);
      ud->text_length = 0;
      ud->translated_length = 0;
      return 1;
    }
  if (ud->contents == 1)
    {
      if (ud->braille_pages && ud->braille_page_number_at && (action ==
							      heading1
							      || action ==
							      heading2
							      || action ==
							      heading3
							      || action ==
							      heading4))
	getBraillePageString ();
      start_heading (action, translatedBuffer, translatedLength);
    }
  switch (style->format)
    {
    case centered:
    case rightJustified:
      doCenterRight ();
      break;
    case alignColumnsLeft:
    case alignColumnsRight:
      doAlignColumns ();
      break;
    case listColumns:
      doListColumns ();
      break;
    case listLines:
      doListLines ();
      break;
    case computerCoded:
      doComputerCode ();
      break;
    case contents:
      doContents ();
      break;
    case leftJustified:
    default:
      doLeftJustify ();
      break;
    }
  write_outbuf ();
  if (ud->contents == 1)
    finish_heading (action);
  styleSpec->status = resumeBody;
  ud->translated_length = 0;
  return 1;
}

static int
finishStyle (void)
{
/*Skip lines or pages after body*/
  if (ud->format_for == utd)
    return (utd_finishStyle ());
  if (ud->braille_pages)
    {
      if (style->newpage_after)
	fillPage ();
      else if (style->lines_after > 0)
	{
	  if ((ud->lines_per_page - ud->lines_on_page) < 2)
	    fillPage ();
	  else
	    {
	      if (!makeBlankLines (style->lines_after, 1))
		return 0;
	    }
	}
    }
  else
    {
      if (style->lines_after)
	{
	  if (!makeBlankLines (style->lines_after, 1))
	    return 0;
	}
    }
  write_outbuf ();
  return 1;
}

int
write_paragraph (sem_act action, xmlNode * node)
{
  StyleType *holdStyle;
  if (ud->contains_utd)
    return 1;
  if (!((ud->text_length > 0 || ud->translated_length > 0) &&
	ud->style_top >= 0))
    return 1;
  holdStyle = action_to_style (action);
  if (holdStyle == NULL)
    holdStyle = lookup_style ("para");
  /* We must do some of the work of start_styl* */
  if (ud->style_top < (STACKSIZE - 2))
    ud->style_top++;
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style = holdStyle;
  styleSpec->node = node;
  styleSpec->status = beforeBody;
  if (style->brlNumFormat != normal)
    ud->brl_page_num_format = style->brlNumFormat;
  styleSpec->curBrlNumFormat = ud->brl_page_num_format;
  startStyle ();
  insert_translation (ud->main_braille_table);
  styleBody ();
  end_style ();
  return 1;
}

static char *xmlTags[] = {
  "<pagenum>", "</pagenum>", NULL
};

static int
insertEscapeChars (int number)
{
  int k;
  if (number <= 0)
    return 0;
  if ((ud->text_length + number) >= MAX_LENGTH)
    return 0;
  for (k = 0; k < number; k++)
    ud->text_buffer[ud->text_length++] = (widechar) escapeChar;
  return 1;
}

static int
makeParagraph (void)
{
  int translationLength = 0;
  int translatedLength;
  int charactersWritten = 0;
  int pieceStart;
  int k;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <=
	 32 && ud->text_buffer[ud->text_length - 1] != escapeChar)
    ud->text_length--;
  if (ud->text_length == 0)
    return 1;
  ud->text_buffer[ud->text_length] = 0;
  k = 0;
  while (k < ud->text_length)
    {
      if (ud->text_buffer[k] == *litHyphen
	  && ud->text_buffer[k + 1] == 10
	  && ud->text_buffer[k + 2] != escapeChar)
	k += 2;
      if (k > translationLength)
	ud->text_buffer[translationLength] = ud->text_buffer[k];
      k++;
      translationLength++;
    }
  translatedLength = MAX_TRANS_LENGTH;
  if (!lou_backTranslateString (ud->main_braille_table,
				ud->text_buffer, &translationLength,
				&ud->translated_buffer[0],
				&translatedLength,
				(char *) ud->typeform, NULL, 0))
    return 0;
  if (ud->back_text == html)
    {
      if (!insertCharacters ("<p>", 3))
	return 0;
    }
  for (k = 0; k < translatedLength; k++)
    if (ud->translated_buffer[k] == 0)
      ud->translated_buffer[k] = 32;
  while (charactersWritten < translatedLength)
    {
      int lineLength;
      if ((charactersWritten + ud->back_line_length) > translatedLength)
	lineLength = translatedLength - charactersWritten;
      else
	{
	  lineLength = ud->back_line_length;
	  while (lineLength > 0
		 && ud->translated_buffer[charactersWritten +
					  lineLength] != 32)
	    lineLength--;
	  if (lineLength == 0)
	    {
	      lineLength = ud->back_line_length;
	      while ((charactersWritten + lineLength) < translatedLength
		     && ud->translated_buffer[charactersWritten +
					      lineLength] != 32)
		lineLength++;
	    }
	}
      pieceStart = charactersWritten;
      if (ud->back_text == html)
	{
	  for (k = charactersWritten; k < charactersWritten + lineLength; k++)
	    if (ud->translated_buffer[k] == '<'
		|| ud->translated_buffer[k] == '&'
		|| ud->translated_buffer[k] == escapeChar)
	      {
		if (!insertWidechars
		    (&ud->translated_buffer[pieceStart], k - pieceStart))
		  return 0;
		if (ud->translated_buffer[k] == '<')
		  {
		    if (!insertCharacters ("&lt;", 4))
		      return 0;
		  }
		else if (ud->translated_buffer[k] == '&')
		  {
		    if (!insertCharacters ("&amp;", 5))
		      return 0;
		  }
		else
		  {
		    int kk;
		    for (kk = k;
			 kk < translatedLength
			 && ud->translated_buffer[kk] == escapeChar; kk++);
		    kk -= k + 1;
		    if (!insertCharacters (xmlTags[kk], strlen (xmlTags[kk])))
		      return 0;
		    k += kk;
		  }
		pieceStart = k + 1;
	      }
	  if (!insertWidechars (&ud->translated_buffer[pieceStart], k -
				pieceStart))
	    return 0;
	}
      else
	{
	  if (!insertWidechars
	      (&ud->translated_buffer[charactersWritten], lineLength))
	    return 0;
	}
      charactersWritten += lineLength;
      if (ud->translated_buffer[charactersWritten] == 32)
	charactersWritten++;
      if (charactersWritten < translatedLength)
	{
	  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	    return 0;
	}
    }
  if (ud->back_text == html)
    {
      if (!insertCharacters ("</p>", 4))
	return 0;
    }
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  write_outbuf ();
  ud->text_length = 0;
  return 1;
}

static int
handlePrintPageNumber (void)
{
  int k, kk;
  int numberStart = 0;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <= 32)
    ud->text_length--;
  for (k = ud->text_length - 1; k > 0; k--)
    {
      if (ud->text_buffer[k] == 10)
	break;
      if (ud->text_buffer[k] != '-')
	numberStart = k;
    }
  if ((numberStart - k) < 12)
    return 1;
  k++;
  if (ud->back_text == html)
    {
      widechar holdNumber[20];
      int kkk = 0;
      for (kk = numberStart; kk < ud->text_length; kk++)
	holdNumber[kkk++] = ud->text_buffer[kk];
      ud->text_length = k;
      if (!insertEscapeChars (1))
	return 0;
      for (kk = 0; kk < kkk; kk++)
	ud->text_buffer[ud->text_length++] = holdNumber[kk];
      if (!insertEscapeChars (2))
	return 0;
    }
  else
    {
      for (kk = numberStart; kk < ud->text_length; kk++)
	ud->text_buffer[k++] = ud->text_buffer[kk];
      ud->text_length = k;
    }
  return 1;
}

static int
discardPageNumber (void)
{
  int lastBlank = 0;
  int k;
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <= 32)
    ud->text_length--;
  for (k = ud->text_length - 1; k > 0 && ud->text_buffer[k] != 10; k--)
    {
      if (!lastBlank && ud->text_buffer[k] == 32)
	lastBlank = k;
      if (lastBlank && ud->text_buffer[k] > 32)
	break;
    }
  if (k > 0 && ud->text_buffer[k] != 10 && (lastBlank - k) > 2)
    ud->text_length = k + 2;
  return 1;
}

int
back_translate_file (void)
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int printPage = 0;
  int newPage = 0;
  char *htmlStart = "<html><head><title>No Title</title></head><body>";
  char *htmlEnd = "</body></html>";
  if (!start_document ())
    return 0;
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlStart, strlen (htmlStart)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      ud->output_encoding = utf8;
    }
  else
    ud->output_encoding = ascii8;
  while ((ch = fgetc (ud->inFile)) != EOF)
    {
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == escapeChar)
	ch = 32;
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (ch == 10 && printPage)
	{
	  handlePrintPageNumber ();
	  printPage = 0;
	}
      if (ch == 10 && newPage)
	{
	  discardPageNumber ();
	  newPage = 0;
	}
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
	  makeParagraph ();
	  leadingBlanks = 0;
	}
      if (!printPage && ppch == 10 && pch == '-' && ch == '-')
	printPage = 1;
      if (!newPage && pch == 10 && ch == ud->pageEnd[0])
	{
	  discardPageNumber ();
	  newPage = 1;
	  continue;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->text_length >= MAX_LENGTH)
	makeParagraph ();
      ud->text_buffer[ud->text_length++] = ch;
    }
  makeParagraph ();
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlEnd, strlen (htmlEnd)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      write_outbuf ();
      ud->output_encoding = ascii8;
    }
  return 1;
}

int
back_translate_braille_string (void)
{
  int charsProcessed = 0;
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int printPage = 0;
  int newPage = 0;
  char *htmlStart = "<html><head><title>No Title</title></head><body>";
  char *htmlEnd = "</body></html>";
  if (!start_document ())
    return 0;
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlStart, strlen (htmlStart)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      ud->output_encoding = utf8;
    }
  else
    ud->output_encoding = ascii8;
  while (charsProcessed < ud->inlen)
    {
      ch = ud->inbuf[charsProcessed++];
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == escapeChar)
	ch = 32;
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (ch == 10 && printPage)
	{
	  handlePrintPageNumber ();
	  printPage = 0;
	}
      if (ch == 10 && newPage)
	{
	  discardPageNumber ();
	  newPage = 0;
	}
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
	  makeParagraph ();
	  leadingBlanks = 0;
	}
      if (!printPage && ppch == 10 && pch == '-' && ch == '-')
	printPage = 1;
      if (!newPage && pch == 10 && ch == ud->pageEnd[0])
	{
	  discardPageNumber ();
	  newPage = 1;
	  continue;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->text_length >= MAX_LENGTH)
	makeParagraph ();
      ud->text_buffer[ud->text_length++] = ch;
    }
  makeParagraph ();
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlEnd, strlen (htmlEnd)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      write_outbuf ();
      ud->output_encoding = ascii8;
    }
  return 1;
}

static int
makeLinkOrTarget (xmlNode * node, int which)
{
  StyleType *saveStyle;
  int saveFirst;
  int saveOutlen;
  xmlNode *child;
  int branchCount = 0;
  xmlChar *URL = get_attr_value (node);
  if (which == 0)
    insertCharacters ("<a href=\"", 9);
  else
    insertCharacters ("<a name=\"", 9);
  insertCharacters ((char *) URL, strlen ((char *) URL));
  insertCharacters ("\">", 2);
  saveOutlen = ud->outlen_so_far;
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  insert_code (node, branchCount);
	  branchCount++;
	  transcribe_paragraph (child, 1);
	  break;
	case XML_TEXT_NODE:
	  insert_text (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  insert_translation (ud->main_braille_table);
  saveStyle = style;
  saveFirst = styleSpec->status;
  styleSpec->status = startBody;
  style = lookup_style ("para");
  editTrans ();
  doLeftJustify ();
  style = saveStyle;
  styleSpec->status = saveFirst;
  if (ud->outlen_so_far > saveOutlen)
    ud->outlen_so_far -= strlen (ud->lineEnd);
  if (!insertCharacters ("</a>", 4))
    return 0;
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  write_outbuf ();
  return 1;
}

int
insert_linkOrTarget (xmlNode * node, int which)
{
  fineFormat ();
  makeLinkOrTarget (node, which);
  return 1;
}

int
doBoxline (xmlNode * node)
{
  widechar boxChar;
  widechar boxLine[MAXNAMELEN];
  int k;
  int start = ud->text_length;
  int availableCells;
  insert_code (node, 0);
  if (!(ud->text_length - start))
    return 0;
  boxChar = ud->text_buffer[start];
  ud->text_length = start;
  cellsOnLine = 0;
  availableCells = startLine ();
  while (availableCells != ud->cells_per_line)
    {
      finishLine ();
      availableCells = startLine ();
    }
  for (k = 0; k < availableCells; k++)
    boxLine[k] = boxChar;
  if (!insertWidechars (boxLine, availableCells))
    return 0;
  cellsOnLine = ud->cells_per_line;
  finishLine ();
  return 1;
}

int
do_boxline (xmlNode * node)
{
  fineFormat ();
  return doBoxline (node);
}

int
do_newpage (void)
{
  fineFormat ();
  if (ud->lines_on_page > 0)
    fillPage ();
  return 1;
}

int
do_blankline (void)
{
  fineFormat ();
  makeBlankLines (1, 2);
  return 1;
}

int
do_softreturn (void)
{
  fineFormat ();
  return 1;
}

int
do_righthandpage (void)
{
  do_newpage ();
  if (ud->braille_pages && ud->interpoint && !(ud->braille_page_number & 1))
    fillPage ();
  return 1;
}

void
do_linespacing (xmlNode * node)
{
  widechar spacing;
  int savedTextLength = ud->text_length;
  insert_code (node, 0);
  if (ud->text_length == savedTextLength)
    spacing = '0';
  else
    spacing = ud->text_buffer[savedTextLength];
  ud->text_length = savedTextLength;
  if (spacing < '0' || spacing > '3')
    spacing = '0';
  ud->line_spacing = spacing - '0';
}

int
start_style (StyleType * curStyle, xmlNode * node)
{
  if (curStyle == NULL)
    curStyle = lookup_style ("para");
  if (prevStyle == NULL)
    prevStyle = lookup_style ("para");
  if ((ud->text_length > 0 || ud->translated_length > 0) &&
      ud->style_top >= 0)
    {
      /*Continue last style */
      insert_translation (ud->main_braille_table);
      styleSpec = &ud->style_stack[ud->style_top];
      style = styleSpec->style;
      ud->brl_page_num_format = styleSpec->curBrlNumFormat;
      styleBody ();
    }
  if (ud->style_top < (STACKSIZE - 2))
    ud->style_top++;
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style = curStyle;
  styleSpec->status = beforeBody;
  styleSpec->node = node;
  if (style->brlNumFormat != normal)
    ud->brl_page_num_format = style->brlNumFormat;
  styleSpec->curBrlNumFormat = ud->brl_page_num_format;
  startStyle ();
  styleSpec->status = startBody;
  return 1;
}

int
end_style ()
{
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style;
  ud->brl_page_num_format = styleSpec->curBrlNumFormat;
  insert_translation (ud->main_braille_table);
  styleBody ();
  if (!ud->after_contents)
    finishStyle ();
  memcpy (&prevStyleSpec, styleSpec, sizeof (prevStyleSpec));
  prevStyle = prevStyleSpec.style;
  ud->style_top--;
  if (ud->style_top < 0)
    ud->style_top = 0;
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style;
  ud->brl_page_num_format = styleSpec->curBrlNumFormat;
  return 1;
}

/* Routines for Unified Tactile Ducument Markup Language */

#define SPACE B16
/* Dot patterns must include B16 and be enclosed in parentheses.*/
#define NBSP (B16 | B10)
#define CR (B16 | B11)
#define HYPHEN (B16 | B3 | B6)
#define ESCAPE (B16 | B11 | B1)
#define CDOTS (B16 | B1 | B4)
#define EDOTS (B16 | B1 |  B5)
#define RDOTS (B16 | B1 | B2 | B3 | B5)

static int *indices;
static int *backIndices;
static widechar *backBuf;
static int backLength;
static xmlNode *brlNode;
static xmlNode *firstBrlNode;
static xmlNode *prevBrlNode;
static xmlNode *documentNode = NULL;
static xmlNode *brlOnlyNode;
static xmlNode *newlineNode;
static xmlChar *brlContent;
static int maxContent;
static char *utilStringBuf;
static int vertLinePos;
static int maxVertLinePos;
static int lineWidth;
static int numWide;
static int cellsToWrite;
static widechar spaces[3 * MAXNUMLEN];

static int
utd_start ()
{
  int k;
  brlContent = (xmlChar *) ud->outbuf;
  maxContent = ud->outlen * CHARSIZE;
  utilStringBuf = (char *) ud->text_buffer;
  brlNode = firstBrlNode = prevBrlNode = NULL;
  maxVertLinePos = ud->top_margin + NORMALLINE * ud->lines_per_page;
  ud->louis_mode = dotsIO;
  indices = malloc (MAX_TRANS_LENGTH * sizeof (int));
  backIndices = NULL;
  backBuf = NULL;
  backLength = 0;
  if (spaces[0] != SPACE)
    for (k = 0; k < 3 * MAXNUMLEN; k++)
      spaces[k] = SPACE;
  return 1;
}

static xmlParserCtxt *ctxt;

static xmlNode *
makeDaisyDoc (void)
{
  xmlDoc *doc;
  xmlNode *newNode;
  xmlNode *rootNode;
  xmlNode *bodyNode;
  xmlNode *bookNode;
  xmlNode *retNode;
  char *starter =
    "<?xml version='1.0' encoding='UTF-8' standalone='yes'?><dtbook/>";
  static int initialized = 0;
  if (!initialized)
    {
      initialized = 1;
      LIBXML_TEST_VERSION xmlKeepBlanksDefault (0);
      xmlSubstituteEntitiesDefault (1);
      xmlThrDefIndentTreeOutput (1);
      xmlThrDefKeepBlanksDefaultValue (0);
      xmlThrDefLineNumbersDefaultValue (1);
    }
  ctxt = xmlNewParserCtxt ();
  xmlSetGenericErrorFunc (ctxt, libxml_errors);
  doc = xmlParseMemory (starter, strlen (starter));
  rootNode = xmlDocGetRootElement (doc);
  newNode = xmlNewNode (NULL, (xmlChar *) "head");
  ud->head_node = xmlAddChild (rootNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "book");
  bookNode = xmlAddChild (rootNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "frrontmatter");
  xmlAddChild (bookNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "bodymatter");
  bodyNode = xmlAddChild (bookNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "level");
  retNode = xmlAddChild (bodyNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "h1");
  xmlAddChild (retNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "rearmater");
  xmlAddChild (bookNode, newNode);
  ud->doc = doc;
  return retNode;
}

static int
processDaisyDoc (void)
{
  /* This function complements makeDaisyDoc. */
  xmlNode *rootElement = NULL;
  int haveSemanticFile;
  if (ud->doc == NULL)
    {
      lou_logPrint ("Document could not be processed");
      return 0;
    }
  rootElement = xmlDocGetRootElement (ud->doc);
  if (rootElement == NULL)
    {
      lou_logPrint ("Document is empty");
      return 0;
    }
  haveSemanticFile = compile_semantic_table (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  transcribe_document (rootElement);
  xmlFreeDoc (ud->doc);
  xmlCleanupParser ();
  initGenericErrorDefaultFunc (NULL);
  xmlFreeParserCtxt (ctxt);
  return 1;
}

static int
handleChar (int ch, unsigned char *buf, int *posx)
{
  int pos = *posx;
  if (ch > 127 && ud->input_encoding == ascii8)
    {
      buf[pos++] = 0xc1;
      buf[pos++] = ch;
    }
  else if (ch == '<' || ch == '&')
    {
      buf[pos++] = '&';
      if (ch == '<')
	{
	  buf[pos++] = 'l';
	  buf[pos++] = 't';
	}
      else
	{
	  buf[pos++] = 'a';
	  buf[pos++] = 'm';
	  buf[pos++] = 'p';
	}
      buf[pos++] = ';';
    }
  else
    buf[pos++] = ch;
  *posx = pos;
  return 1;
}

static int
utd_transcribe_text_string (void)
{
  xmlNode *addPara = makeDaisyDoc ();
  xmlNode *newPara;
  xmlNode *textNode;
  int charsProcessed = 0;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      while (charsProcessed < ud->inlen)
	{
	  ch = ud->inbuf[charsProcessed++];
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  pch = ch;
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  handleChar (ch, paragraphBuffer, &charsInParagraph);
	}
      ch = ud->inbuf[charsProcessed++];
      if (charsInParagraph == 0)
	break;
      paragraphBuffer[charsInParagraph] = 0;
      newPara = xmlNewNode (NULL, (xmlChar *) "p");
      textNode = xmlNewText (paragraphBuffer);
      xmlAddChild (newPara, textNode);
      xmlAddChild (addPara, newPara);
      if (ch == 10)
	do_blankline ();
      charsInParagraph = 0;
      pch = 0;
      handleChar (ch, paragraphBuffer, &charsInParagraph);
    }
  processDaisyDoc ();
  ud->input_encoding = utf8;
  return 1;
}

static int
utd_transcribe_text_file (void)
{
  xmlNode *addPara = makeDaisyDoc ();
  xmlNode *newPara;
  xmlNode *textNode;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char *paragraphBuffer = (unsigned char *) ud->translated_buffer;
  ud->input_encoding = ud->input_text_encoding;
  while (1)
    {
      while ((ch = fgetc (ud->inFile)) != EOF)
	{
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  pch = ch;
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  handleChar (ch, paragraphBuffer, &charsInParagraph);
	}
      if (charsInParagraph == 0)
	break;
      ch = fgetc (ud->inFile);
      if (ch != EOF)
	{
	  paragraphBuffer[charsInParagraph - 1] = 0;
	  newPara = xmlNewNode (NULL, (xmlChar *) "p");
	  if (ch == 10)
	    xmlNewProp (newPara, (xmlChar *) "before", (xmlChar *) "1");
	  textNode = xmlNewText (paragraphBuffer);
	  xmlAddChild (newPara, textNode);
	  xmlAddChild (addPara, newPara);
	}
      charsInParagraph = 0;
      pch = 0;
      if (ch != EOF)
	handleChar (ch, paragraphBuffer, &charsInParagraph);
    }
  processDaisyDoc ();
  ud->input_encoding = utf8;
  return 1;
}

int
link_brl_node (xmlNode * node)
{
  if (node == NULL)
    return 0;
  if (firstBrlNode == NULL)
    {
      firstBrlNode = node;
      prevBrlNode = firstBrlNode;
    }
  else
    {
      prevBrlNode->_private = node;
      prevBrlNode = node;
    }
  brlNode = node;
  return 1;
}

static int
backTranslateBlock (xmlNode * node)
{
  xmlNode *thisBrl;
  xmlNode *child;
  xmlNode *backText;
  int translationLength;
  int translatedLength;
  int goodTrans;
  int pos;
  int k, kk;
  if (node == NULL)
    return 1;
  thisBrl = node->children;
  ud->text_length = 0;
  child = thisBrl->children;
  while (child)
    {
      if (child->type == XML_TEXT_NODE)
	insert_utf8 (child->content);
      child = child->next;
    }
  lou_dotsToChar (ud->main_braille_table, ud->text_buffer,
		  ud->text_buffer, ud->text_length);
  if (ud->text_length > backLength)
    {
      backLength = ud->text_length;
      if (backBuf != NULL)
	free (backBuf);
      backBuf = malloc ((3 * backLength + 4) * CHARSIZE);
      if (backIndices != NULL)
	free (backIndices);
      backIndices = malloc ((3 * backLength + 4) * sizeof (int));
    }
  translationLength = ud->text_length;
  translatedLength = 3 * backLength;
  goodTrans = lou_backTranslate (ud->main_braille_table, ud->text_buffer,
				 &translationLength,
				 backBuf, &translatedLength, NULL, NULL,
				 backIndices, NULL, NULL, 0);
  if (!goodTrans)
    {
      translatedLength = translationLength;
      memcpy (backBuf, ud->text_buffer, translatedLength * CHARSIZE);
    }
  pos = 0;
  for (k = 0; k < translatedLength; k++)
    {
      widechar ch = backBuf[k];
      if (ch < 127)
	{
	  if (ch == '<' || ch == '&')
	    {
	      utilStringBuf[pos++] = '&';
	      if (ch == '<')
		{
		  utilStringBuf[pos++] = 'l';
		  utilStringBuf[pos++] = 't';
		}
	      else
		{
		  utilStringBuf[pos++] = 'a';
		  utilStringBuf[pos++] = 'm';
		  utilStringBuf[pos++] = 'p';
		}
	      utilStringBuf[pos++] = ';';
	    }
	  else
	    utilStringBuf[k] = ch;
	}
      else
	{
	  unsigned char *utf8str = utfwcto8 (ch);
	  for (kk = 0; utf8str[kk]; kk++)
	    utilStringBuf[pos++] = utf8str[kk];
	}
    }
  utilStringBuf[pos] = 0;
  backText = xmlNewText ((xmlChar *) utilStringBuf);
  xmlAddPrevSibling (thisBrl, backText);
  if (!goodTrans)
    return 1;
  k = kk = 0;
  while (k < translatedLength)
    {
      char posx[MAXNUMLEN];
      int posxLen = sprintf (posx, "%d,", backIndices[k]);
      strcpy (&utilStringBuf[kk], posx);
      kk += posxLen;
    }
  utilStringBuf[--kk] = 0;
  xmlNewProp (thisBrl, (xmlChar *) "index", (xmlChar *) utilStringBuf);
  return 1;
}

static int
makeTextNode (xmlNode * node, const widechar * content, int length, int kind)
{
  xmlNode *textNode;
  int k;
  int kk = 0;
  if (length <= 0)
    return 1;
  if ((3 * length) >= maxContent)
    length = maxContent / 3 - 4;
  if (kind)
    lou_charToDots (ud->main_braille_table, content, ud->text_buffer, length);
  else
    memcpy (ud->text_buffer, content, length * CHARSIZE);
  for (k = 0; k < length; k++)
    {
      xmlChar *utf8Char = utfwcto8 ((ud->text_buffer[k] & 0xff) | 0x2800);
      memcpy (&brlContent[kk], utf8Char, 3);
      kk += 3;
    }
  brlContent[kk] = 0;
  textNode = xmlNewText (brlContent);
  xmlAddChild (node, textNode);
  return 1;
}

static int
utd_insertCharacters (xmlNode * node, char *text, int length)
{
  widechar charBuf[MAXNAMELEN];
  int k;
  if (length <= 0)
    return 1;
  if (length >= MAXNAMELEN)
    length = MAXNAMELEN - 4;
  for (k = 0; k < length; k++)
    charBuf[k] = text[k];
  makeTextNode (node, charBuf, length, 1);
  return 1;
}

static int
makeBrlOnlyNode ()
{
  xmlNode *newNode;
  newNode = xmlNewNode (NULL, (xmlChar *) "span");
  xmlNewProp (newNode, (xmlChar *) "class", (xmlChar *) "brlonly");
  brlOnlyNode = xmlAddChild (brlNode, newNode);
  return 1;
}

static int
shortBrlOnly (const widechar * content, int length, int kind)
{
  makeBrlOnlyNode ();
  makeTextNode (brlOnlyNode, content, length, kind);
  return 1;
}

static int
checkTextFragment (widechar * text, int length)
{
  int k;
  widechar dots;
  for (k = 0; k < length; k++)
    {
      dots = text[k];
      if ((dots & (B7 | B8)))
	lineWidth = WIDELINE;
      if (dots == NBSP)
	text[k] = SPACE;
    }
  return 1;
}

static int
insertTextFragment (widechar * content, int length)
{
  if (length <= 0)
    return 1;
  checkTextFragment (content, length);
  makeTextNode (brlNode, content, length, 0);
  return 1;
}

static int
assignIndices (void)
{
  int prevSegment = 0;
  int curPos = 0;
  brlNode = firstBrlNode;
  while (curPos < translatedLength)
    {
      if (translatedBuffer[curPos] == ENDSEGMENT)
	{
	  int indexPos = prevSegment;
	  int firstIndex = indices[indexPos];
	  int kk;
	  while (translatedBuffer[indexPos] != ENDSEGMENT)
	    {
	      int k;
	      kk = 0;
	      char pos[MAXNUMLEN];
	      int posLen =
		sprintf (pos, "%d,", indices[indexPos] - firstIndex);
	      strcpy (&utilStringBuf[kk], pos);
	      kk += posLen;
	      indexPos++;
	    }
	  utilStringBuf[--kk] = 0;
	  xmlNewProp (brlNode, (xmlChar *) "index", (xmlChar *)
		      utilStringBuf);
	  brlNode = brlNode->_private;
	  curPos += indexPos;
	}
      curPos++;
    }
  return 1;
}

static int utd_fillPage (void);
static int makeNewline (xmlNode * parent, int start);

static int
makePageSeparator (xmlChar * printPageNumber, int length)
{
  int k;
  int kk;
  widechar translatedBuffer[MAXNUMLEN];
  int translatedLength = MAXNUMLEN;
  widechar separatorLine[128];
  char setup[MAXNUMLEN];
  if (!ud->print_pages || !*printPageNumber)
    return 1;
  strcpy (setup, " ");
  if (!(printPageNumber[0] >= '0' && printPageNumber[0] <= '9'))
    strcat (setup, LETSIGN);
  strcat (setup, printPageNumber);
  length = strlen (setup);
  translationLength = MAXNUMLEN;
  utf8ToWc (setup, &length, separatorLine, &translationLength);
  if (!lou_translateString (ud->main_braille_table,
			    separatorLine,
			    &translationLength, ud->print_page_number,
			    &translatedLength, NULL, NULL, 0))
    return 0;
  ud->print_page_number[translatedLength] = 0;
  if (ud->braille_pages && ud->lines_on_page == 0)
    return 1;
  if (ud->format_for == utd)
    {
      lou_charToDots (ud->main_braille_table,
		      ud->print_page_number, translatedBuffer,
		      translatedLength);
      translatedBuffer[0] = HYPHEN;
      for (k = 0; k < (ud->cells_per_line - translatedLength); k++)
	separatorLine[k] = HYPHEN;
      kk = 0;
      for (; k < ud->cells_per_line; k++)
	separatorLine[k] = translatedBuffer[kk++];
      separatorLine[k] = 0;
      ud->print_page_number[0] = 'a';
      if (ud->braille_pages && ud->lines_on_page >= (ud->lines_per_page - 2))
	utd_fillPage ();
      makeBrlOnlyNode ();
      makeNewline (brlOnlyNode, 0);
      makeTextNode (brlOnlyNode, separatorLine, ud->cells_per_line, 0);
    }
  else
    {
      memcpy (translatedBuffer, ud->print_page_number, translatedLength
	      * CHARSIZE);
      translatedBuffer[0] = '-';
      for (k = 0; k < (ud->cells_per_line - translatedLength); k++)
	separatorLine[k] = '-';
      kk = 0;
      for (; k < ud->cells_per_line; k++)
	separatorLine[k] = translatedBuffer[kk++];
      if (ud->braille_pages && ud->lines_on_page >= (ud->lines_per_page - 2))
	fillPage ();
      if (!insertWidechars (separatorLine, ud->cells_per_line))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      ud->lines_on_page++;
      write_outbuf ();
    }
  return 1;
}

static int
makeNewpage (xmlNode * parent)
{
  char number[MAXNUMLEN];
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newpage");
  sprintf (number, "%d", ud->braille_page_number);
  xmlNewProp (newNode, (xmlChar *) "number", (xmlChar *) number);
  xmlAddChild (parent, newNode);
  return 1;
}

static int
makeNewline (xmlNode * parent, int start)
{
  char position[MAXNUMLEN];
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newline");
  sprintf (position, "%d,%d", (CELLWIDTH * start +
			       ud->left_margin), vertLinePos);
  xmlNewProp (newNode, (xmlChar *) "xy", (xmlChar *) position);
  xmlAddChild (parent, newNode);
  vertLinePos += lineWidth;
  if (lineWidth == WIDELINE)
    {
      numWide++;
      if (numWide >= (WIDELINE / 2))
	{
	  ud->lines_on_page++;
	  numWide = 0;
	}
    }
  lineWidth = NORMALLINE;
  return 1;
}

static int
utd_insert_translation (const char *table)
{
  int translationLength;
  int translatedLength;
  int k;
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  k = lou_translate (table,
		     ud->text_buffer,
		     &translationLength,
		     &ud->
		     translated_buffer[ud->translated_length],
		     &translatedLength,
		     (char *) ud->typeform, NULL, NULL,
		     &indices[ud->translated_length], NULL, dotsIO);
  memset (ud->typeform, 0, sizeof (ud->typeform));
  ud->text_length = 0;
  if (!k)
    {
      table = NULL;
      return 0;
    }
  if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
    ud->translated_length += translatedLength;
  else
    ud->translated_length = MAX_TRANS_LENGTH;
  return 1;
}

static void
utd_insert_text (xmlNode * node, int length)
{
  int wcLength;
  xmlNode *newNode;
  int k;
  newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  link_brl_node (xmlAddNextSibling (node, newNode));
  switch (ud->stack[ud->top])
    {
    case notranslate:
      utd_insert_translation (ud->main_braille_table);
      insert_utf8 (node->content);
      if ((ud->translated_length + ud->text_length) > MAX_TRANS_LENGTH)
	ud->text_length = MAX_TRANS_LENGTH - ud->translated_length;
      lou_charToDots (ud->main_braille_table, ud->text_buffer,
		      &ud->translated_buffer[ud->translated_length],
		      ud->text_length);
      for (k = 0; k < ud->text_length; k++)
	indices[ud->translated_length + k] = k;
      ud->translated_length += ud->text_length;
      ud->translated_buffer[ud->translated_length++] = ENDSEGMENT;
      ud->text_length = 0;
      return;
    case pagenum:
      if (!ud->print_pages)
	return;
      fineFormat ();
      makePageSeparator (node->content, length);
      return;
    default:
      break;
    }
  wcLength = insert_utf8 (node->content);
  ud->text_buffer[ud->text_length++] = ENDSEGMENT;
  switch (ud->stack[ud->top])
    {
    case italicx:
      if (!(ud->emphasis & italic))
	break;
      memset (&ud->typeform[ud->old_text_length], italic, wcLength);
      break;
    case underlinex:
      if (!(ud->emphasis & underline))
	break;
      memset (&ud->typeform[ud->old_text_length], underline, wcLength);
      break;
    case boldx:
      if (!(ud->emphasis & bold))
	break;
      memset (&ud->typeform[ud->old_text_length], bold, wcLength);
      break;
    case compbrl:
      if (!(ud->emphasis & computer_braille))
	break;
      memset (&ud->typeform[ud->old_text_length], computer_braille, wcLength);
      break;
    default:
      break;
    }
  return;
}

static int utd_finishLine (int leadingBlanks, int lengtgh);

static int
setNewlineNode (void)
{
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newline");
  newlineNode = xmlAddChild (brlNode, newNode);
  lineWidth = NORMALLINE;
  return 1;
}

static int
setNewlineProp (int horizLinePos)
{
  char position[MAXNUMLEN];
  sprintf (position, "%d,%d", (CELLWIDTH * horizLinePos +
			       ud->left_margin), vertLinePos);
  xmlNewProp (newlineNode, (xmlChar *) "xy", (xmlChar *) position);
  vertLinePos += lineWidth;
  if (lineWidth == WIDELINE)
    {
      numWide++;
      if (numWide >= (WIDELINE / 2))
	{
	  ud->lines_on_page++;
	  numWide = 0;
	}
    }
  return 1;
}

static int
utd_startLine ()
{
  int availableCells = 0;
  while (availableCells == 0)
    {
      setNewlineNode ();
      if (!ud->braille_pages)
	return ud->cells_per_line;
      ud->lines_on_page++;
      getPageNumber ();
      if (ud->lines_on_page == 1)
	{
	  if (ud->running_head_length > 0 || (style->skip_number_lines &&
					      pageNumberLength > 0))
	    {
	      utd_finishLine (0, 0);
	      setNewlineNode ();
	      continue;
	    }
	  availableCells = ud->cells_per_line - pageNumberLength;
	}
      else if (ud->lines_on_page == ud->lines_per_page)
	{
	  if (ud->footer_length > 0 ||
	      (style->skip_number_lines && pageNumberLength > 0))
	    {
	      utd_finishLine (0, 0);
	      setNewlineNode ();
	      continue;
	    }
	  availableCells = ud->cells_per_line - pageNumberLength;
	}
      else
	availableCells = ud->cells_per_line;
    }
  return availableCells;
}

static int
utd_finishLine (int leadingBlanks, int length)
{
  int cellsOnLine = 0;
  int cellsToWrite = 0;
  int k;
  int leaveBlank;
  int horizLinePos = leadingBlanks;
  cellsOnLine = leadingBlanks + length;
  for (leaveBlank = -1; leaveBlank < ud->line_spacing; leaveBlank++)
    {
      if (leaveBlank != -1)
	{
	  utd_startLine ();
	  setNewlineProp (0);
	}
      if (ud->braille_pages)
	{
	  if (cellsOnLine > 0 && pageNumberLength > 0)
	    {
	      cellsToWrite =
		ud->cells_per_line - pageNumberLength - cellsOnLine;
	      if (!insertTextFragment (spaces, cellsToWrite))
		return 0;
	      if (!shortBrlOnly (pageNumberString, pageNumberLength, 1))
		return 0;
	    }
	  else if (ud->lines_on_page == 1)
	    {
	      if (ud->running_head_length > 0)
		{
		  cellsToWrite =
		    minimum (ud->running_head_length,
			     ud->cells_per_line - pageNumberLength);
		  if (!shortBrlOnly (ud->running_head, cellsToWrite, 1))
		    return 0;
		  if (pageNumberLength)
		    {
		      cellsToWrite =
			ud->cells_per_line - pageNumberLength - cellsOnLine;
		      if (!makeTextNode (brlNode, spaces, cellsToWrite, 0))
			return 0;
		      if (!shortBrlOnly
			  (pageNumberString, pageNumberLength, 1))
			return 0;
		    }
		}
	      else
		{
		  if (pageNumberLength)
		    {
		      cellsToWrite = ud->cells_per_line - pageNumberLength;
		      if (!makeTextNode (brlNode, spaces, cellsToWrite, 0))
			return 0;
		      if (!shortBrlOnly
			  (pageNumberString, pageNumberLength, 1))
			return 0;
		    }
		}
	    }
	  else if (ud->lines_on_page == ud->lines_per_page)
	    {
	      if (ud->footer_length > 0)
		{
		  cellsToWrite =
		    minimum (ud->footer_length,
			     ud->cells_per_line - pageNumberLength);
		  if (!shortBrlOnly (ud->footer, cellsToWrite, 1))
		    return 0;
		  if (pageNumberLength)
		    {
		      cellsToWrite =
			ud->cells_per_line - pageNumberLength - cellsOnLine;
		      if (!makeTextNode (brlNode, spaces, cellsToWrite, 0))
			return 0;
		      if (!shortBrlOnly
			  (pageNumberString, pageNumberLength, 1))
			return 0;
		    }
		}
	      else
		{
		  if (pageNumberLength)
		    {
		      horizLinePos = ud->cells_per_line - pageNumberLength;
		      if (!shortBrlOnly
			  (pageNumberString, pageNumberLength, 1))
			return 0;
		    }
		}
	    }
	}
      setNewlineProp (horizLinePos);
      if (ud->braille_pages && ud->lines_on_page == ud->lines_per_page)
	{
	  ud->lines_on_page = 0;
	  ud->braille_page_number++;
	  numWide = 0;
	  lineWidth = NORMALLINE;
	  vertLinePos = ud->top_margin;
	  makeNewpage (brlNode);
	}
    }
  return 1;
}

static int
hasIndex (xmlNode * node)
{
  xmlAttr *attributes = node->properties;
  if (attributes == NULL)
    return 0;
  while (attributes)
    {
      if (strcmp ((char *) attributes->name, "index") == 0)
	return 1;
      attributes = attributes->next;
    }
  return 0;
}

static int
utd_doOrdinaryText (void)
{
  int availableCells;
  int origAvailableCells;
  int cellsOnLine = 0;
  int leadingBlanks = 0;
  widechar dots;
  int lastSpace;
  int charactersWritten = 0;
  int newLineNeeded = 1;
  brlNode = firstBrlNode;
  while (brlNode)
    {
      if (!hasIndex (brlNode))
	{
	  brlNode = brlNode->_private;
	  if (brlNode == NULL)
	    break;
	}
      do
	{
	  if (newLineNeeded)
	    {
	      newLineNeeded = 0;
	      if (translatedBuffer[charactersWritten] == SPACE)
		charactersWritten++;
	      origAvailableCells = availableCells = utd_startLine ();
	      if (style->format == leftJustified)
		{
		  if (styleSpec->status == startBody)
		    leadingBlanks =
		      style->left_margin + style->first_line_indent;
		  else
		    leadingBlanks = style->left_margin;
		  availableCells -= leadingBlanks;
		}
	      styleSpec->status = resumeBody;
	    }
	  lastSpace = 0;
	  for (cellsToWrite = 0; cellsToWrite < availableCells && (dots
								   =
								   translatedBuffer
								   [charactersWritten
								    +
								    cellsToWrite])
	       != ENDSEGMENT; cellsToWrite++)
	    if (dots == SPACE)
	      lastSpace = cellsToWrite;
	  if (cellsToWrite == availableCells)
	    newLineNeeded = 1;
	  if (dots != ENDSEGMENT && lastSpace != 0)
	    cellsToWrite = lastSpace + 1;
	  cellsOnLine += cellsToWrite;
	  availableCells -= cellsToWrite;
	  insertTextFragment (&translatedBuffer[charactersWritten],
			      cellsToWrite);
	  charactersWritten += cellsToWrite;
	  if (newLineNeeded || (charactersWritten + 1) >= translatedLength)
	    {
	      switch (style->format)
		{
		default:
		case leftJustified:
		  break;
		case centered:
		  leadingBlanks = (origAvailableCells - cellsOnLine) / 2;
		  break;
		case rightJustified:
		  leadingBlanks = origAvailableCells - cellsOnLine;
		  break;
		}
	      utd_finishLine (leadingBlanks, cellsToWrite);
	      cellsOnLine = 0;
	      newLineNeeded = 1;
	    }
	}
      while (dots != ENDSEGMENT);
      charactersWritten++;
      prevBrlNode = brlNode;
      brlNode = brlNode->_private;
      prevBrlNode->_private = NULL;
    }
  brlNode = prevBrlNode;	/*for utd_finishStyle */
  return 1;
}

static int
utd_makeBlankLines (int number, int beforeAfter)
{
  int availableCells;
  int k;
  if (number == 0)
    return 1;
  if (ud->braille_pages)
    {
      if (beforeAfter == 0 && (ud->lines_on_page == 0 ||
			       prevStyle->lines_after > 0
			       || prevStyle->action == document))
	return 1;
      else if (beforeAfter == 1 && (ud->lines_per_page - ud->lines_on_page -
				    number) < 2)
	return 1;
    }
  else
    {
      if (beforeAfter == 0 && (prevStyle->lines_after || prevStyle->action ==
			       document))
	return 1;
    }
  for (k = 0; k < number; k++)
    {
      availableCells = utd_startLine ();
      if (!utd_finishLine (0, 0))
	return 0;
    }
  return 1;
}

static int
utd_fillPage (void)
{
  if (!ud->braille_pages)
    return 1;
  ud->lines_on_page = ud->lines_per_page - 1;
  vertLinePos = ud->top_margin + NORMALLINE * ud->lines_per_page;
  utd_startLine ();
  utd_finishLine (0, 0);
  return 1;
}

static int
utd_doComputerCode (void)
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (translatedBuffer[charactersWritten] == CR)
    charactersWritten++;
  while (charactersWritten < translatedLength)
    {
      int lineTooLong = 0;
      availableCells = utd_startLine ();
      for (cellsToWrite = 0; cellsToWrite < availableCells; cellsToWrite++)
	if ((charactersWritten + cellsToWrite) >= translatedLength
	    || translatedBuffer[charactersWritten + cellsToWrite] == CR)
	  break;
      if ((charactersWritten + cellsToWrite) > translatedLength)
	cellsToWrite--;
      if (cellsToWrite <= 0 && translatedBuffer[charactersWritten] != CR)
	break;
      if (cellsToWrite == availableCells &&
	  translatedBuffer[charactersWritten + cellsToWrite] != CR)
	{
	  cellsToWrite = availableCells - strlen (compHyphen);
	  lineTooLong = 1;
	}
      if (translatedBuffer[charactersWritten + cellsToWrite] == CR)
	translatedBuffer[charactersWritten + cellsToWrite] = SPACE;
      if (!insertTextFragment
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == SPACE)
	charactersWritten++;
      if (lineTooLong)
	{
	  if (!utd_insertCharacters (brlNode, compHyphen, strlen
				     (compHyphen)))
	    return 0;
	}
      utd_finishLine (0, cellsToWrite);
    }
  return 1;
}

static int
utd_doAlignColumns (void)
{
  int numRows = 0;
  int rowNum = 0;
  int numCols = 0;
  int colNum = 0;
  int colLength = 0;
  int rowLength;
  int colSize[MAXCOLS];
  widechar rowBuf[MAXROWSIZE];
  int bufPos;
  int k;
  unsigned int ch;
  int rowEnd = 0;
  for (bufPos = 0; bufPos < translatedLength; bufPos++)
    if (translatedBuffer[bufPos] == ESCAPE)
      break;
  if (bufPos >= translatedLength)
    {
      utd_doOrdinaryText ();
      return 1;
    }
  for (k = 0; k < MAXCOLS; k++)
    colSize[k] = 0;

  /*Calculate number of columns and column sizes */
  while (bufPos < translatedLength)
    {
      ch = translatedBuffer[bufPos++];
      if (ch == ESCAPE)
	{
	  unsigned int nch = translatedBuffer[bufPos];
	  if (nch == RDOTS)	/*End of row */
	    {
	      numRows++;
	      if (rowEnd == 0)
		rowEnd = colLength;
	      colLength = 0;
	      colNum = 0;
	      bufPos++;
	    }
	  else if (nch == CDOTS)
	    {
	      if (numRows == 0)
		numCols++;
	      if (colSize[colNum] < colLength)
		colSize[colNum] = colLength;
	      colNum++;
	      colLength = 0;
	      bufPos++;
	    }
	  else if (nch == EDOTS)
	    break;
	}
      else
	colLength++;
    }
  colSize[numCols - 1] += rowEnd;
  if (style->format == alignColumnsLeft)
    {
      /*Calculate starting points of columns in output */
      int colStart = 0;
      for (colNum = 0; colNum < numCols; colNum++)
	{
	  k = colSize[colNum];
	  colSize[colNum] = colStart;
	  colStart += k;
	  if (colNum != (numCols - 1))
	    colStart += COLSPACING;
	}
    }
  else
    {
      /*Calculate ending points of columns in output */
      int colEnd = colSize[0];
      for (colNum = 1; colNum < numCols; colNum++)
	{
	  colEnd += colSize[colNum] + COLSPACING;
	  colSize[colNum] = colEnd;
	}
    }

/*Now output the stuff.*/
  if ((ud->lines_per_page - ud->lines_on_page) < numRows)
    utd_fillPage ();
  bufPos = 0;
  for (rowNum = 0; rowNum < numRows; rowNum++)
    {
      int charactersWritten = 0;
      int cellsToWrite = 0;
      int availableCells = 0;
      rowLength = 0;
      if (style->format == alignColumnsLeft)
	{
	  for (colNum = 0; colNum < numCols; colNum++)
	    {
	      while (rowLength < MAXROWSIZE
		     && translatedBuffer[bufPos] != ESCAPE)
		rowBuf[rowLength++] = translatedBuffer[bufPos++];
	      bufPos += 2;
	      if (colNum < (numCols - 1))
		{
		  while (rowLength < MAXROWSIZE && rowLength <
			 colSize[colNum + 1])
		    rowBuf[rowLength++] = ' ';
		}
	      else
		{
		  while (rowLength < MAXROWSIZE
			 && translatedBuffer[bufPos] != ESCAPE)
		    rowBuf[rowLength++] = translatedBuffer[bufPos++];
		  bufPos += 2;	/*actual end of row */
		}
	    }
	}
      else
	{
	  int prevBufPos = bufPos;
	  int prevCol = 0;
	  for (colNum = 0; colNum < numCols; colNum++)
	    {
	      while (translatedBuffer[bufPos] != ESCAPE)
		bufPos++;
	      for (k = bufPos - 1; k >= prevBufPos; k--)
		rowBuf[k + prevCol] = translatedBuffer[k];
	      for (; k >= prevCol; k--)
		rowBuf[k + prevCol] = ' ';
	      prevBufPos = bufPos + 2;
	      prevCol = colSize[colNum];
	      rowLength += colSize[colNum];
	      if (rowLength > MAXROWSIZE)
		break;
	    }
	  while (rowLength < MAXROWSIZE && translatedBuffer[bufPos] != ESCAPE)
	    rowBuf[rowLength++] = translatedBuffer[bufPos++];
	  bufPos += 2;
	}
      while (charactersWritten < rowLength)
	{
	  int rowTooLong = 0;
	  availableCells = utd_startLine ();
	  if ((charactersWritten + availableCells) >= rowLength)
	    cellsToWrite = rowLength - charactersWritten;
	  else
	    {
	      for (cellsToWrite = availableCells; cellsToWrite > 0;
		   cellsToWrite--)
		if (rowBuf[charactersWritten + cellsToWrite] == SPACE)
		  break;
	      if (cellsToWrite == 0)
		{
		  cellsToWrite = availableCells - 1;
		  rowTooLong = 1;
		}
	    }
	  while (rowBuf[charactersWritten + cellsToWrite] == SPACE)
	    cellsToWrite--;
	  if (cellsToWrite == 0)
	    break;
	  if (!insertTextFragment (&rowBuf[charactersWritten], cellsToWrite))
	    return 0;
	  charactersWritten += cellsToWrite;
	  if (rowTooLong)
	    {
	      if (!utd_insertCharacters (brlNode, litHyphen, strlen
					 (litHyphen)))
		return 0;
	    }
	  utd_finishLine (0, cellsToWrite);
	}
    }
  return 1;
}

static int
utd_doListColumns (void)
{
  utd_doOrdinaryText ();
  return 1;
}

static int
utd_doContents (void)
{
  int lastWord;
  int lastWordLength;
  int untilLastWord;
  int numbersStart;
  int numbersLength;
  int leadingBlanks = 0;
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  if (translatedBuffer[translatedLength - 1] == (NBSP))
    {
      /* No page numbers */
      translatedLength--;
      utd_doOrdinaryText ();
      return 1;
    }
  for (k = translatedLength - 1; k > 0 && translatedBuffer[k] != SPACE; k--);
  if (k == 0)
    {
      utd_doOrdinaryText ();

      return 1;
    }
  numbersStart = k + 1;
  numbersLength = translatedLength - numbersStart;
  for (--k; k >= 0 && translatedBuffer[k] > SPACE; k--);
  lastWord = k + 1;
  lastWordLength = numbersStart - lastWord;
  untilLastWord = lastWord - 1;
  while (charactersWritten < untilLastWord)
    {
      int wordTooLong = 0;
      int breakAt = 0;
      availableCells = utd_startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = style->left_margin + style->first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = style->left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      availableCells -= leadingBlanks;
      if ((charactersWritten + availableCells) >= untilLastWord)
	cellsToWrite = untilLastWord - charactersWritten;
      else
	{
	  for (cellsToWrite = availableCells - 2; cellsToWrite > 0;
	       cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == SPACE)
	      break;
	  if (cellsToWrite == 0)
	    {
	      cellsToWrite = availableCells - 1;
	      wordTooLong = 1;
	    }
	  else
	    {
	      if (ud->hyphenate)
		breakAt =
		  hyphenatex (charactersWritten + cellsToWrite,
			      charactersWritten + availableCells);
	      if (breakAt)
		cellsToWrite = breakAt - charactersWritten;
	    }
	}
      if (!insertTextFragment
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == SPACE)
	charactersWritten++;
      if ((breakAt && translatedBuffer[breakAt - 1] != *litHyphen)
	  || wordTooLong)
	{
	  if (!utd_insertCharacters (brlNode, litHyphen, strlen (litHyphen)))
	    return 0;
	}
      if (charactersWritten < untilLastWord)
	utd_finishLine (leadingBlanks, cellsToWrite);
      else
	{
	  availableCells -= cellsToWrite;
	  if (availableCells <= 0)
	    {
	      utd_finishLine (leadingBlanks, cellsToWrite);
	      availableCells = 0;
	    }
	}
    }
  if (availableCells == 0)
    {
      availableCells = utd_startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = style->left_margin + style->first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = style->left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      availableCells -= leadingBlanks;
    }
  if ((lastWordLength + numbersLength + 2) < availableCells)
    {
      insertCharacters (blanks, 1);
      availableCells--;
      if (!insertTextFragment (&translatedBuffer[lastWord], lastWordLength))
	return 0;
      availableCells -= lastWordLength;
      if ((availableCells - numbersLength) < 3)
	utd_insertCharacters (brlNode, blanks, availableCells -
			      numbersLength);
      else
	{
	  insertCharacters (blanks, 1);
	  for (k = availableCells - (numbersLength + 1); k > 0; k--)
	    insertCharacters (&ud->line_fill, 1);
	  insertCharacters (blanks, 1);
	}
      if (!insertTextFragment (&translatedBuffer[numbersStart],
			       numbersLength))
	return 0;
      utd_finishLine (leadingBlanks, cellsToWrite);
    }
  else
    {
      utd_finishLine (leadingBlanks, cellsToWrite);
      availableCells = utd_startLine ();
      leadingBlanks = style->left_margin;
      availableCells -= leadingBlanks;
      if (!insertTextFragment (&translatedBuffer[lastWord], lastWordLength))
	return 0;
      availableCells -= lastWordLength;
      if ((availableCells - numbersLength) < 3)
	utd_insertCharacters (brlNode, blanks, availableCells -
			      numbersLength);
      else
	{
	  insertCharacters (blanks, 1);
	  for (k = availableCells - (numbersLength + 1); k > 0; k--)
	    insertCharacters (&ud->line_fill, 1);
	  insertCharacters (blanks, 1);
	}
      if (!insertTextFragment (&translatedBuffer[numbersStart],
			       numbersLength))
	return 0;
      utd_finishLine (leadingBlanks, numbersLength);
    }
  return 1;
}

static int
utd_startStyle (void)
{
  firstBrlNode = NULL;
  if ((style->action == document || style->lines_before ||
       style->skip_number_lines ||
       style->newpage_before || style->righthand_page) && styleSpec->node
      != NULL)
    {
      xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "brl");
      link_brl_node (xmlAddPrevSibling (styleSpec->node->children, newNode));
      if (style->action == document)
	{
	  documentNode = styleSpec->node;
	  vertLinePos = ud->top_margin;
	  makeNewpage (brlNode);
	}
    }
  if (ud->braille_pages && prevStyle->action != document)
    {
      if (style->righthand_page)
	{
	  utd_fillPage ();
	  if (ud->interpoint && !(ud->braille_page_number & 1))
	    makeBlankPage ();
	}
      else if (style->newpage_before)
	utd_fillPage ();
      else if (style->lines_before > 0
	       && prevStyle->lines_after == 0 && ud->lines_on_page > 0)
	{
	  if ((ud->lines_per_page - ud->lines_on_page) < 2)
	    utd_fillPage ();
	  else if (!utd_makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  else
    {
      if (style->lines_before > 0 && prevStyle->lines_after == 0 &&
	  prevStyle->action != document)
	{
	  if (!utd_makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  return 1;
}

static int
utd_editTrans (void)
{
  if (!(ud->contents == 2) && !(style->format == computerCoded) &&
      ud->edit_table_name && (ud->has_math || ud->has_chem || ud->has_music))
    {
      lou_dotsToChar (ud->edit_table_name, ud->translated_buffer,
		      ud->text_buffer, ud->translated_length);
      translationLength = ud->translated_length;
      translatedLength = MAX_TRANS_LENGTH;
      if (!lou_translate (ud->edit_table_name,
			  ud->text_buffer,
			  &translationLength,
			  ud->translated_buffer,
			  &translatedLength, NULL, NULL,
			  NULL, indices, NULL, dotsIO))
	{
	  ud->edit_table_name = NULL;
	  return 0;
	}
    }
  translatedBuffer = ud->translated_buffer;
  translatedLength = ud->translated_length;
  return 1;
}

static int
utd_styleBody (void)
{
  sem_act action;
  if (ud->translated_length == 0)
    return 1;
  if (!utd_editTrans ())
    return 0;
  assignIndices ();
  cellsOnLine = 0;
  action = style->action;
  if (action == contentsheader && ud->contents != 2)
    {
      initialize_contents ();
      start_heading (action, translatedBuffer, translatedLength);
      finish_heading (action);
      ud->text_length = 0;
      ud->translated_length = 0;
      return 1;
    }
  if (ud->contents == 1)
    {
      if (ud->braille_pages && ud->braille_page_number_at && (action ==
							      heading1
							      || action ==
							      heading2
							      || action ==
							      heading3
							      || action ==
							      heading4))
	getBraillePageString ();
      start_heading (action, translatedBuffer, translatedLength);
    }
  switch (style->format)
    {
    case centered:
    case rightJustified:
    case leftJustified:
    default:
      utd_doOrdinaryText ();
      break;
    case alignColumnsLeft:
    case alignColumnsRight:
      utd_doAlignColumns ();
      break;
    case listColumns:
      utd_doListColumns ();
      break;
    case computerCoded:
      utd_doComputerCode ();
      break;
    case contents:
      utd_doContents ();
      break;
    }
  if (ud->contents == 1)
    finish_heading (action);
  styleSpec->status = resumeBody;
  ud->translated_length = 0;
  firstBrlNode = NULL;
  return 1;
}

static int
utd_finishStyle (void)
{
  if (ud->braille_pages)
    {
      if (style->newpage_after)
	utd_fillPage ();
      else if (style->lines_after > 0)
	{
	  if ((ud->lines_per_page - ud->lines_on_page) < 2)
	    utd_fillPage ();
	  else
	    {
	      if (!utd_makeBlankLines (style->lines_after, 1))
		return 0;
	    }
	}
    }
  else
    {
      if (style->lines_after)
	{
	  if (!utd_makeBlankLines (style->lines_after, 1))
	    return 0;
	}
    }
  brlNode = firstBrlNode = NULL;
  return 1;
}

static int makeVolumes ();

static int
utd_finish ()
{
  xmlNode *newNode;
  newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  brlNode = xmlAddChild (documentNode, newNode);
  if (ud->style_top < 0)
    ud->style_top = 0;
  if (ud->text_length != 0)
    insert_translation (ud->main_braille_table);
  if (ud->translated_length != 0)
    write_paragraph (para, NULL);
  if (ud->braille_pages)
    utd_fillPage ();
  if (ud->contents)
    make_contents ();
  if (ud->head_node)
    {
      newNode = xmlNewNode (NULL, (xmlChar *) "meta");
      xmlNewProp (newNode, (xmlChar *) "name", (xmlChar *) "utd");
      sprintf (utilStringBuf, "braillePageNumber=%d,\
topMargin=%d,\
leftMargin=%d,\
cellsPerLine=%d,\
linesPerPage=%d", ud->braille_page_number, ud->top_margin, ud->left_margin, ud->cells_per_line, ud->lines_per_page);
      xmlNewProp (newNode, (xmlChar *) "content", (xmlChar *) utilStringBuf);
      xmlAddChild (ud->head_node, newNode);
    }
  free (indices);
  if (backIndices != NULL)
    free (backIndices);
  if (backBuf != NULL)
    free (backBuf);
  if (ud->volume_sem)
    makeVolumes ();
  else
    {
      if (ud->outFile)
	xmlDocDump (ud->outFile, ud->doc);
      else
	{
	  xmlChar *dumpLoc;
	  int dumpSize;
	  xmlDocDumpMemory (ud->doc, &dumpLoc, &dumpSize);
	  if (dumpSize > (CHARSIZE * ud->outlen))
	    ud->outlen_so_far = 0;
	  else
	    {
	      memcpy (ud->outbuf, dumpLoc, dumpSize);
	      ud->outlen_so_far = dumpSize;
	    }
	  xmlFree (dumpLoc);
	}
    }
  return 1;
}

/* Functions for dividing a book into volumes */

static int
nullPrivate (xmlNode * node)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  node->_private = NULL;
  child = node->children;
  while (child)
    {
      child->_private = NULL;
      nullPrivate (child);
      child = child->next;
    }
  return 1;
}

static int
makeVolumes ()
{
  xmlNode *rootElement = xmlDocGetRootElement (ud->doc);
  int haveSemanticFile;
  if (rootElement == NULL)
    {
      lou_logPrint ("Document is empty");
      return 0;
    }
  clean_semantic_table ();
  ud->format_for = textDevice;
  ud->contains_utd = 1;
  ud->semantic_files = ud->volume_sem;
  haveSemanticFile = compile_semantic_table (rootElement);
  nullPrivate (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  transcribe_document (rootElement);
  return 1;
}
