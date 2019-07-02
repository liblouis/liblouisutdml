/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   Abilitiessoft, Inc. www.abilitiessoft.org
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

   Maintained by John J. Boyer john.boyer@abilitiessoft.org
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"

static xmlNode *firstBrlNode;
static xmlNode *prevBrlNode;
static xmlNode *documentNode;
static xmlNode *parentOfBrlOnlyNode;
static xmlNode *brlOnlyNode;
static xmlNode *newlineNode;
static xmlNode *newpageNode;
static xmlChar *brlContent;
static int maxContent;
static StyleRecord *styleSpec;
/* Note that the following is an actual data area, not a pointer*/
static StyleRecord prevStyleSpec;
static StyleType *style;
static StyleType *prevStyle;
static int styleBody ();
static xmlNode *brlNode;
/*
 * addBoxline adds a boxline to the document using the character given.
 * boxChar: The character to fill the line with.
 * beforeAfter: Whether the line is before or after content. Value 1 for after
 *              and -1 for before.
 */
static int addBoxline(const char *boxChar, int beforeAfter);
/*
 * utd_addBoxline, UTD version of addBoxline
 * boxChar: The character to fill the line with.
 * beforeAfter: Whether the boxline should be before or after the content.
 *              Value 1 for after and value -1 for before.
 */
static int utd_addBoxline(const char *boxChar, int beforeAfter);
/*
 * makeDotsTextNode, adds a text node containing dots to a brl node.
 */
static int makeDotsTextNode(xmlNode *node, const widechar *content, int length, int kind);
static int utd_startLine();
static int utd_finishLine(int number, int beforeAfter);

static int handlePagenum (xmlChar * printPageNumber, int length);

static void
initializeTranscriber()
{
  firstBrlNode = NULL;
  prevBrlNode = NULL;
  documentNode = NULL;
  parentOfBrlOnlyNode = NULL;
  brlOnlyNode = NULL;
  newlineNode = NULL;
  newpageNode = NULL;
  brlContent = NULL;
}
int
fineFormat ()
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
find_current_style ()
{
  StyleRecord *sr = &ud->style_stack[ud->style_top];
  return sr->style;
}

static int doLeftJustify ();
static widechar pageNumberString[MAXNUMLEN];
static int pageNumberLength;
static char *blanks =
  "                                                                      ";
static int fillPage ();
static int writeOutbuf ();
static int insertCharacters (char *chars, int length);

void widecharcpy (widechar * to, const widechar * from, int length);

void
widestrcpy (widechar * to, const widechar * from)
{
  widecharcpy (to, from, -1);
}

void
widecharcpy (widechar * to, const widechar * from, int length)
{
  int k;
  if (length < 0)
    {
      for (k = 0; from[k]; k++)
	to[k] = from[k];
    }
  else
    {
      for (k = 0; k < length; k++)
	to[k] = from[k];
    }
  to[k] = 0;
}

void
unsignedcharcpy (char *to, const char *from, int length)
{
  int k;
  for (k = 0; k < length; k++)
    to[k] = from[k];
  to[k] = 0;
}

void
charcpy (char *to, const char *from, int length)
{
  int k;
  for (k = 0; k < length; k++)
    to[k] = from[k];
  to[k] = 0;
}

static char *makeRomanNumber (int n);
static char *makeRomanCapsNumber (int n);
static int utd_start ();
static int utd_finish ();
static int utd_insert_translation (const char *table);
static void utd_insert_text (xmlNode * node, int length);
static int utd_makeBlankLines (int number, int beforeAfter);
static void utd_pagebreak (xmlNode * node, char *printPageNumber, int length);
static int utd_startStyle ();
static int utd_styleBody ();
static int utd_finishStyle ();
static const void *firstTableHeader;
static const char *firstTableName;

int
start_document ()
{
  logMessage(LOU_LOG_INFO, "Starting new document");
  ud->head_node = NULL;
  initializeTranscriber();
  if (ud->has_math)
    firstTableName = ud->main_braille_table = ud->mathtext_table_name;
  else
    firstTableName = ud->main_braille_table = ud->contracted_table_name;
  if (!(firstTableHeader = lou_getTable (ud->main_braille_table)))
    {
      logMessage (LOU_LOG_ERROR, "Cannot open main table %s", ud->main_braille_table);
      return 0;
    }
  if (ud->has_contentsheader)
    ud->braille_page_number = 1;
  else
    ud->braille_page_number = ud->beginning_braille_page_number;
  if (ud->has_pagebreak)
    {
      ud->page_number = 1;
      /* Prepare first page number */
      handlePagenum (NULL, 0);
    }
  ud->outbuf1_len_so_far = 0;
  styleSpec = &prevStyleSpec;
  style = prevStyle = lookup_style ("document");
  prevStyleSpec.style = prevStyle;
  if (ud->format_for != utd)
    {
      if (ud->outFile && ud->output_encoding == lbu_utf16)
	{
	  /*Little Endian indicator */
	  fputc (0xff, ud->outFile);
	  fputc (0xfe, ud->outFile);
	}
      switch (ud->format_for)
	{
	case textDevice:
	  break;
	case browser:
	  if (!insertCharacters
	      ("<html><head><title>HTML Document</title></head><body><pre>",
	       58))
	    return 0;
	  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	    return 0;
	  writeOutbuf ();
	  break;
	default:
	  break;
	}
    }
  if (ud->contents && !ud->has_contentsheader)
    initialize_contents ();
  if(ud->endnotes)
	initialize_endnotes();
  if (ud->format_for == utd)
    return utd_start ();
  return 1;
}

int
end_document ()
{
  if (ud->format_for == utd)
    return utd_finish ();
  if (ud->style_top < 0)
    ud->style_top = 0;
  if (ud->text_length != 0)
    insert_translation (ud->main_braille_table);
  if (ud->translated_length != 0)
    write_paragraph (para, NULL);
  if (ud->braille_pages)
    {
      fillPage ();
      writeOutbuf ();
    }
  if(ud->endnotes)
	make_endnotes();
  if (ud->contents)
    make_contents ();
  switch (ud->format_for)
    {
    case textDevice:
      break;
    case browser:
      if (!insertCharacters ("</pre></body></html>", 20))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      writeOutbuf ();
      break;
    default:
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
transcribe_text_string ()
{
  int charsProcessed = 0;
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char paragraphBuffer[BUFSIZE];
  StyleType *docStyle = lookup_style ("document");
  StyleType *paraStyle = lookup_style ("para");
  if (!start_document ())
    return 0;
  ud->input_encoding = ud->input_text_encoding;
  start_style (docStyle, NULL);
  while (1)
    {
      while (charsProcessed < ud->inlen)
	{
	  start_style (paraStyle, NULL);
	  ch = ud->inbuf[charsProcessed++];
	  if (ch == 0 || ch == 13)
	    continue;
	  if (ch == '\n' && pch == '\n')
	    break;
	  if (charsInParagraph == 0 && ch <= 32)
	    continue;
	  pch = ch;
	  if (ch == 10)
	    ch = ' ';
	  if (charsInParagraph >= MAX_LENGTH)
	    break;
	  paragraphBuffer[charsInParagraph++] = ch;
	}
      if (charsInParagraph == 0)
	break;
      ch = ud->inbuf[charsProcessed++];
      paragraphBuffer[charsInParagraph] = 0;
      if (!insert_utf8 (paragraphBuffer))
	return 0;
      if (!insert_translation (ud->main_braille_table))
	return 0;
      if (ch == 10)
	do_blankline ();
      end_style (paraStyle);
      charsInParagraph = 0;
      pch = 0;
      if (ch > 32)
	paragraphBuffer[charsInParagraph++] = ch;
    }
  ud->input_encoding = lbu_utf8;
  end_style (docStyle);
  end_document ();
  return 1;
}

int
transcribe_text_file ()
{
  int charsInParagraph = 0;
  int ch;
  int pch = 0;
  unsigned char paragraphBuffer[BUFSIZE];
  widechar outbufx[BUFSIZE];
  int outlenx = MAX_LENGTH;
  StyleType *docStyle = lookup_style ("document");
  StyleType *paraStyle = lookup_style ("para");
  if (!start_document ())
    return 0;
  start_style (docStyle, NULL);
  ud->outbuf = outbufx;
  ud->outbuf1_len = outlenx;
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
  ud->input_encoding = lbu_utf8;
  end_style ();
  end_document ();
  return 1;
}

#define MAXBYTES 7
static int first0Bit[MAXBYTES] = { 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0XFE };

int
utf8_string_to_wc (const unsigned char *inStr, int *inSize, widechar *
		   outstr, int *outSize)
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
      ch = inStr[in++] & 0xff;
      if (ch < 128 || ud->input_encoding == lbu_ascii8)
	{
	  outstr[out++] = (widechar) ch;
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
	  utf32 = (utf32 << 6) + (inStr[in++] & 0x3f);
	}
      if (CHARSIZE == 2 && utf32 > 0xffff)
	utf32 = 0xffff;
      outstr[out++] = (widechar) utf32;
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

int
wc_string_to_utf8 (const widechar * inStr, int *inSize, unsigned char *outstr,
		   int *outSize)
{
  int in = 0;
  int out = 0;
  unsigned char utf8Str[10];
  unsigned int utf8Bytes[MAXBYTES] = { 0, 0, 0, 0, 0, 0, 0 };
  int utf8Length;
  int numBytes;
  unsigned int utf32;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin wc_string_to_utf8: inSize=%d, outSize=%d", *inSize, *outSize);
  logWidecharBuf(LOU_LOG_DEBUG, "inStr=", inStr, *inSize);
  logMessage(LOU_LOG_DEBUG, "inStr=%d, outstr=%d", inStr, outstr);
  while (in < *inSize)
    {
      utf32 = inStr[in++];
      if (utf32 < 128)
	{
	  utf8Str[0] = utf32;
	  utf8Length = 1;
	}
      else
	{
	  for (numBytes = 0; numBytes < MAXBYTES - 1; numBytes++)
	    {
	      utf8Bytes[numBytes] = utf32 & 0x3f;
	      utf32 >>= 6;
	      if (utf32 == 0)
		break;
	    }
	  utf8Str[0] = first0Bit[numBytes] | utf8Bytes[numBytes];
	  numBytes--;
	  utf8Length = 1;
	  while (numBytes >= 0)
	    utf8Str[utf8Length++] = utf8Bytes[numBytes--] | 0x80;
	  if ((out + utf8Length) > *outSize)
	    {
	      *inSize = in;
	      *outSize = out;
              logMessage(LOU_LOG_DEBUG, "Finish wc_string_to_utf8 due to not enough memory in outstr");
	      return 1;
	    }
	}
      logMessage(LOU_LOG_DEBUG, "Adding UTF8 character to output");
      for (k = 0; k < utf8Length; k++)
      {
        logMessage(LOU_LOG_DEBUG, "out=%d, k=%d, utf8Str[%d]=%c", out, k, k, utf8Str[k]);
        outstr[out++] = utf8Str[k];
      }
      logMessage(LOU_LOG_DEBUG, "Moving to next character");
    }
  // We normally want to null terminate the string but should not if it will not fit in allocated memory
  if (out < *outSize)
    outstr[out++] = 0;
  *inSize = in;
  *outSize = out;
  logMessage(LOU_LOG_DEBUG, "Finish wc_string_to_utf8: outstr=%s, outSize=%d", outstr, *outSize);
  return 1;
}

static unsigned char *
wcCharToUtf8 (widechar ch)
{
  static unsigned char utf8Str[10];
  unsigned int utf8Bytes[MAXBYTES] = { 0, 0, 0, 0, 0, 0, 0 };
  int numBytes;
  int k;
  unsigned int utf32;
  if (ch < 128)
    {
      utf8Str[0] = ch;
      utf8Str[1] = 0;
      return utf8Str;
    }
  utf32 = ch;
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
utf8ToWc (const unsigned char *utf8str, int *inSize, widechar *
	  utfwcstr, int *outSize)
{
  int in = 0;
  int out = 0;
  int lastInSize = 0;
  int lastOutSize = 0;
  unsigned int ch;
  int numBytes;
  unsigned int utf32;
  int k;
  while (in < *inSize)
    {
      ch = utf8str[in++] & 0xff;
      if (ch < 128 || ud->input_encoding == lbu_ascii8)
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
	{
	  utf32 = 0xfffd;
	  logMessage(LOU_LOG_WARN, "Warning: Character 0x%.4x out of range, substituting with u+fffd", utf32);
	}
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

static int
maximum (int x, int y)
{
  if (x >= y)
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
  return length;
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

#define SHY 0x00AD
#define ZWSP 0x200B

int
remove_soft_hyphens (const widechar * inbuf, const int inlen,
		     widechar * outbuf, int *outlen, int *indices)
{
  int i, j;
  j = 0;
  for (i = 0; i < inlen; i++)
    if (inbuf[i] != SHY && inbuf[i] != ZWSP)
      {
	outbuf[j] = inbuf[i];
	indices[j] = i;
	j++;
      }
  *outlen = j;
  return (*outlen == inlen);
}

int
translate_possibly_prehyphenated (const char *table,
				  const widechar * inbuf, int *inlen,
				  widechar * outbuf, int *outlen,
				  formtype *typeform, int *indices, int mode)
{
  static widechar tmp_outbuf[2 * BUFSIZE];
  static formtype tmp_typeform[2 * BUFSIZE];
  static int tmp_indices_1[2 * BUFSIZE];
  static int tmp_indices_2[2 * BUFSIZE];
  int tmp_outlen;
  int k;
  if (ud->hyphenate & 2) // pre-hyphenated
    {
      remove_soft_hyphens (inbuf, *inlen, tmp_outbuf, &tmp_outlen,
			   tmp_indices_1);
      if (typeform != NULL)
	for (k = 0; k < tmp_outlen; k++)
	  tmp_typeform[k] = typeform[tmp_indices_1[k]];
      if (!lou_translate
	  (table, tmp_outbuf, &tmp_outlen, outbuf, outlen,
	   (typeform == NULL ? NULL : tmp_typeform), NULL, NULL,
	   tmp_indices_2, NULL, mode))
	return 0;
      if (indices != NULL)
	for (k = 0; k < *outlen; k++)
	  indices[k] = tmp_indices_1[tmp_indices_2[k]];
      return 1;
    }
  else
    return lou_translate (table, inbuf, inlen, outbuf, outlen, typeform, NULL,
			  NULL, indices, NULL, mode);
}

int
insert_translation (const char *table)
{
  int translationLength;
  int translatedLength;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin insert_translation");
  if (style->translation_table != NULL)
    table = style->translation_table;
  if (table == NULL)
    {
      memset (ud->typeform, 0, sizeof (ud->typeform));
      ud->text_length = 0;
      logMessage(LOU_LOG_DEBUG, "Finished insert_translation, table not defined");
      return 0;
    }
  if (ud->text_length == 0)
  {
    logMessage(LOU_LOG_DEBUG, "Finished insert_translation, no text to translate");
    return 1;
  }
  for (k = 0; k < ud->text_length && ud->text_buffer[k] <= 32; k++);
  if (k == ud->text_length)
    {
      ud->text_length = 0;
      logMessage(LOU_LOG_DEBUG, "Finished insert_translation, only whitespace");
      return 1;
    }
  if (styleSpec != NULL && styleSpec->status == resumeBody)
    styleSpec->status = bodyInterrupted;
  if (ud->format_for == utd)
  {
    logMessage(LOU_LOG_DEBUG, "Finished insert_translation, delegating to utd_insert_translation");
    return (utd_insert_translation (table));
  }
  if (ud->translated_length > 0 && ud->translated_length <
      MAX_TRANS_LENGTH &&
      ud->translated_buffer[ud->translated_length - 1] > 32
	  && ud->text_buffer[0] != 32)
    {
      ud->translated_buffer[ud->translated_length++] = 32;
      if (ud->in_sync)
	{
	  ud->positions_array[ud->translated_length - 1] =
	    ud->sync_text_length;
	  ud->sync_text_buffer[ud->sync_text_length++] = 32;
	}
    }
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  ud->text_buffer[ud->text_length++] = 32;
  ud->text_buffer[ud->text_length++] = 32;
  k = translate_possibly_prehyphenated (table,
					&ud->text_buffer[0],
					&translationLength,
					&ud->translated_buffer[ud->
							       translated_length],
					&translatedLength,
					&ud->typeform[0],
					ud->in_sync ? &ud->
					positions_array[ud->
							translated_length] :
					NULL, 0);
  memset (ud->typeform, 0, sizeof (ud->typeform));
  ud->text_length = 0;
  if (!k)
    {
      logMessage (LOU_LOG_ERROR, "Cannot find table %s", table);
      return 0;
    }
  if (ud->in_sync)
    {
      if (ud->sync_text_length > 0)
	for (k = 0; k < translationLength; k++)
	  ud->positions_array[ud->translated_length + k] +=
	    ud->sync_text_length;
      memcpy (&ud->sync_text_buffer[ud->sync_text_length], ud->text_buffer,
	      translationLength * CHARSIZE);
      ud->sync_text_length += translationLength;
    }
  if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
    ud->translated_length += translatedLength;
  else
    {
      ud->translated_length = MAX_TRANS_LENGTH;
      if (!write_paragraph (para, NULL))
      {
        logMessage(LOU_LOG_DEBUG, "Finished insert_translation, issue with write_paragraph");
	return 0;
      }
    }
  logMessage(LOU_LOG_DEBUG, "Finished insert_translation");
  return 1;
}

static int cellsWritten;
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
  if ((ud->outbuf1_len_so_far + length) >= ud->outbuf1_len)
    return 0;
  for (k = 0; k < length; k++)
    ud->outbuf1[ud->outbuf1_len_so_far++] = (widechar) chars[k];
  cellsWritten += length;
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
  cellsWritten += length;
  if (length == 0)
    return 1;
  if ((ud->outbuf1_len_so_far + length) >= ud->outbuf1_len)
    return 0;
  switch (ud->format_for)
    {
    case textDevice:
      for (k = 0; k < length; k++)
	ud->outbuf1[ud->outbuf1_len_so_far++] = (widechar) chars[k];
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
	    ud->outbuf1[ud->outbuf1_len_so_far++] = (widechar) chars[k];
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
  cellsWritten += length;
  if (length == 0)
    return 1;
  if ((ud->outbuf1_len_so_far + length) >= ud->outbuf1_len)
    return 0;
  switch (ud->format_for)
    {
    case textDevice:
      memcpy (&ud->outbuf1[ud->outbuf1_len_so_far], chars, length * CHARSIZE);
      ud->outbuf1_len_so_far += length;
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
	    ud->outbuf1[ud->outbuf1_len_so_far++] = chars[k];
	}
      break;
    default:
      break;
    }
  return 1;
}

static int startLine ();
static int finishLine ();
static int lastLineInStyle;

static int
makeBlankLines (int number)
{
  int k;
  for (k = 0; k < number; k++)
    {
      startLine ();
      if (!finishLine ())
	return 0;
    }
  return 1;
}

static int
fillPage ()
{
  if (!ud->braille_pages)
    return 0;
  if (ud->outbuf3_enabled && ud->lines_length <= MAXLINES)
    ud->lines_newpage[ud->lines_length] = 1;
  if (ud->fill_pages == 0 && ud->lines_on_page > 0)
    ud->fill_pages = 1;
  if (ud->fill_pages)
    {
      startLine ();
      writeOutbuf ();
      return 1;
    }
  return 0;
}

static int
fillPageForce ()
{
  if (!ud->braille_pages)
    return 0;
  if (ud->outbuf3_enabled && ud->lines_length <= MAXLINES)
    ud->lines_newpage[ud->lines_length] = 1;
  ud->fill_pages++;
  startLine ();
  writeOutbuf ();
  return 1;
}

static int
handlePagenum (xmlChar * printPageNumber, int length)
{
  widechar translationBuffer[MAXNUMLEN];
  int translationLength = MAXNUMLEN - 1;
  widechar translatedBuffer[MAXNUMLEN];
  int translatedLength = MAXNUMLEN;
  char autoPageNumber[MAXNUMLEN];
  char setup[MAXNAMELEN];
  if (length == 0)
    {
      sprintf (autoPageNumber, "%d", ud->page_number);
      ud->page_number++;
      printPageNumber = autoPageNumber;
    }
  else if ((printPageNumber[0] >= '0' && printPageNumber[0] <= '9'))
    ud->page_number = strtol(printPageNumber, NULL, 10);
  strcpy (setup, " ");
  if (!(printPageNumber[0] >= '0' && printPageNumber[0] <= '9'))
    strcat (setup, ud->letsign);
  strcat (setup, printPageNumber);
  length = strlen (setup);
  utf8ToWc (setup, &length, &translationBuffer[0], &translationLength);
  if (!lou_translateString (ud->main_braille_table, translationBuffer,
			    &translationLength, translatedBuffer,
			    &translatedLength, NULL, NULL, 0))
    return 0;
  if (translatedBuffer[0] != ' ')
    {
      /* Translation dropped leading space */
      ud->print_page_number[0] = ' ';
      widecharcpy (&ud->print_page_number[1], translatedBuffer,
		   translatedLength);
    }
  else
    widecharcpy (ud->print_page_number, translatedBuffer, translatedLength);
  if (!ud->page_separator_number_first[0] ||
      ud->page_separator_number_first[0] == '_' || ud->ignore_empty_pages)
    widestrcpy (ud->page_separator_number_first, ud->print_page_number);
  else
    widestrcpy (ud->page_separator_number_last, ud->print_page_number);
  return 1;
}

static int utd_makePageSeparator (xmlNode *node, char *printPageNumber, int length);

void
set_runninghead_string (widechar * chars, int length)
{
  if (length > (sizeof (ud->running_head) / CHARSIZE))
    length = sizeof (ud->running_head) / CHARSIZE - 4;
  ud->running_head_length = length;
  memcpy (ud->running_head, chars, ud->running_head_length * CHARSIZE);
}

void
set_footer_string (widechar * chars, int length)
{
  if (length > (sizeof (ud->footer) / CHARSIZE))
    length = sizeof (ud->footer) / CHARSIZE - 4;
  ud->footer_length = length;
  memcpy (ud->footer, chars, ud->footer_length * CHARSIZE);
}

void
do_pagebreak (xmlNode * node)
{
  xmlChar *number = get_attr_value (node);
  if (ud->format_for == utd)
    utd_pagebreak (node, number, strlen (number));
  else
    handlePagenum (number, strlen (number));
}

void
do_attrtotext (xmlNode * node)
{
  char attrValue[MAXNAMELEN];
  strcpy (attrValue, get_attr_value (node));
  if (ud->orig_format_for == dsbible)
    {				/* Enbncode verse numbers */
      int k;
      for (k = 0; attrValue[k]; k++)
	attrValue[k] -= 32;
    }
  insert_text_string (node, attrValue);
}

static void
setEmphasis ()
{
  int k;
  int top = ud->top;
  memset (&ud->typeform[ud->old_text_length], 0, ud->text_length -
	  ud->old_text_length);
  while (top >= 0 && (ud->stack[top] == italicx || ud->stack[top] == boldx
		      || ud->stack[top] == underlinex
		      || ud->stack[top] == compbrl))
    {
      switch (ud->stack[top])
	{
	case italicx:
	  if (!(ud->emphasis & italic))
	    break;
	  for (k = ud->old_text_length; k < ud->text_length; k++)
	    ud->typeform[k] |= italic;
	  break;
	case underlinex:
	  if (!(ud->emphasis & underline))
	    break;
	  for (k = ud->old_text_length; k < ud->text_length; k++)
	    ud->typeform[k] |= underline;
	  break;
	case boldx:
	  if (!(ud->emphasis & bold))
	    break;
	  for (k = ud->old_text_length; k < ud->text_length; k++)
	    ud->typeform[k] |= bold;
	  break;
	case compbrl:
	  if (!(ud->emphasis & computer_braille))
	    break;
	  for (k = ud->old_text_length; k < ud->text_length; k++)
	    ud->typeform[k] |= computer_braille;
	  break;
	default:
	  break;
	}
      //top--;
      top = -1; /*to be removed when liblouis fixed*/
    }
  return;
}

void
insert_text_string (xmlNode * node, xmlChar * str)
{
  xmlNode *newNode;
  if (ud->format_for == utd)
    {
      newNode = xmlNewNode (NULL, (xmlChar *) "brl");
      link_brl_node (xmlAddNextSibling (node, newNode));
    }
  insert_utf8 (str);
  if (ud->format_for == utd)
    ud->text_buffer[ud->text_length++] = LOU_ENDSEGMENT;
  return;
}

void
insert_text (xmlNode * node)
{
  int length = strlen ((char *) node->content);
  int k;
  // int stripSpace = 0;
  logMessage(LOU_LOG_DEBUG, "Begin insert_text: node->content=%s", node->content);
  // for (k = length; k > 0 && node->content[k - 1] <= 32; k--);
    // We want to track if the node only contains space 0x20 characters
    // if (node->content[k - 1] != 32)
    //   stripSpace = 1;
  // if (stripSpace == 0)
  //   k = length; // We want to keep the spaces
  //if (k <= 0)
  //  return;
  //if (k < length)
  //  length = k + 1;		/*Keep last whitespace */
  if (style->emphasis)
    push_action (style->emphasis);
  if (ud->format_for == utd)
    {
      utd_insert_text (node, length);
      logMessage(LOU_LOG_DEBUG, "Finished insert_text");
      return;
    }
  switch (ud->stack[ud->top])
    {
    case notranslate:
      insert_translation (ud->main_braille_table);
      insert_utf8 (node->content);
      if ((ud->translated_length + ud->text_length) > MAX_TRANS_LENGTH)
	ud->text_length = MAX_TRANS_LENGTH - ud->translated_length;
      memcpy (&ud->translated_buffer[ud->translated_length], ud->text_buffer,
	      ud->text_length * CHARSIZE);
      if (ud->in_sync)
	{
	  for (k = 0; k < ud->text_length; k++)
	    ud->positions_array[ud->translated_length + k] = k;
	  memcpy (&ud->sync_text_buffer[ud->sync_text_length],
		  ud->text_buffer, ud->text_length * CHARSIZE);
	  ud->sync_text_length += ud->text_length;
	}
      ud->translated_length += ud->text_length;
      ud->text_length = 0;
      logMessage(LOU_LOG_DEBUG, "Finished insert_text, notranslate action used");
      return;
    case pagenum:
      handlePagenum (node->content, length);
      logMessage(LOU_LOG_DEBUG, "Finished insert_text, pagenum action used");
      return;
    default:
      break;
    }
  ud->old_text_length = ud->text_length;
  insert_utf8 (node->content);
  setEmphasis ();
  logMessage(LOU_LOG_DEBUG, "Finished insert_text");
}

static int
getBraillePageString ()
{
  int k;
  char brlPageString[40];
  widechar translationBuffer[MAXNUMLEN];
  int translationLength;
  int translatedLength = MAXNUMLEN;
  static const char * pagenumTable;
  if (!ud->number_braille_pages)
    {
      pageNumberLength = 0;
      return 1;
    }
  switch (ud->cur_brl_page_num_format)
    {
    case blank:
      return 1;
    default:
    case normal:
      translationLength =
	sprintf (brlPageString, "%d", ud->braille_page_number);
      break;
    case p:
      translationLength =
	sprintf (brlPageString, "p%d", ud->braille_page_number);
      break;
    case roman:
      strcpy (brlPageString, " ");
      strcat (brlPageString, ud->letsign);
      strcat (brlPageString, makeRomanNumber (ud->braille_page_number));
      translationLength = strlen (brlPageString);
      break;
    case romancaps:
      strcpy (brlPageString, " ");
      strcat (brlPageString, ud->letsign);
      strcat (brlPageString, makeRomanCapsNumber (ud->braille_page_number));
      translationLength = strlen (brlPageString);
      break;
    }
  for (k = 0; k < translationLength; k++)
    translationBuffer[k] = brlPageString[k];
  pagenumTable = ud->pagenum_table_name;
  if (pagenumTable == NULL)
      pagenumTable = ud->main_braille_table;
  if (!lou_translateString (pagenumTable, translationBuffer,
			    &translationLength, ud->braille_page_string,
			    &translatedLength, NULL, NULL, 0))
    return 0;
  switch (ud->cur_brl_page_num_format) {
    case roman: case romancaps:
      translatedLength--;
      for (k = 0; k < translatedLength; k++)
        ud->braille_page_string[k] = ud->braille_page_string[k+1];
      ud->braille_page_string[k] = 0;
      break;
  }
  ud->braille_page_string[translatedLength] = 0;
  widecharcpy (&(pageNumberString[pageNumberLength]), ud->braille_page_string,
	       translatedLength);
  pageNumberLength += translatedLength;
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
  if (n <= 0)
    return NULL;
  romNum[0] = 0;
  while (n>1000) {
    strcat (romNum, hundreds[10]);
    n = n-1000;
  }
  strcat (romNum, hundreds[n / 100]);
  strcat (romNum, tens[(n / 10) % 10]);
  strcat (romNum, units[n % 10]);
  return romNum;
}

static char *
makeRomanCapsNumber (int n)
{
  static char romNum[40];
  static const char *hundreds[] = {
    "",
    "C",
    "CC",
    "CCC",
    "CD",
    "D",
    "DC",
    "DCC",
    "DCCC",
    "CM",
    "M"
  };
  static const char *tens[] = {
    "",
    "X",
    "XX",
    "XXX",
    "XL",
    "L",
    "LX",
    "LXX",
    "LXXX",
    "XC"
  };
  static const char *units[] = {
    "",
    "I",
    "II",
    "III",
    "IV",
    "V",
    "VI",
    "VII",
    "VIII",
    "IX"
  };
  if (n <= 0)
    return NULL;
  romNum[0] = 0;
  while (n>1000) {
    strcat (romNum, hundreds[10]);
    n = n-1000;
  }
  strcat (romNum, hundreds[n / 100]);
  strcat (romNum, tens[(n / 10) % 10]);
  strcat (romNum, units[n % 10]);
  return romNum;
}

static void
getPrintPageString ()
{
  int k;
  if (ud->print_page_number_first[0] != '_')
    {
      if (ud->print_page_number_first[0] != ' '
	  && ud->print_page_number_first[0] != '+')
	{
	  pageNumberString[pageNumberLength++] =
	    ud->print_page_number_first[0];
	}
      for (k = 1; ud->print_page_number_first[k]; k++)
	{
	  pageNumberString[pageNumberLength++] =
	    ud->print_page_number_first[k];
	}
      if (ud->print_page_number_last[0])
	{
	  pageNumberString[pageNumberLength++] = '-';
	  for (k = 1; ud->print_page_number_last[k]; k++)
	    {
	      pageNumberString[pageNumberLength++] =
		ud->print_page_number_last[k];
	    }
	}
    }
}

static int
getPageNumber ()
{
  int k;
  int braillePageNumber = 0;
  int printPageNumber = 0;
  pageNumberLength = 0;
  if (ud->lines_on_page == 1)
    {
      if (ud->print_pages && ud->print_page_number_at
	  && ud->print_page_number_first[0] != '_')
	{
	  printPageNumber = 1;
	}
      if (ud->braille_pages && !ud->braille_page_number_at
	  && ud->cur_brl_page_num_format != blank)
	{
	  braillePageNumber = 1;
	}
    }
  else if (ud->lines_on_page == ud->lines_per_page)
    {
      if (ud->print_pages && !ud->print_page_number_at
	  && ud->print_page_number_first[0] != '_')
	{
	  printPageNumber = 1;
	}
      if (ud->braille_pages && ud->braille_page_number_at
	  && ud->cur_brl_page_num_format != blank)
	{
	  braillePageNumber = 1;
	}
    }
  if (ud->interpoint && !(ud->braille_page_number & 1))
    braillePageNumber = 0;
  if (printPageNumber || braillePageNumber)
    {
      pageNumberString[pageNumberLength++] = ' ';
      pageNumberString[pageNumberLength++] = ' ';
      if (printPageNumber)
	{
	  //pageNumberString[pageNumberLength++] = ' ';
	  getPrintPageString ();
	}
      if (braillePageNumber)
	{
	  if(printPageNumber)
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
	  widestrcpy (ud->print_page_number_first,
		      ud->page_separator_number_first);
	}
      else if (ud->page_separator_number_first[0] != '_'
	       && (ud->print_page_number_range
		   || (ud->lines_on_page == 0 && !ud->ignore_empty_pages)))
	{
	  widestrcpy (ud->print_page_number_last,
		      ud->page_separator_number_first);
	}
      if (ud->page_separator_number_last[0]
	  && (ud->print_page_number_range || ud->lines_on_page == 0))
	{
	  widestrcpy (ud->print_page_number_last,
		      ud->page_separator_number_last);
	}
    }
  ud->page_separator_number_first[0] = 0;
  ud->page_separator_number_last[0] = 0;
}

static int
nextPrintPage ()
{
  int k;
  int kk;
  widechar separatorLine[128];
  int pageSeparatorNumberFirstLength = 0;
  int pageSeparatorNumberLastLength = 0;
  int pageSeparatorInserted = 0;
  if (ud->page_separator_number_first[0])
    {
      if (ud->braille_pages && ud->lines_on_page == 0)
	{
	}
      else if (!ud->page_separator)
	{
	}
      else if (ud->fill_pages > 0)
	{
	}
      else if (ud->braille_pages &&
	       (ud->lines_on_page == ud->lines_per_page - 1))
	{
	  ud->lines_on_page++;
	  cellsWritten = 0;
	  getPageNumber ();
	  finishLine ();
	}
      else if (ud->braille_pages &&
	       (ud->lines_on_page == ud->lines_per_page - 2))
	{
	  insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
	  ud->lines_on_page = ud->lines_per_page;
	  cellsWritten = 0;
	  getPageNumber ();
	  finishLine ();
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
		pageSeparatorNumberLastLength = 0;

	      k = 0;
	      while (k <
		     (ud->cells_per_line - pageSeparatorNumberFirstLength -
		      pageSeparatorNumberLastLength + 1))
		{
		  separatorLine[k++] = '-';
		}
	      kk = 1;
	      while (k < (ud->cells_per_line - pageSeparatorNumberLastLength))
		{
		  separatorLine[k++] = ud->page_separator_number_first[kk++];
		}
	      if (pageSeparatorNumberLastLength > 0)
		{
		  separatorLine[k++] = '-';
		  kk = 1;
		  while (k < (ud->cells_per_line))
		    {
		      separatorLine[k++] =
			ud->page_separator_number_last[kk++];
		    }
		}
	    }
	  insertWidechars (separatorLine, ud->cells_per_line);
	  pageSeparatorInserted = 1;
	  insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
	  if (ud->braille_pages)
	    ud->lines_on_page++;
	  writeOutbuf ();
	}
      addPagesToPrintPageNumber ();
    }
  return pageSeparatorInserted;
}

static void
continuePrintPageNumber ()
{
  int k;
  if (!ud->braille_pages)
    return;
  if (ud->print_page_number[0] == '_')
    {
    }
  else if (!ud->continue_pages)
    {
      ud->print_page_number[0] = '+';
    }
  else if (ud->print_page_number[0] == ' ')
    {
      ud->print_page_number[0] = 'a';
    }
  else if (ud->print_page_number[0] == 'z')
    {
      ud->print_page_number[0] = '_';
      ud->print_page_number[1] = 0;
    }
  else
    {
      ud->print_page_number[0]++;
    }
  widestrcpy (ud->print_page_number_first, ud->print_page_number);
  ud->print_page_number_last[0] = 0;
}

static int
nextBraillePage ()
{
  if (ud->braille_pages)
    {
      if (!write_buffer (1, 0))
	return 0;
      if (ud->outbuf2_enabled)
	{
	  ud->lines_on_page = 1;
	  cellsWritten = 0;
	  getPageNumber ();
	  finishLine ();
	  if (!write_buffer (1, 2))
	    return 0;
	  if (!write_buffer (2, 0))
	    return 0;
	}
      if (!insertCharacters (ud->pageEnd, strlen (ud->pageEnd)))
	return 0;
      if (!write_buffer (1, 2))
	return 0;
      ud->lines_on_page = 0;
      ud->braille_page_number++;
      continuePrintPageNumber ();
    }
  return 1;
}

static int
startLine ()
{
  int availableCells = 0;
  int blank_lines = ud->blank_lines;

  while (availableCells == 0 ||
	 (ud->braille_pages && ud->fill_pages > 0) || blank_lines > 0)
    {
      if (ud->page_separator_number_first[0])
	{
	  if (nextPrintPage ())
	    {
	      blank_lines = 0;
	      ud->blank_lines = style->lines_before;
	    }
	}

      if (ud->braille_pages)
	{
	  ud->lines_on_page++;
	  ud->after_contents = 0;
	  cellsWritten = 0;

	  if (ud->lines_on_page == 1)
	    {
	      ud->cur_brl_page_num_format = ud->brl_page_num_format;
	      getBraillePageString ();
	      getPageNumber ();
	    }
	  else if (ud->lines_on_page == ud->lines_per_page)
	    getPageNumber ();
	  else
	    pageNumberLength = 0;

	  if (ud->lines_on_page == 1)
	    {
	      blank_lines = 0;
	      ud->blank_lines = style->lines_before;
	    }

	  if (ud->lines_on_page == 1 && ud->outbuf2_enabled)
	    {
	      pageNumberLength = 0;
	      ud->lines_on_page++;
	      availableCells = ud->cells_per_line;
	    }
	  else if (ud->lines_on_page == 1 && ud->running_head_length > 0)
	    {
	      availableCells = 0;
	      blank_lines = ud->blank_lines;
	    }
	  else if (ud->lines_on_page == 1 &&
		   (pageNumberLength > 0 &&
		    (style->skip_number_lines ||
		     ud->page_number_top_separate_line)))
	    {
	      availableCells = 0;
	    }
	  else if (ud->lines_on_page == ud->lines_per_page &&
		   (ud->footer_length > 0 ||
		    (pageNumberLength > 0 &&
		     (style->skip_number_lines ||
		      ud->page_number_bottom_separate_line))))
	    {
	      availableCells = 0;
	    }
	  else
	    availableCells = ud->cells_per_line - pageNumberLength;
	}
      else if (ud->lines_on_page == 0)
	{
	  ud->lines_on_page++;
	  ud->blank_lines = 0;
	  return ud->cells_per_line;
	}
      else if (blank_lines == 0)
	{
	  ud->blank_lines = 0;
	  return ud->cells_per_line;
	}

      if (ud->braille_pages && ud->fill_pages > 0)
	finishLine ();
      else if (blank_lines > 0)
	{
	  finishLine ();
	  blank_lines--;
	  availableCells = 0;
	}
      else if (availableCells == 0)
	{
	  ud->blank_lines = 0;
	  finishLine ();
	}
      else
	{
	  ud->blank_lines = 0;
	  if (ud->outbuf3_enabled && ud->lines_length < MAXLINES)
	    {
	      ud->lines_pagenum[ud->lines_length] = ud->braille_page_number;
	      ud->lines_newpage[ud->lines_length] = 0;
	      ud->lines_length++;
	    }
	}

      if (ud->fill_pages > 0 && ud->lines_on_page == 0)
	{
	  ud->fill_pages--;
	  if (ud->fill_pages == 0)
	    break;
	  else
	    availableCells = 0;
	}
    }
  return availableCells;
}

static int
centerHeadFoot (widechar * toCenter, int length)
{
  int leadingBlanks;
  int trailingBlanks;
  int numCells = ud->cells_per_line - (2 * pageNumberLength);
  if (length > numCells)
    {
      int k;
      length = numCells;
      for (k = length; toCenter[k] != ' ' && k >= 0; k--);
      if (k > 0)
	length = k;
    }
  leadingBlanks = ((numCells - length) / 2) + pageNumberLength;
  trailingBlanks = numCells - leadingBlanks - length + pageNumberLength;
  if (!insertCharacters (blanks, leadingBlanks))
    return 0;
  if (!insertWidechars (toCenter, length))
    return 0;
  if (pageNumberLength > 0)
    {
      if (!insertCharacters (blanks, trailingBlanks))
	return 0;
      if (!insertWidechars (pageNumberString, pageNumberLength))
	return 0;
    }
  return 1;
}

static int
finishLine ()
{
  int cellsToWrite = 0;
  int leaveBlank;
  for (leaveBlank = -1; leaveBlank < ud->line_spacing; leaveBlank++)
    {
      if (leaveBlank != -1)
	startLine ();
      if (ud->braille_pages)
	{
	  if (cellsWritten > 0 && pageNumberLength > 0)
	    {
	      cellsToWrite =
		ud->cells_per_line - pageNumberLength - cellsWritten;
	      if (!insertCharacters (blanks, cellsToWrite))
		return 0;
	      if (!insertWidechars (pageNumberString, pageNumberLength))
		return 0;
	    }
	  else if (ud->lines_on_page == 1)
	    {
	      if (ud->running_head_length > 0)
		centerHeadFoot (ud->running_head, ud->running_head_length);
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
		centerHeadFoot (ud->footer, ud->footer_length);
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
      if (!(lastLineInStyle && !style->newline_after))
	{
	  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	    return 0;
	}
      if (ud->braille_pages && ud->lines_on_page == ud->lines_per_page)
	{
	  if (!nextBraillePage ())
	    return 0;
	}
    }
  return 1;
}

static int
makeBlankPage ()
{
  if (!ud->braille_pages)
    return 1;
  if (ud->format_for != utd)
    {
      if (!makeBlankLines (ud->lines_per_page))
	return 0;
    }
  else
    {
      if (!utd_makeBlankLines (ud->lines_per_page, 2))
	return 0;
    }
  return 1;
}

static int
writeOutbuf ()
{
  return write_buffer (1, 0);
}

int
write_buffer (int from, int skip)
{
  int to = 0;
  widechar *buffer_from;
  widechar *buffer_to;
  int buffer_from_len;
  int buffer_to_len;
  int *buffer_from_len_so_far;
  int *buffer_to_len_so_far;
  int k;
  switch (from)
    {
    case 1:
      if (skip != 2 && ud->outbuf2_enabled)
	to = 2;
      else if (skip != 3 && ud->outbuf3_enabled)
	to = 3;
      buffer_from = ud->outbuf1;
      buffer_from_len = ud->outbuf1_len;
      buffer_from_len_so_far = &ud->outbuf1_len_so_far;
      break;
    case 2:
      if (!ud->outbuf2_enabled)
	return 0;
      if (skip != 3 && ud->outbuf3_enabled)
	to = 3;
      buffer_from = ud->outbuf2;
      buffer_from_len = ud->outbuf2_len;
      buffer_from_len_so_far = &ud->outbuf2_len_so_far;
      break;
    case 3:
      if (!ud->outbuf3_enabled)
	return 0;
      buffer_from = ud->outbuf3;
      buffer_from_len = ud->outbuf3_len;
      buffer_from_len_so_far = &ud->outbuf3_len_so_far;
      break;
    default:
      return 0;
    }
  switch (to)
    {
    case 0:
      if (*buffer_from_len_so_far == 0)
	return 1;
      if (ud->outFile == NULL)
	{
	  if ((ud->outlen_so_far + *buffer_from_len_so_far) >= ud->outlen)
	    return 0;
	  for (k = 0; k < *buffer_from_len_so_far; k++)
	    ud->outbuf[ud->outlen_so_far++] = buffer_from[k];
	  *buffer_from_len_so_far = 0;
	  return 1;
	}
      else
	{
	  unsigned char *utf8Str;
	  switch (ud->output_encoding)
	    {
	    case lbu_utf8:
	      for (k = 0; k < *buffer_from_len_so_far; k++)
		{
		  utf8Str = utfwcto8 (buffer_from[k]);
		  fwrite (utf8Str, strlen ((char *) utf8Str), 1, ud->outFile);
		}
	      break;
	    case lbu_utf16:
	      for (k = 0; k < *buffer_from_len_so_far; k++)
		{
		  unsigned short uc16 = (unsigned short) buffer_from[k];
		  fwrite (&uc16, 1, sizeof (uc16), ud->outFile);
		}
	      break;
	    case lbu_utf32:
	      for (k = 0; k < *buffer_from_len_so_far; k++)
		{
		  unsigned int uc32 = (unsigned int) buffer_from[k];
		  fwrite (&uc32, 1, sizeof (uc32), ud->outFile);
		}
	      break;
	    case lbu_ascii8:
	      for (k = 0; k < *buffer_from_len_so_far; k++)
		{
		  fputc ((char) buffer_from[k], ud->outFile);
		}
	      break;
	    default:
	      break;
	    }
	  *buffer_from_len_so_far = 0;
	}
      return 1;
    case 2:
      buffer_to = ud->outbuf2;
      buffer_to_len = ud->outbuf2_len;
      buffer_to_len_so_far = &ud->outbuf2_len_so_far;
      break;
    case 3:
      buffer_to = ud->outbuf3;
      buffer_to_len = ud->outbuf3_len;
      buffer_to_len_so_far = &ud->outbuf3_len_so_far;
      break;
    default:
      return 0;
    }
  if (*buffer_from_len_so_far == 0)
    return 1;
  if ((*buffer_to_len_so_far + *buffer_from_len_so_far) >= buffer_to_len)
    return 0;
  for (k = 0; k < *buffer_from_len_so_far; k++)
    buffer_to[(*buffer_to_len_so_far)++] = buffer_from[k];
  *buffer_from_len_so_far = 0;
  return 1;
}

static widechar *translatedBuffer;
static int translatedLength;
static int *positionsArray;

static widechar *saved_translatedBuffer;
static int saved_translatedLength;
static int *saved_positionsArray;

void
save_translated_buffer ()
{
  saved_translatedBuffer = translatedBuffer;
  saved_translatedLength = translatedLength;
  saved_positionsArray = positionsArray;
}

void
restore_translated_buffer ()
{
  translatedBuffer = saved_translatedBuffer;
  translatedLength = saved_translatedLength;
  positionsArray = saved_positionsArray;
}

/**
 * hyphenatex
 * 
 * Find out where to break the current line so that the first word that
 * doesn't fit entirely on the line is hyphenated correctly. The word in
 * question will not be split if it results in parts smaller than
 * MIN_SYLLABLE_LENGTH, if the word is smaller than MIN_WORD_LENGTH, or if
 * less than MIN_NEXT_LINE characters remain on the next line.
 * 
 * Two situations can occur:
 * - If the text buffer and the translated buffer are in sync (i.e. positions
 *   in the text buffer can be derived from positions in the translated
 *   buffer), the word being split is looked up in the text buffer, then it's
 *   hyphenated (if not already pre-hyphenated), and finally from the hyphen
 *   positions the valid break points in the translated buffer are derived.
 * - If the buffers are not in sync, the possible break points are computed by
 *   hyphenating the braille word (using back-translation).
 * 
 * The last break point that still fits on the line is returned. Break points
 * that don't require a hyphen character to be inserted are chosen over break
 * points that do. If no suitable break point can be found, zero is returned.
 * 
 * @param lastBlank      The position of the last whitespace character before
 *                       lineEnd.
 * @param lineEnd        The position of the last character that could fit on
 *                       the current line.
 * @return               Non-zero if a suitable break point was found.
 * @return *breakAt      The position where the line should be broken.
 * @return *insertHyphen Non-zero if a hyphen character needs to be inserted
 *                       after the break point.
 */
static int
hyphenatex (int lastBlank, int lineEnd, int *breakAt, int *insertHyphen)
{

#define MIN_SYLLABLE_LENGTH 2
#define MIN_WORD_LENGTH 5
#define MIN_NEXT_LINE 12

  int k;
  char hyphens[MAXNAMELEN];
  char textHyphens[MAXNAMELEN];
  int wordStart = lastBlank + 1;
  int wordLength;
  int textWordStart, textWordLength;
  int const textLength = ud->sync_text_length;
  widechar *const textBuffer = ud->sync_text_buffer;
  int minSyllableLength = MIN_SYLLABLE_LENGTH;
  if (ud->min_syllable_length > 0)
      minSyllableLength = ud->min_syllable_length;
  
  if (!ud->hyphenate)
    return 0;
  
  // Don't break if not enough characters remain on next line
  if ((translatedLength - wordStart) < MIN_NEXT_LINE)
    return 0;
  
  // Find word boundaries
  for (wordLength = wordStart; wordLength < translatedLength; wordLength++)
    if (translatedBuffer[wordLength] == ' ')
      break;
  wordLength -= wordStart;
  
  // Don't break if word is too short
  if (wordLength < MIN_WORD_LENGTH)
    return 0;
  
  // Find the word boundaries in the text buffer
  if (ud->in_sync)
    {
      textWordStart = positionsArray[wordStart];
      if (wordStart + wordLength >= translatedLength)
	textWordLength = textLength - textWordStart;
      else
	textWordLength =
	  positionsArray[wordStart + wordLength] - textWordStart;
      if (textWordStart < 0 || textWordLength < 1
	  || textWordStart + textWordLength > textLength)
	return 0;
    }
  
  // First look for break points that don't require a hyphen character to be
  // inserted
  *insertHyphen = 0;
  
  for (k = minimum(lineEnd - wordStart, wordLength - minSyllableLength);
       k > minSyllableLength;
       k--)
    {
      if (ud->in_sync && ud->hyphenate & 2)
	{
	  if (textBuffer[positionsArray[wordStart + k] - 1] == ZWSP)
	    {
	      *breakAt = wordStart + k;
	      return 1;
	    }
	}
      if (ud->hyphenate & 1)
	{
	  if(translatedBuffer[wordStart + k - 1] == *ud->lit_hyphen)
	    {
	      *breakAt = wordStart + k;
	      return 1;
	    }
	}
    }
  
  // Then look for other break points
  for (k = 0; k <= wordLength; k++)
    hyphens[k] = '0';
  
  // Derive hyphen positions in translated buffer from those in text buffer
  if (ud->in_sync)
    {
      for (k = 0; k < textWordLength; k++)
	textHyphens[k] = '0';
      switch (ud->hyphenate)
	{
	case 2:
	case 3: {
	  int containsShy = 0;
	  for (k = 1; k < textWordLength; k++)
	    if (textBuffer[textWordStart + k - 1] == SHY) {
	      textHyphens[k] = '1';
	      containsShy = 1;
	    }
	  if (ud->hyphenate == 2 || containsShy) break;
	  // if "hyphenate yes" and word contains no SHY
	}
	case 1:
	  if (!lou_hyphenate (ud->main_braille_table,
			      &textBuffer[textWordStart], textWordLength,
			      textHyphens, 0))
	    return 0;
	  break;
	}
      for (k = 1; k < wordLength; k++)
        hyphens[k] =
          textHyphens[positionsArray[wordStart + k] - textWordStart];
    }
  
  // Otherwise, use back-translation
  else if (!lou_hyphenate (ud->main_braille_table,
			   &translatedBuffer[wordStart], wordLength,
			   hyphens, 1))
    return 0;

  // First look for break points that don't require a hyphen character to be
  // inserted
  *insertHyphen = 0;
  for (k = minimum(lineEnd - wordStart, wordLength - minSyllableLength);
       k >= minSyllableLength;
       k--)
    if (hyphens[k] == '2')
      {
	*breakAt = wordStart + k;
	return 1;
      }

  // Then look for other break points
  *insertHyphen = 1;
  for (k = minimum(lineEnd - wordStart - 1, wordLength - minSyllableLength);
       k >= minSyllableLength;
       k--)
    if (hyphens[k] == '1')
      {
	*breakAt = wordStart + k;
	return 1;
      }
  
  return 0;
}

#define escapeChar 0x1b

static int
doAlignColumns ()
{
#define MAXCOLS 100
#define MAXROWSIZE 400
#define COLSPACING 2
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
  if (ud->style_format == alignColumnsLeft)
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
      if (ud->style_format == alignColumnsLeft)
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
	      if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
		return 0;
	    }
	  finishLine ();
	}
    }
  return 1;
}

static int
doListColumns ()
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
	      int insertHyphen = 0;
	      int leadingBlanks = 0;
	      availableCells = startLine ();
	      if (styleSpec->status == startBody)
		{
		  if (ud->style_first_line_indent < 0)
		    leadingBlanks = 0;
		  else
		    leadingBlanks =
		      ud->style_left_margin + ud->style_first_line_indent;
		  styleSpec->status = resumeBody;
		}
	      else
		leadingBlanks = ud->style_left_margin;
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
		    wordTooLong = 1;
		  if (ud->hyphenate)
		    if (hyphenatex (charactersWritten + cellsToWrite,
				    charactersWritten + availableCells,
				    &breakAt,
				    &insertHyphen)) {
		      cellsToWrite = breakAt - charactersWritten;
		      wordTooLong = 0;
		    }
		  if (wordTooLong)
		    cellsToWrite = availableCells - 1;
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
	      if ((breakAt && insertHyphen) || wordTooLong)
		if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
		  return 0;
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
doListLines ()
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
	    int insertHyphen = 0;
	    int leadingBlanks = 0;
	    availableCells = startLine ();
	    if (styleSpec->status == startBody)
	      {
		if (ud->style_first_line_indent < 0)
		  leadingBlanks = 0;
		else
		  leadingBlanks =
		    ud->style_left_margin + ud->style_first_line_indent;
		styleSpec->status = resumeBody;
	      }
	    else
	      leadingBlanks = ud->style_left_margin;
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
		  wordTooLong = 1;
		if (ud->hyphenate)
		  if (hyphenatex (charactersWritten + cellsToWrite,
				  charactersWritten + availableCells,
				  &breakAt,
				  &insertHyphen)) {
		    cellsToWrite = breakAt - charactersWritten;
		    wordTooLong = 0;
		  }
		if (wordTooLong)
		  cellsToWrite = availableCells - 1;
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
	    if ((breakAt && insertHyphen) || wordTooLong)
	      if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
		return 0;
	    finishLine ();
	  }
      }
  return 1;
}

static int
doComputerCode ()
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
	  cellsToWrite = availableCells - strlen (ud->comp_hyphen);
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
	  if (!insertDubChars (ud->comp_hyphen, strlen (ud->comp_hyphen)))
	    return 0;
	}
      finishLine ();
    }
  return 1;
}

static int
doLeftJustify ()
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (charactersWritten < translatedLength)
    {
      int wordTooLong = 0;
      int breakAt = 0;
      int insertHyphen;
      int leadingBlanks = 0;
      int trailingBlanks = 0;
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = ud->style_left_margin + ud->style_first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = ud->style_left_margin;
      trailingBlanks = ud->style_right_margin;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      availableCells -= trailingBlanks;
      if ((charactersWritten + availableCells) >= translatedLength)
	{
	  cellsToWrite = translatedLength - charactersWritten;
	  lastLineInStyle = 1;
	}
      else
	{
	  for (cellsToWrite = availableCells; cellsToWrite > 0;
	       cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == ' ')
	      break;
	  if (cellsToWrite == 0)
	    wordTooLong = 1;
	  if (ud->hyphenate)
	    if (hyphenatex (charactersWritten + cellsToWrite,
			    charactersWritten + availableCells,
			    &breakAt,
			    &insertHyphen)) {
	      cellsToWrite = breakAt - charactersWritten;
	      wordTooLong = 0;
	    }
	  if (wordTooLong)
	    cellsToWrite = availableCells - 1;
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
      if ((breakAt && insertHyphen) || wordTooLong)
	if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
	  return 0;
      finishLine ();
    }
  return 1;
}

static int
doContents ()
{
  int lastWord;
  int untilLastWord;
  int numbersStart;
  int numbersLength;
  int leadingBlanks = 0;
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int minGuideDots = 2;		// Only print guide dots if space between last word and page number >= 1 + 2 + 1
  int minSpaceAfterLastWord = 1;	// Minumum space between last word and page number = 1 cell
  int minSpaceAfterNotLastWord = 6;	// Minimum space after any braille line that is continued on the next line = 6 cells
  int lastWordNewRule = 0;	// Last word begins on a new rule

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
  for (k = numbersStart; k < translatedLength; k++)
    if (translatedBuffer[k] == 0xa0)
      translatedBuffer[k] = ' ';
  untilLastWord = lastWord - 1;
  while (charactersWritten < untilLastWord)
    {
      int wordTooLong = 0;
      int breakAt = 0;
      int insertHyphen = 0;
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = ud->style_left_margin + ud->style_first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = ud->style_left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;

      if ((charactersWritten + availableCells) >=
	  (untilLastWord + minSpaceAfterNotLastWord))
	cellsToWrite = untilLastWord - charactersWritten;
      else
	{
	  for (cellsToWrite = availableCells - minSpaceAfterNotLastWord;
	       cellsToWrite > 0; cellsToWrite--)
	    if (translatedBuffer[charactersWritten + cellsToWrite] == ' ')
	      break;
	  if (cellsToWrite <= 0)
	    {
	      wordTooLong = 1;
	      cellsToWrite = 0;
	    }
	  if (ud->hyphenate)
	    if (hyphenatex (charactersWritten + cellsToWrite,
			    charactersWritten + availableCells -
			    minSpaceAfterNotLastWord,
			    &breakAt, &insertHyphen)) {
	      cellsToWrite = breakAt - charactersWritten;
	      wordTooLong = 0;
	    }
	  if (wordTooLong)
	    {
	      cellsToWrite = availableCells - minSpaceAfterNotLastWord - 1;
	      if (cellsToWrite <= 0)
		cellsToWrite = 1;
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
      if ((breakAt && insertHyphen) || wordTooLong)
	if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
	  return 0;
      if (charactersWritten < untilLastWord)
	finishLine ();
      else
	{
	  availableCells -= cellsToWrite;
	  if (availableCells <= minSpaceAfterNotLastWord)
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
	  leadingBlanks = ud->style_left_margin + ud->style_first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = ud->style_left_margin;
      if (leadingBlanks < 0)
	leadingBlanks = 0;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      lastWordNewRule = 1;
    }
  else
    {
      insertCharacters (blanks, 1);
      availableCells--;
    }
  charactersWritten = lastWord;
  while (((numbersStart - 1) - charactersWritten) >
	 (availableCells - minSpaceAfterLastWord - numbersLength))
    {
      int breakAt = 0;
      int insertHyphen = 0;
      if (ud->hyphenate)
	{
	  if (((numbersStart - 1) - charactersWritten) >
	      (availableCells - minSpaceAfterNotLastWord))
	    hyphenatex (charactersWritten,
			charactersWritten + availableCells -
			minSpaceAfterNotLastWord,
			&breakAt, &insertHyphen);
	  else
	    hyphenatex (charactersWritten, numbersStart - 1,
			&breakAt, &insertHyphen);
	}
      if (breakAt || lastWordNewRule)
	{
	  if (breakAt)
	    cellsToWrite = breakAt - charactersWritten;
	  else
	    {
	      if (((numbersStart - 1) - charactersWritten) >
		  (availableCells - minSpaceAfterNotLastWord))
		cellsToWrite =
		  (availableCells - minSpaceAfterNotLastWord) - 1;
	      else
		cellsToWrite = ((numbersStart - 1) - charactersWritten) - 1;
	      if (cellsToWrite <= 0)
		cellsToWrite = 1;
	    }
	  if (!insertWidechars
	      (&translatedBuffer[charactersWritten], cellsToWrite))
	    return 0;
	  charactersWritten += cellsToWrite;
	  if ((breakAt && insertHyphen) || !breakAt)
	    if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
	      return 0;
	}
      finishLine ();
      availableCells = startLine ();
      leadingBlanks = ud->style_left_margin;
      if (!insertCharacters (blanks, leadingBlanks))
	return 0;
      availableCells -= leadingBlanks;
      lastWordNewRule = 1;
      if (availableCells < (1 + minSpaceAfterLastWord + numbersLength))
	break;
    }

  if (!insertWidechars (&translatedBuffer[charactersWritten],
			(numbersStart - 1) - charactersWritten))
    return 0;
  availableCells -= (numbersStart - 1) - charactersWritten;
  if ((availableCells - numbersLength) < (1 + minGuideDots + 1))
    insertCharacters (blanks, availableCells - numbersLength);
  else
    {
      insertCharacters (blanks, 1);
      for (k = availableCells - (numbersLength + 2); k > 0; k--)
	insertCharacters (&ud->line_fill, 1);
      insertCharacters (blanks, 1);
    }
  if (!insertWidechars (&translatedBuffer[numbersStart], numbersLength))
    return 0;
  finishLine ();
  return 1;
}

static int
doCenterRight ()
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  while (charactersWritten < translatedLength)
    {
      int wordTooLong = 0;
      int leadingBlanks = 0;
      int trailingBlanks = 0;
      int breakAt = 0;
      int insertHyphen;
      availableCells = startLine ();
      if (styleSpec->status == startBody)
	{
	  leadingBlanks = ud->style_left_margin + ud->style_first_line_indent;
	  styleSpec->status = resumeBody;
	}
      else
	leadingBlanks = ud->style_left_margin;
      trailingBlanks = ud->style_right_margin;
      availableCells -= leadingBlanks;
      availableCells -= trailingBlanks;
	if(ud->style_format == centered)
	{
		if(pageNumberLength > 0)
			availableCells = ud->cells_per_line - 2*(pageNumberLength);
		else
			availableCells = ud->cells_per_line;
		availableCells -= leadingBlanks;
		availableCells -= trailingBlanks;
		
		if(translatedLength - charactersWritten <= availableCells)
		{
			if(pageNumberLength > 0)
			{
				leadingBlanks += pageNumberLength + (availableCells-(translatedLength - charactersWritten))/2;
				trailingBlanks += (availableCells-(translatedLength - charactersWritten))/2;
			}
			else
			{
				trailingBlanks += (availableCells-(translatedLength - charactersWritten))/2;
				leadingBlanks += (availableCells-(translatedLength - charactersWritten))/2;
			}
			
			if (!insertCharacters (blanks, leadingBlanks))
			return 0;
			
			if (!insertWidechars(
						&translatedBuffer[charactersWritten],translatedLength - charactersWritten)
					)
			return 0;
			finishLine ();
			break;
		}
	}
	else if((translatedLength - charactersWritten) < availableCells)
	{
		k = availableCells - (translatedLength - charactersWritten);
		if (ud->style_format != rightJustified)
			return 0;
		if (!insertCharacters (blanks, leadingBlanks + k))
			return 0;
		if (!insertWidechars(&translatedBuffer[charactersWritten],
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
	    wordTooLong = 1;
		if(ud->hyphenate)
			if(hyphenatex(charactersWritten+cellsToWrite,
			charactersWritten + availableCells,
			&breakAt,
			&insertHyphen)) {
				cellsToWrite = breakAt - charactersWritten;
				wordTooLong = 0;
			}
		if (wordTooLong)
		  cellsToWrite = availableCells - 1;
	}
      for (k = charactersWritten; k < (charactersWritten + cellsToWrite); k++)
	if (translatedBuffer[k] == 0xa0)	/*unbreakable space */
	  translatedBuffer[k] = 0x20;	/*space */
      if (!wordTooLong)
	{
		k = availableCells - cellsToWrite;
		if (ud->style_format == centered)
		{
			availableCells = ud->cells_per_line - 2*pageNumberLength;
			availableCells -= leadingBlanks;
			availableCells -= trailingBlanks;
			if(pageNumberLength > 0)
			{
				leadingBlanks += pageNumberLength + (availableCells-cellsToWrite)/2;
				trailingBlanks += (availableCells-cellsToWrite)/2;
			}
			else
			{
				trailingBlanks += (availableCells-cellsToWrite)/2;
				leadingBlanks += (availableCells-cellsToWrite)/2;
			}
			k=0;
		}
	}
      else
	k = 0;
      if (!insertCharacters (blanks, leadingBlanks + k))
	return 0;
      if (!insertWidechars
	  (&translatedBuffer[charactersWritten], cellsToWrite))
	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == ' ')
	charactersWritten++;
      if ((breakAt && insertHyphen) || wordTooLong)
		if (!insertDubChars (ud->lit_hyphen, strlen (ud->lit_hyphen)))
			return 0;
    finishLine ();
    }
  return 1;
}

static int
editTrans ()
{
  int translationLength;
  if (ud->needs_editing && !(ud->contents == 2) && !(ud->style_format
						     == computerCoded) &&
      ud->edit_table_name != NULL && (ud->has_math || ud->has_chem ||
				      ud->has_music))
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
      ud->in_sync = 0;
      translatedBuffer = ud->text_buffer;
    }
  else
    {
      translatedBuffer = ud->translated_buffer;
      translatedLength = ud->translated_length;
      positionsArray = ud->positions_array;
    }
  return 1;
}

static int
startStyle ()
{
/*Line or page skipping before body*/
  styleSpec->status = startBody;
  lastLineInStyle = 0;
  if (ud->format_for == utd)
    return utd_startStyle ();
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
    }
  if (style->topBoxline[0])
  {
    addBoxline(style->topBoxline, -1);
  }
  writeOutbuf ();
  ud->blank_lines = maximum (ud->blank_lines, style->lines_before);

  return 1;
}

static int
styleBody ()
{
  sem_act action = style->action;
  while (ud->translated_length > 0 &&
	 ud->translated_buffer[ud->translated_length - 1] <= 32)
    ud->translated_length--;
  if (ud->translated_length == 0)
    return 1;
  if (ud->format_for == utd)
    return utd_styleBody ();
  if (!editTrans ())
    return 0;
  if (ud->style_format != computerCoded && action != document)
    {
      int realStart;
      for (realStart = 0; realStart < translatedLength &&
	   translatedBuffer[realStart] <= 32 &&
	   translatedBuffer[realStart] != escapeChar; realStart++);
      if (realStart > 0)
	{
	  translatedBuffer = &translatedBuffer[realStart];
	  translatedLength -= realStart;
	  if (ud->in_sync)
	    positionsArray = &positionsArray[realStart];
	}
    }
  while (translatedLength > 0
	 && translatedBuffer[translatedLength - 1] <= 32 &&
	 translatedBuffer[translatedLength - 1] != escapeChar)
    translatedLength--;
  if (translatedLength <= 0)
    {
      ud->translated_length = ud->sync_text_length = 0;
      ud->in_sync = ud->hyphenate;
      return 1;
    }
  if (!ud->paragraphs)
    {
      cellsWritten = 0;
      if (!insertWidechars (translatedBuffer, translatedLength))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      writeOutbuf ();
      ud->translated_length = ud->sync_text_length = 0;
      ud->in_sync = ud->hyphenate;
      return 1;
    }
  if (action == contentsheader && ud->contents != 2)
    {
      if (!fillPage ())
	{
	  addPagesToPrintPageNumber ();
	  continuePrintPageNumber ();
	}
      write_buffer (3, 0);
      ud->outbuf3_enabled = 0;
      initialize_contents ();
      start_heading (action, translatedBuffer, translatedLength);
      finish_heading (action);
      ud->text_length = ud->translated_length = ud->sync_text_length = 0;
      ud->in_sync = ud->hyphenate;
      return 1;
    }
  if (ud->contents == 1)
    {
      if (ud->braille_pages && (action == heading1 || action == heading2 ||
				action == heading3 || action == heading4 ||
				action == heading5 || action == heading6 ||
				action == heading7 || action == heading8 ||
				action == heading9 || action == heading10))
	getBraillePageString ();
      start_heading (action, translatedBuffer, translatedLength);
    }
  if(ud->endnotes && action == note && ud->endnote_stage == 1)				////
  {
	link_endnote(styleSpec->node);
	ud->text_length = ud->translated_length = ud->sync_text_length = 0;
	return 1;
  }
  else if(ud->endnotes && action == notesheader && ud->endnote_stage == 1)
  {
	set_notes_header();
	ud->text_length = ud->translated_length = ud->sync_text_length = 0;
	return 1;
  }
  else if(ud->endnotes && action == notesdescription && ud->endnote_stage == 1)
  {
	set_notes_description();
	ud->text_length = ud->translated_length = ud->sync_text_length = 0;
	return 1;
  }
  else if(!ud->endnotes && (action == note || action == notesheader || action == notesdescription))
  {
	ud->text_length = ud->translated_length = ud->sync_text_length = 0;
	return 1;
  }
  switch (ud->style_format)
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
  writeOutbuf ();
  if (ud->contents == 1)
    finish_heading (action);
  styleSpec->status = resumeBody;
  ud->translated_length = ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  return 1;
}

static int
finishStyle ()
{
/*Skip lines or pages after body*/
  if (ud->format_for == utd)
    return utd_finishStyle ();
  if (style->bottomBoxline[0])
  {
    addBoxline(style->bottomBoxline, 1);
  }
  if (ud->braille_pages)
    {
      if (style->newpage_after)
	fillPage ();
    }
  writeOutbuf ();
  ud->blank_lines = maximum (ud->blank_lines, style->lines_after);
  return 1;
}

int
write_paragraph (sem_act action, xmlNode * node)
{
  StyleType *holdStyle;
  logMessage(LOU_LOG_DEBUG, "Begin write_paragraph");
  if (!((ud->text_length > 0 || ud->translated_length > 0) &&
	ud->style_top >= 0))
  {
    logMessage(LOU_LOG_DEBUG, "Finished write_paragraph, no text translated");
    return 1;
  }
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
  if (style->format != inherit)
    ud->style_format = style->format;
  else if (ud->style_format != leftJustified &&
	   ud->style_format != rightJustified && ud->style_format != centered)
    {
      ud->style_format = leftJustified;
    }
  if (style->left_margin != -100)
    ud->style_left_margin = style->left_margin;
  if (style->right_margin != -100)
    ud->style_right_margin = style->right_margin;
  if (style->first_line_indent != -100)
    ud->style_first_line_indent = style->first_line_indent;
  styleSpec->curBrlNumFormat = ud->brl_page_num_format;
  styleSpec->curStyleFormat = ud->style_format;
  styleSpec->curLeftMargin = ud->style_left_margin;
  styleSpec->curRightMargin = ud->style_right_margin;
  styleSpec->curFirstLineIndent = ud->style_first_line_indent;
  startStyle ();
  insert_translation (ud->main_braille_table);
  styleBody ();
  end_style ();
  ud->needs_editing = 0;
  logMessage(LOU_LOG_DEBUG, "Finish write_paragraph");
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
makeParagraph ()
{
  int translationLength = 0;
  int translatedLength;
  int charactersWritten = 0;
  int pieceStart;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin makeParagraph");
  while (ud->text_length > 0 && ud->text_buffer[ud->text_length - 1] <=
	 32 && ud->text_buffer[ud->text_length - 1] != escapeChar)
    ud->text_length--;
  if (ud->text_length == 0)
    return 1;
  ud->text_buffer[ud->text_length] = 0;
  k = 0;
  while (k < ud->text_length)
    {
      if (ud->text_buffer[k] == *ud->lit_hyphen
	  && ud->text_buffer[k + 1] == 10
	  && ud->text_buffer[k + 2] != escapeChar)
	k += 2;
      if (k > translationLength)
	ud->text_buffer[translationLength] = ud->text_buffer[k];
      k++;
      translationLength++;
    }
  translatedLength = MAX_TRANS_LENGTH;
  logMessage(LOU_LOG_DEBUG, "About to perform back translation");
  if (!lou_backTranslateString (ud->main_braille_table,
				ud->text_buffer, &translationLength,
				&ud->translated_buffer[0],
				&translatedLength,
				ud->typeform, NULL, 0))
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
  writeOutbuf ();
  ud->text_length = 0;
  logMessage(LOU_LOG_DEBUG, "Finish makeParagraph");
  return 1;
}

static int
handlePrintPageNumber ()
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
discardPageNumber ()
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
back_translate_braille_string ()
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
  logMessage(LOU_LOG_DEBUG, "Begin back_translate_braille_string");
  if (ud->format_for == utd)
    return utd_back_translate_braille_string ();
  if (!start_document ())
    return 0;
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlStart, strlen (htmlStart)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      ud->output_encoding = lbu_utf8;
    }
  else
    ud->output_encoding = lbu_ascii8;
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
      writeOutbuf ();
      ud->output_encoding = lbu_ascii8;
    }
  logMessage(LOU_LOG_DEBUG, "Finish back_translate_braille_string");
  return 1;
}

int
back_translate_file ()
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int printPage = 0;
  int newPage = 0;
  widechar outbufx[BUFSIZE];
  char *htmlStart = "<html><head><title>No Title</title></head><body>";
  char *htmlEnd = "</body></html>";
  if (!start_document ())
    return 0;
  ud->outbuf = outbufx;
  ud->outbuf1_len = MAX_LENGTH;
  if (ud->back_text == html)
    {
      if (!insertCharacters (htmlStart, strlen (htmlStart)))
	return 0;
      if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
	return 0;
      ud->output_encoding = lbu_utf8;
    }
  else
    ud->output_encoding = lbu_ascii8;
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
      writeOutbuf ();
      ud->output_encoding = lbu_ascii8;
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
  saveOutlen = ud->outbuf1_len_so_far;
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
  if (ud->outbuf1_len_so_far > saveOutlen)
    ud->outbuf1_len_so_far -= strlen (ud->lineEnd);
  if (!insertCharacters ("</a>", 4))
    return 0;
  if (!insertCharacters (ud->lineEnd, strlen (ud->lineEnd)))
    return 0;
  writeOutbuf ();
  return 1;
}

int
insert_linkOrTarget (xmlNode * node, int which)
{
  fineFormat ();
  makeLinkOrTarget (node, which);
  return 1;
}

static int
addBoxline(const char *boxChar, int beforeAfter)
{
  int k;
  int availableCells = 0;
  if (ud->format_for == utd)
    return utd_addBoxline(boxChar, beforeAfter);
  logMessage(LOU_LOG_DEBUG, "Begin addBoxline");
  logMessage(LOU_LOG_DEBUG, "styleSpec->node->name=%s", styleSpec->node->name);
  availableCells = startLine();
  while (availableCells != ud->cells_per_line)
  {
    finishLine();
    availableCells = startLine();
  }
  logMessage(LOU_LOG_DEBUG, "availableCells=%d", availableCells);
  for (k = 0; k < availableCells; k++)
  {
    ud->outbuf1[k+ud->outbuf1_len_so_far] = *boxChar;
  }
  ud->outbuf1_len_so_far += availableCells;
  cellsWritten += availableCells;
  finishLine();
  logMessage(LOU_LOG_DEBUG, "Finished addBoxline");
  return 1;
}

static int
utd_addBoxline(const char *boxChar, int beforeAfter)
{
  int k;
  int availableCells = 0;
  int inlen = CHARSIZE * (ud->cells_per_line);
  xmlNode *tmpBrlNode;
  widechar wTmpBuf = (widechar)boxChar[0];
  widechar *lineBuf;
  // Make sure that styleSpec relates to a node
  if (styleSpec->node == NULL)
    return 0;
  logMessage(LOU_LOG_DEBUG, "Begin utd_addBoxline");
  // We should catch the current brlNode so we can restore afterwards
  tmpBrlNode = brlNode;
  // Create the brl node for the boxline
  brlNode = xmlNewNode(NULL, (const xmlChar *)"brl");
  // Find a complete blank line
  availableCells = utd_startLine();
  while (availableCells != ud->cells_per_line)
  {
    utd_finishLine(0, 0);
    availableCells = utd_startLine();
  }
  // Create the line of characters
  lineBuf = malloc(inlen);
  for (k = 0; k < availableCells; k++)
  {
    lineBuf[k] = wTmpBuf;
  }
  inlen = availableCells;
  xmlSetProp(brlNode, (const xmlChar *)"type", (const xmlChar *)"brlonly");
  makeDotsTextNode(brlNode, lineBuf, inlen, 1);
  free(lineBuf);
  if (styleSpec->node->children && beforeAfter == -1)
  {
    brlNode = xmlAddPrevSibling(styleSpec->node->children, brlNode);
  }
  else if (styleSpec->node->children && beforeAfter == 1)
  {
    brlNode = xmlAddNextSibling(styleSpec->node->last, brlNode);
  }
  else
  {
    brlNode = xmlAddChild(styleSpec->node, brlNode);
  }
  utd_finishLine(0, 0);
  // Restore original brlNode
  brlNode = tmpBrlNode;
  logMessage(LOU_LOG_DEBUG, "Finish utd_addBoxline");
  return 1;
}

int
do_newpage ()
{
  fineFormat ();
  if (ud->lines_on_page > 0)
    fillPage ();
  return 1;
}

int
do_blankline ()
{
  fineFormat ();
  makeBlankLines (1);
  return 1;
}

int
do_softreturn ()
{
  fineFormat ();
  return 1;
}

int
do_righthandpage ()
{
  do_newpage ();
  if (ud->braille_pages && ud->interpoint && !(ud->braille_page_number & 1))
    fillPage ();
  return 1;
}

int
do_pagenum ()
{
  if (ud->page_separator)
    fineFormat ();
  if (!ud->merge_unnumbered_pages)
    {
      ud->print_page_number[0] = '_';
      ud->print_page_number[1] = 0;
      if (!ud->page_separator_number_first[0] || ud->ignore_empty_pages)
	widestrcpy (ud->page_separator_number_first, ud->print_page_number);
    }
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
  logMessage(LOU_LOG_DEBUG, "Begin start_style");
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
      ud->style_format = styleSpec->curStyleFormat;
      ud->style_left_margin = styleSpec->curLeftMargin;
      ud->style_right_margin = styleSpec->curRightMargin;
      ud->style_first_line_indent = styleSpec->curFirstLineIndent;
      styleBody ();
    }
  ud->needs_editing = 0;
  if (ud->style_top < (STACKSIZE - 2))
    ud->style_top++;
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style = curStyle;
  styleSpec->status = beforeBody;
  styleSpec->node = node;
  if (style->brlNumFormat != normal)
    ud->brl_page_num_format = style->brlNumFormat;
  if (style->format != inherit)
    ud->style_format = style->format;
  else if (ud->style_format != leftJustified &&
	   ud->style_format != rightJustified && ud->style_format != centered)
    {
      ud->style_format = leftJustified;
    }
  if (style->left_margin != -100)
    ud->style_left_margin = style->left_margin;
  if (style->right_margin != -100)
    ud->style_right_margin = style->right_margin;
  if (style->first_line_indent != -100)
    ud->style_first_line_indent = style->first_line_indent;
  styleSpec->curBrlNumFormat = ud->brl_page_num_format;
  styleSpec->curStyleFormat = ud->style_format;
  styleSpec->curLeftMargin = ud->style_left_margin;
  styleSpec->curRightMargin = ud->style_right_margin;
  styleSpec->curFirstLineIndent = ud->style_first_line_indent;
  startStyle ();
  if (node && !node->children)
    return 1;
  styleSpec->status = startBody;
  logMessage(LOU_LOG_DEBUG, "Finish start_style");
  return 1;
}

int
end_style ()
{
  logMessage(LOU_LOG_DEBUG, "Begin end_style");
  styleSpec = &ud->style_stack[ud->style_top];
  style = styleSpec->style;
  ud->brl_page_num_format = styleSpec->curBrlNumFormat;
  ud->style_format = styleSpec->curStyleFormat;
  ud->style_left_margin = styleSpec->curLeftMargin;
  ud->style_right_margin = styleSpec->curRightMargin;
  ud->style_first_line_indent = styleSpec->curFirstLineIndent;
  if (!(styleSpec->node && !styleSpec->node->children))
    {
      insert_translation (ud->main_braille_table);
      if (style->runningHead)
	set_runninghead_string (ud->translated_buffer, ud->translated_length);
      styleBody ();
    }
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
  ud->style_format = styleSpec->curStyleFormat;
  ud->style_left_margin = styleSpec->curLeftMargin;
  ud->style_right_margin = styleSpec->curRightMargin;
  ud->style_first_line_indent = styleSpec->curFirstLineIndent;
  ud->needs_editing = 0;
  logMessage(LOU_LOG_DEBUG, "Finish end_style");
  return 1;
}

/* Routines for Unified Tactile Ducument Markup Language (UTDML) */

#define SPACE LOU_DOTS
/* Dot patterns must include LOU_DOTS and be enclosed in parentheses.*/
#define NBSP (LOU_DOTS | LOU_DOT_10)
#define CR (LOU_DOTS | LOU_DOT_11)
#define HYPHEN (LOU_DOTS | LOU_DOT_3 | LOU_DOT_6)
#define ESCAPE (LOU_DOTS | LOU_DOT_11 | LOU_DOT_1)
#define CDOTS (LOU_DOTS | LOU_DOT_1 | LOU_DOT_4)
#define EDOTS (LOU_DOTS | LOU_DOT_1 |  LOU_DOT_5)
#define RDOTS (LOU_DOTS | LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5)

static const char *currentTable;
static char currentTableName[MAXNAMELEN];
static int firstPage;
static int postponedStart;
static int *indices;
static int *backIndices;
static widechar *backBuf;
static int backLength;

static char *utilStringBuf;
static int lineWidth;
static int cellsToWrite;

static int
utd_start ()
{
  ud->braille_pages = 1;
  firstPage = 1;
  postponedStart = 0;
  brlContent = (xmlChar *) ud->outbuf1;
  maxContent = ud->outbuf1_len * CHARSIZE;
  utilStringBuf = (char *) ud->text_buffer;
  currentTable = firstTableName;
  brlNode = firstBrlNode = prevBrlNode = NULL;
  ud->louis_mode = dotsIO;
  indices = NULL;
  if (!(ud->mode & notSync))
    indices = ud->positions_array;
  backIndices = NULL;
  backBuf = NULL;
  backLength = 0;
  lineWidth = ud->normal_line;
  return 1;
}

static xmlParserCtxt *ctxt;

static xmlNode *
makeDaisyDoc ()
{
  xmlDoc *doc;
  xmlNode *newNode;
  xmlNode *rootNode;
  xmlNode *bodyNode;
  xmlNode *sectionNode;
  static const char *starter =
    "<?xml version='1.0' encoding='UTF-8' standalone='yes'?><document/>";
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
  newNode = xmlNewNode (NULL, (xmlChar *) "body");
  bodyNode = xmlAddChild (rootNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "section");
  sectionNode = xmlAddChild (bodyNode, newNode);
  ud->doc = doc;
  return sectionNode;
}

static int
processDaisyDoc ()
{
  /* This function complements makeDaisyDoc. */
  xmlNode *rootElement = NULL;
  int haveSemanticFile;
  if (ud->doc == NULL)
    {
      logMessage (LOU_LOG_FATAL, "Document could not be processed");
      return 0;
    }
  rootElement = xmlDocGetRootElement (ud->doc);
  if (rootElement == NULL)
    {
      logMessage (LOU_LOG_FATAL, "Document is empty");
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
freeDaisyDoc ()
{
/* Call this function if you have used w
makeDaisyDoc but not 
* procesDaisyDoc. */
  if (ud->doc == NULL)
    {
      logMessage (LOU_LOG_FATAL, "Document could not be processed");
      return 0;
    }
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
  if (ch > 127 && ud->input_encoding == lbu_ascii8)
    {
      buf[pos++] = 0xc3;
      buf[pos++] = ch & 0x3f;
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

int
utd_transcribe_text_string ()
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
  ud->input_encoding = lbu_utf8;
  return 1;
}

int
utd_transcribe_text_file ()
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
  ud->input_encoding = lbu_utf8;
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
backTranslateBlock (xmlNode * curBlock, xmlNode * curBrl)
{
  xmlNode *child;
  xmlNode *backText;
  xmlNode *addBrl;
  int translationLength;
  int translatedLength;
  int goodTrans;
  int pos;
  int k, kk;
  logMessage(LOU_LOG_DEBUG, "Begin backTranslateBlock");
  if (curBlock == NULL || curBrl == NULL)
    return 1;
  ud->text_length = 0;
  child = curBrl->children;
  while (child)
    {
      if (child->type == XML_TEXT_NODE)
	insert_utf8 (child->content);
      child = child->next;
    }
  if (ud->text_length > backLength)
    {
      backLength = ud->text_length;
      backBuf = ud->outbuf1;
      backIndices = NULL;
      if (!(ud->mode & notSync))
	backIndices = ud->positions_array;
    }
  translationLength = ud->text_length;
  translatedLength = 4 * backLength;
  goodTrans = lou_backTranslate (ud->main_braille_table, ud->text_buffer,
				 &translationLength,
				 backBuf, &translatedLength, NULL, NULL,
				 backIndices, NULL, NULL, 0);
  if (!goodTrans)
    {
      translatedLength = translationLength;
      lou_dotsToChar (ud->main_braille_table, ud->text_buffer, backBuf,
		      translatedLength, 0);
    }
  pos = 0;
  for (k = 0; k < translatedLength; k++)
    {
      widechar ch = backBuf[k];
      unsigned char *utf8str = wcCharToUtf8 (ch);
      for (kk = 0; utf8str[kk]; kk++)
	utilStringBuf[pos++] = utf8str[kk];
    }
  utilStringBuf[pos] = 0;
  backText = xmlNewText ((xmlChar *) utilStringBuf);
  xmlAddChild (curBlock, backText);
  addBrl = xmlAddChild (curBlock, curBrl);
  if (!goodTrans)
    {
      if (!(ud->mode & notSync))
	xmlNewProp (addBrl, (xmlChar *) "index", (xmlChar *) "-1");
      return 1;
    }
  kk = 0;
  if (!(ud->mode & notSync))
    {
      for (k = 0; k < translationLength; k++)
	{
	  char posx[MAXNUMLEN];
	  int posxLen = sprintf (posx, "%d ", backIndices[k]);
	  strcpy (&utilStringBuf[kk], posx);
	  kk += posxLen;
	}
      utilStringBuf[--kk] = 0;
      xmlNewProp (addBrl, (xmlChar *) "index", (xmlChar *) utilStringBuf);
    }
  logMessage(LOU_LOG_DEBUG, "Finish backTranslateBlock");
  return 1;
}

static int
makeDotsTextNode (xmlNode * node, const widechar * content, int length,
		  int kind)
{
  xmlNode *textNode;
  int inlen, outlen;
  logMessage(LOU_LOG_DEBUG, "Begin makeDotsTextNode");
  if (length <= 0)
    return 1;
  if (ud->mode & notUC)
    {
      int k;
      if (kind)
	memcpy (ud->outbuf1, content, length * CHARSIZE);
      else
        {
          widechar *tmpContent = malloc(sizeof(widechar) * length);
          if (tmpContent)
            {
              memcpy(tmpContent, content, sizeof(widechar) * length);
              lou_dotsToChar (currentTable, tmpContent, ud->outbuf1, length, 0);
              free(tmpContent);
            }
        }
      inlen = 0;
      for (k = 0; k < length; k++)
	ud->text_buffer[inlen++] = ud->outbuf1[k];
    }
  else
    {
      if (kind)
	lou_charToDots (currentTable, content,
			ud->text_buffer, length, dotsIO);
      else
	memcpy (ud->text_buffer, content, length * CHARSIZE);
      if (!(ud->mode & louisDots))
	for (inlen = 0; inlen < length; inlen++)
	  ud->text_buffer[inlen] = (ud->text_buffer[inlen] & 0x00ff) | 0x2800;
      else
	inlen = length;
    }
  outlen = maxContent;
  wc_string_to_utf8 (ud->text_buffer, &inlen, brlContent, &outlen);
  logMessage(LOU_LOG_DEBUG, "brlContent=%s", brlContent);
  textNode = xmlNewText (brlContent);
  xmlAddChild (node, textNode);
  logMessage(LOU_LOG_DEBUG, "Finished makeDotsTextNode");
  return 1;
}

static xmlNode *addBlock;

static int
formatBackBlock ()
{
  xmlNode *newBlock;
  xmlNode *curBrl;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin formatBackBlock");
  if (ud->translated_length <= 0)
    return 1;
  newBlock = xmlNewNode (NULL, (xmlChar *) "p");
  curBrl = xmlNewNode (NULL, (xmlChar *) "brl");
  makeDotsTextNode (curBrl, ud->translated_buffer, ud->translated_length, 1);
  ud->translated_length = ud->sync_text_length = 0;
  ud->in_sync = 1;
  backTranslateBlock (xmlAddChild (addBlock, newBlock), curBrl);
  logMessage(LOU_LOG_DEBUG, "Finish formatBackBlock");
  return 1;
}

int
utd_back_translate_file ()
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  ud->main_braille_table = ud->contracted_table_name;
  if (!lou_getTable (ud->main_braille_table))
    return 0;
  ud->output_encoding = lbu_utf8;
  utd_start ();
  addBlock = makeDaisyDoc ();
  ud->translated_length = ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  while ((ch = fgetc (ud->inFile)) != EOF)
    {
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
	  formatBackBlock ();
	  leadingBlanks = 0;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->translated_length >= MAX_LENGTH)
	formatBackBlock ();
      if (ch >= 32)
	ud->translated_buffer[ud->translated_length++] = ch;
      ud->in_sync = 0;
    }
  formatBackBlock ();
  ud->text_length = ud->translated_length = ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  utd_finish ();
  freeDaisyDoc ();
  return 1;
}

int
utd_back_translate_braille_string ()
{
  int ch;
  int ppch = 0;
  int pch = 0;
  int leadingBlanks = 0;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin utd_back_trranslate_braille_string");
  ud->main_braille_table = ud->contracted_table_name;
  if (!lou_getTable (ud->main_braille_table))
    return 0;
  ud->output_encoding = lbu_utf8;
  utd_start ();
  addBlock = makeDaisyDoc ();
  for (k = 0; k < ud->inlen; k++)
    {
      ch = ud->inbuf[k];
      if (ch == 13)
	continue;
      if (pch == 10 && ch == 32)
	{
	  leadingBlanks++;
	  continue;
	}
      if (ch == '[' || ch == '\\' || ch == '^' || ch == ']' || ch == '@'
	  || (ch >= 'A' && ch <= 'Z'))
	ch |= 32;
      if (pch == 10 && (ch == 10 || leadingBlanks > 1))
	{
	  formatBackBlock ();
	  leadingBlanks = 0;
	}
      if (ch == 10)
	leadingBlanks = 0;
      ppch = pch;
      pch = ch;
      if (ud->translated_length >= MAX_LENGTH)
	formatBackBlock ();
      if (ch >= 32)
	ud->translated_buffer[ud->translated_length++] = ch;
      ud->in_sync = 0;
    }
  formatBackBlock ();
  ud->text_length = ud->translated_length = ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  utd_finish ();
  freeDaisyDoc ();
  logMessage(LOU_LOG_DEBUG, "Finish utd_back_translate_braille_string");
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
  makeDotsTextNode (node, charBuf, length, 1);
  return 1;
}

static int
makeBrlOnlyNode ()
{
  xmlNode *newNode;
  newNode = xmlNewNode (NULL, (xmlChar *) "span");
  xmlNewProp (newNode, (xmlChar *) "class", (xmlChar *) "brlonly");
  parentOfBrlOnlyNode = xmlAddChild (brlNode, newNode);
  newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  brlOnlyNode = xmlAddChild (parentOfBrlOnlyNode, newNode);
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
      if ((dots & (LOU_DOT_7 | LOU_DOT_8)))
	lineWidth = ud->wide_line;
      if (dots == NBSP)
	text[k] = SPACE;
    }
  return 1;
}

static int
insertTextFragment (widechar * content, int length)
{
  logMessage(LOU_LOG_DEBUG, "Begin insertTextFragment");
  if (length <= 0)
    return 1;
  checkTextFragment (content, length);
  makeDotsTextNode (brlNode, content, length, 0);
  return 1;
}

typedef enum
{
  topOfPage,
  lastLine,
  bottomOfPage,
  nearBottom,
  midPage
} PageStatus;

static PageStatus
checkPageStatus ()
{
  int remaining;
  logMessage(LOU_LOG_DEBUG, "Begin checkPageStatus");
  if (ud->vert_line_pos == ud->page_top || ud->lines_on_page == 0)
  {
    logMessage(LOU_LOG_DEBUG, "Finish checkPageStatus: return=topOfPage");
    return topOfPage;
  }
  remaining = ud->page_bottom - ud->vert_line_pos;
  if (remaining < ud->normal_line || ud->lines_on_page >= ud->lines_per_page)
  {
    logMessage(LOU_LOG_DEBUG, "Finish checkPageStatus: return=bottomOfPage");
    return bottomOfPage;
  }
  if ((ud->lines_on_page + 1) >= ud->lines_per_page || remaining ==
      ud->normal_line)
  {
    logMessage(LOU_LOG_DEBUG, "Finish checkPageStatus: return=lastLine");
    return lastLine;
  }
  if (remaining > (2 * ud->normal_line) && remaining < (3 * ud->normal_line))
  {
    logMessage(LOU_LOG_DEBUG, "Finish checkPageStatus: return=nearBottom");
    return nearBottom;
  }
  logMessage(LOU_LOG_DEBUG, "Finish checkPageStatus: return=midPage");
  return midPage;
}

/* Handle and generate short strings that appear only in braille. */

typedef struct
{
  widechar origText[MAXNAMELEN];
  widechar transText[MAXNAMELEN];
  int origTextLength;
  int transTextLength;
  widechar prefixedOrigText[MAXNAMELEN];
  widechar prefixedTransText[MAXNAMELEN];
  int prefixedOrigTextLength;
  int prefixedTransTextLength;
} ShortBrlOnlyStrings;

static int
setOrigTextChar (ShortBrlOnlyStrings * sbstr, unsigned char *inStr, int
		 length)
{
  for (; length >= 0 && inStr[length - 1] <= 32; length--);
  if (length <= 0)
    return 0;
  sbstr->origTextLength = MAXNAMELEN - 4;
  utf8_string_to_wc (inStr, &length, sbstr->origText, &sbstr->origTextLength);
  sbstr->transTextLength = 0;
  sbstr->prefixedOrigTextLength = 0;
  sbstr->prefixedTransTextLength = 0;
  return 1;
}

static int
setOrigTextWidechar (ShortBrlOnlyStrings * sbstr, widechar * inStr,
		     int length)
{
  if (length <= 0)
    return 0;
  if (length >= MAXNAMELEN)
    length = MAXNAMELEN - 4;
  memcpy (sbstr->origText, inStr, length * CHARSIZE);
  sbstr->origTextLength = length;
  sbstr->transTextLength = 0;
  sbstr->prefixedOrigTextLength = 0;
  sbstr->prefixedTransTextLength = 0;
  return 1;
}

static int
translateShortBrlOnly (ShortBrlOnlyStrings * sbstr)
{
  int translationLength = sbstr->origTextLength;
  int translatedLength = MAXNAMELEN - 4;
  if (!lou_translateString (firstTableName, sbstr->origText,
			    &translationLength,
			    sbstr->transText, &translatedLength, NULL, NULL,
			    dotsIO))
    return 0;
  for (; sbstr->transText[translatedLength - 1] == SPACE; translatedLength--);
  sbstr->transText[translatedLength] = 0;
  sbstr->transTextLength = translatedLength;
  return 1;
}

static int
addBrlOnly (xmlNode * node, ShortBrlOnlyStrings * sbstr)
{
  int wcLength = sbstr->prefixedOrigTextLength;
  xmlChar buf[2 * MAXNAMELEN];
  int utf8Length = sizeof (buf) - 4;
  makeDotsTextNode (node, sbstr->prefixedTransText,
		    sbstr->prefixedTransTextLength, 0);
  wc_string_to_utf8 (sbstr->prefixedOrigText, &wcLength, buf, &utf8Length);
  xmlAddPrevSibling (node, xmlNewText (buf));
  return 1;
}

static int
addPrefixes (ShortBrlOnlyStrings * sbstr, widechar dots, widechar
	     character, int howMany)
{
  int k;
  int kk = 0;
  for (k = 0; k < howMany && k < MAXNAMELEN; k++)
    sbstr->prefixedTransText[k] = dots;
  for (; k < MAXNAMELEN; k++)
    {
      sbstr->prefixedTransText[k] = sbstr->transText[kk++];
      if (kk > sbstr->transTextLength)
	break;
    }
  sbstr->prefixedTransTextLength = k;
  kk = 0;
  for (k = 0; k < howMany && k < MAXNAMELEN; k++)
    sbstr->prefixedOrigText[k] = character;
  for (; k < MAXNAMELEN; k++)
    {
      sbstr->prefixedOrigText[k] = sbstr->origText[kk++];
      if (kk > sbstr->origTextLength)
	break;
    }
  sbstr->prefixedOrigTextLength = k;
  return 1;
}

static int
addSpaces (ShortBrlOnlyStrings * sbstr, int howMany)
{
  addPrefixes (sbstr, SPACE, ' ', howMany);
  memcpy (sbstr->origText, sbstr->prefixedOrigText,
	  sbstr->prefixedOrigTextLength * CHARSIZE);
  memcpy (sbstr->transText, sbstr->prefixedTransText,
	  sbstr->prefixedTransTextLength * CHARSIZE);
  sbstr->origTextLength = sbstr->prefixedOrigTextLength;
  sbstr->transTextLength = sbstr->prefixedTransTextLength;
  return 1;
}

static int utd_fillPage ();
static int makeNewline (xmlNode * parent, int start);
static ShortBrlOnlyStrings pageNumber;

static int
insertPageNumber (int howMany)
{
  if (howMany < 0)
    howMany = 1;
  addPrefixes (&pageNumber, SPACE, ' ', howMany);
  makeBrlOnlyNode ();
  if (!addBrlOnly (brlOnlyNode, &pageNumber))
    return 0;
  return 1;
}

static int utd_fillPage ();

static int
utd_makePageSeparator (xmlNode *node, char *printPageNumber, int length)
{
  ShortBrlOnlyStrings sb;
  int k, kk;
  char setup[MAXNUMLEN];
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  PageStatus curPageStatus = checkPageStatus ();
  if (!ud->print_pages || !*printPageNumber)
    return 1;
  strcpy (setup, "-");
  if (!(printPageNumber[0] >= '0' && printPageNumber[0] <= '9'))
    strcat (setup, ud->letsign);
  kk = strlen (setup);
  for (k = 0; k < length; k++)
    setup[kk++] = printPageNumber[k];
  setup[kk] = 0;
  length = strlen (setup);
  memset (&sb, 0, sizeof (sb));
  setOrigTextChar (&sb, setup, length);
  memcpy (ud->print_page_number, sb.origText, sb.origTextLength * CHARSIZE);
  ud->print_page_number[sb.origTextLength] = 0;
  translateShortBrlOnly (&sb);
  if (curPageStatus == topOfPage)
    return 1;
  brlNode = xmlAddNextSibling (node, newNode);
  addPrefixes (&sb, HYPHEN, '-', ud->cells_per_line - sb.transTextLength);
  ud->print_page_number[0] = 'a';
  if (curPageStatus == nearBottom)
    {
      utd_fillPage ();
      return 1;
    }
  makeBrlOnlyNode ();
  makeNewline (brlOnlyNode, 0);
  addBrlOnly (brlOnlyNode, &sb);
  return 1;
}

static void
utd_pagebreak (xmlNode * node, char *printPageNumber, int length)
{
  utd_makePageSeparator (node, printPageNumber, length);
}

static int
utd_getBraillePageString ()
{
  char brlPageString[40];
  if (!ud->number_braille_pages)
    {
      pageNumber.transTextLength = 0;
      return 1;
    }
  switch (ud->cur_brl_page_num_format)
    {
    case blank:
      return 1;
    default:
    case normal:
      sprintf (brlPageString, "%d", ud->braille_page_number);
      break;
    case p:
      sprintf (brlPageString, "p%d", ud->braille_page_number);
      break;
    case roman:
      strcpy (brlPageString, ud->letsign);
      strcat (brlPageString, makeRomanNumber (ud->braille_page_number));
      break;
    }
  setOrigTextChar (&pageNumber, brlPageString, strlen (brlPageString));
  translateShortBrlOnly (&pageNumber);
  addSpaces (&pageNumber, 3);
  return 1;
}

static int
utd_getPrintPageString ()
{
  widechar printPageString[40];
  int k;
  for (k = 0; ud->print_page_number[k]; k++)
    printPageString[k] = ud->print_page_number[k];
  setOrigTextWidechar (&pageNumber, printPageString, k);
  translateShortBrlOnly (&pageNumber);
  addSpaces (&pageNumber, 3);
  ud->print_page_number[0]++;
  return 1;
}

static int
utd_getPageNumber ()
{
  int k;
  PageStatus curPageStatus;
  int braillePageNumber = 0;
  int printPageNumber = 0;
  memset (&pageNumber, 0, sizeof (pageNumber));
  curPageStatus = checkPageStatus ();
  if (curPageStatus == midPage)
    return 1;
  if (curPageStatus == topOfPage)
    {
      if (ud->print_pages && ud->print_page_number_at
	  && ud->print_page_number[0] != '_')
	{
	  printPageNumber = 1;
	}
      if (!ud->braille_page_number_at && ud->cur_brl_page_num_format != blank)
	{
	  braillePageNumber = 1;
	}
    }
  else if (curPageStatus == lastLine)
    {
      if (ud->print_pages && !ud->print_page_number_at
	  && ud->print_page_number[0] != '_')
	{
	  printPageNumber = 1;
	}
      if (ud->braille_page_number_at && ud->cur_brl_page_num_format != blank)
	{
	  braillePageNumber = 1;
	}
    }
  if (ud->interpoint && !(ud->braille_page_number & 1))
    braillePageNumber = 0;
  if (printPageNumber)
    utd_getPrintPageString ();
  if (braillePageNumber)
    utd_getBraillePageString ();
  return 1;
}

static int
utd_centerHeadFoot (widechar * toCenter, int length)
{
  ShortBrlOnlyStrings sb;
  int leadingBlanks;
  int trailingBlanks;
  int numCells = ud->cells_per_line - pageNumberLength;
  setOrigTextWidechar (&sb, toCenter, length);
  translateShortBrlOnly (&sb);
  if (sb.transTextLength > numCells)
    sb.transTextLength = numCells - 4;
  leadingBlanks = (numCells - sb.transTextLength) / 2;
  trailingBlanks = numCells - leadingBlanks - sb.transTextLength;
  addPrefixes (&sb, SPACE, ' ', leadingBlanks);
  addPrefixes (&pageNumber, SPACE, ' ', trailingBlanks);
  return 1;
}

/*End of handling and generating braille-only strings*/

int
hasAttrValue (xmlNode * node, char *attrName, char *value)
{
  char values[MAXNAMELEN];
  xmlChar *allValues;
  int k;
  int prevValue = 0;
  if (node == NULL)
    return 0;
  allValues = xmlGetProp (node, (xmlChar *) attrName);
  if (allValues == NULL)
    return 0;
  strcpy (values, allValues);
  for (k = 0; values[k]; k++)
    if (values[k] == ' ')
      {
	values[k] = 0;
	if (strcmp (&values[prevValue], value) == 0)
	  return 1;
	prevValue = k + 1;
      }
  if (strcmp (&values[prevValue], value) == 0)
    return 1;
  return 0;
}

static int
assignTranslations ()
{
  int nextSegment = 0;
  int curPos = 0;
  xmlNode *curBrlNode;
  if (firstBrlNode == NULL)
    return 0;
  curBrlNode = firstBrlNode;
  while (curPos < translatedLength && curBrlNode != NULL &&
	 nextSegment < translatedLength)
    {
      if (translatedBuffer[curPos] == LOU_ENDSEGMENT || nextSegment == 0)
	{
	  int nextPos = nextSegment;
	  while (translatedBuffer[nextPos] != LOU_ENDSEGMENT && nextPos <
		 translatedLength)
	    nextPos++;
	  makeDotsTextNode (curBrlNode, &translatedBuffer[nextSegment],
			    nextPos - nextSegment, 0);
	  if (curBrlNode && curBrlNode->_private != NULL)
	    curBrlNode = curBrlNode->_private;
	  curPos = nextPos;
	}
      nextSegment = curPos + 1;
    }
  return 1;
}

static int
makeNewpage (xmlNode * parent)
{
  char number[MAXNUMLEN];
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newpage");
  logMessage(LOU_LOG_DEBUG, "Begin makeNewpage");
  sprintf (number, "%d", ud->braille_page_number);
  xmlNewProp (newNode, (xmlChar *) "brlnumber", (xmlChar *) number);
  newpageNode = xmlAddChild (parent, newNode);
  ud->lines_on_page = 0;
  logMessage(LOU_LOG_DEBUG, "Finish makeNewpage");
  return 1;
}

static int
makeNewline (xmlNode * parent, int start)
{
  char position[MAXNUMLEN];
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newline");
  sprintf (position, "%d,%d", (ud->cell_width * start +
			       ud->page_left), ud->vert_line_pos);
  xmlNewProp (newNode, (xmlChar *) "xy", (xmlChar *) position);
  xmlAddChild (parent, newNode);
  ud->vert_line_pos += lineWidth;
  lineWidth = ud->normal_line;
  ud->lines_on_page++;
  return 1;
}

static xmlNode *startNode;

static int
assignIndices (xmlNode * startNode, int startPos)
{
  int nextSegment = startPos;
  int firstIndex;
  int curPos = startPos;
  xmlNode *curBrlNode;
  logMessage(LOU_LOG_DEBUG, "Begin assignIndices");
  if (indices == NULL)
    return 1;
  if (startNode == NULL)
    return 0;
  curBrlNode = startNode;
  firstIndex = indices[startPos];
  while (curPos < ud->translated_length && curBrlNode != NULL &&
	 nextSegment < ud->translated_length)
    {
      if (ud->translated_buffer[curPos] == LOU_ENDSEGMENT || nextSegment ==
	  startPos)
	{
	  int indexPos = nextSegment;
	  int kk = 0;
	  if (ud->translated_buffer[curPos] == LOU_ENDSEGMENT)
	    firstIndex = indices[curPos + 1];
	  while (ud->translated_buffer[indexPos] != LOU_ENDSEGMENT &&
		 indexPos < ud->translated_length)
	    {
	      char pos[MAXNUMLEN];
	      int posLen;
	      posLen = sprintf (pos, "%d ", indices[indexPos] - firstIndex);
	      strcpy (&utilStringBuf[kk], pos);
	      kk += posLen;
	      indexPos++;
	    }
          logMessage(LOU_LOG_DEBUG, "indexPos=%d", indexPos);
	  utilStringBuf[--kk] = 0;
          logMessage(LOU_LOG_DEBUG, "utilStringBuf=%s", utilStringBuf);
	  if (xmlGetProp (curBrlNode, (xmlChar *) "index") == NULL && indexPos > 0)
	    xmlNewProp (curBrlNode, (xmlChar *) "index", (xmlChar *)
			utilStringBuf);
	  if (curBrlNode && curBrlNode->_private != NULL)
	    curBrlNode = curBrlNode->_private;
	  curPos = indexPos;
	}
      nextSegment = curPos + 1;
    }
  logMessage(LOU_LOG_DEBUG, "Finish assignIndices");
  return 1;
}

static int
utd_insert_translation (const char *table)
{
  int translationLength;
  int translatedLength;
  int oldUdTranslatedLength = ud->translated_length;
  int k;
  int *setIndices;
  logMessage(LOU_LOG_DEBUG, "Begin utd_insert_translation");
  if (table != currentTable)
    {
      for (k = strlen (table); k >= 0; k--)
	if (table[k] == ud->file_separator)
	  break;
      strcpy (currentTableName, &table[k + 1]);
      xmlNewProp (brlNode, (xmlChar *) "changetable", (xmlChar
						       *) currentTableName);
      currentTable = table;
    }
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  if (indices != NULL)
    setIndices = &indices[ud->translated_length];
  else
    setIndices = NULL;
  k = lou_translate (table,
		     ud->text_buffer,
		     &translationLength,
		     &ud->
		     translated_buffer[ud->translated_length],
		     &translatedLength,
		     ud->typeform, NULL, NULL,
		     setIndices, NULL, dotsIO);
  ud->in_sync = 0;
  memset (ud->typeform, 0, sizeof (ud->typeform));
  ud->text_length = 0;
  if (!k)
    {
      logMessage (LOU_LOG_ERROR, "Could not open table %s", table);
      table = NULL;
      return 0;
    }
  if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
    ud->translated_length += translatedLength;
  else
    ud->translated_length = MAX_TRANS_LENGTH;
  assignIndices (startNode, oldUdTranslatedLength);
  logMessage(LOU_LOG_DEBUG, "Finish utd_insert_translation");
  return 1;
}

static void
utd_insert_text (xmlNode * node, int length)
{
  int charsDone;
  xmlNode *newNode;
  int outSize;
  int k;
  if (ud->text_length >= MAX_TEXT_LENGTH)
    return;
  charsDone = length;
  outSize = MAX_TEXT_LENGTH - ud->text_length;
  ud->old_text_length = ud->text_length;
  utf8ToWc (node->content, &charsDone,
	    &ud->text_buffer[ud->text_length], &outSize);
  ud->text_length += outSize;
  newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  switch (ud->stack[ud->top])
    {
    case notranslate:
      utd_insert_translation (ud->main_braille_table);
      if ((ud->translated_length + ud->text_length) > MAX_TRANS_LENGTH)
	ud->text_length = MAX_TRANS_LENGTH - ud->translated_length;
      lou_charToDots (ud->main_braille_table,
		      &ud->text_buffer[ud->old_text_length],
		      &ud->translated_buffer[ud->translated_length],
		      ud->text_length - ud->old_text_length, ud->louis_mode);
      for (k = 0; k < ud->text_length; k++)
	indices[ud->translated_length + k] = k;
      ud->translated_length += ud->text_length;
      ud->translated_buffer[ud->translated_length++] = LOU_ENDSEGMENT;
      ud->text_length = 0;
      ud->in_sync = 0;
      return;
    case pagenum:
      if (ud->print_pages)
	{
	  char printPageNumber[MAXNUMLEN + 1];
	  if (!ud->paragraphs)
	   break;
	  for (k = 0; k < outSize && k < MAXNUMLEN; k++)
	    printPageNumber[k] = ud->text_buffer[ud->old_text_length + k];
	  utd_makePageSeparator (node, printPageNumber, k);
	}
      ud->text_length = ud->old_text_length;
      return;
    default:
      setEmphasis ();
      break;
    }
  if (ud->old_text_length == 0)
    {
      startNode = xmlAddNextSibling (node, newNode);
      link_brl_node (startNode);
    }
  else
    link_brl_node (xmlAddNextSibling (node, newNode));
  ud->text_buffer[ud->text_length++] = LOU_ENDSEGMENT;
  return;
}

static int utd_finishLine (int leadingBlanks, int lengtgh);
static int
setNewlineNode ()
{
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "newline");
  newlineNode = xmlAddChild (brlNode, newNode);
  return 1;
}

static int
setNewlineProp (int horizLinePos)
{
  char position[MAXNUMLEN];
  sprintf (position, "%d,%d", horizLinePos, ud->vert_line_pos);
  if ((newlineNode != NULL) && (xmlHasProp(newlineNode, (xmlChar *) "xy") == NULL))
    xmlNewProp (newlineNode, (xmlChar *) "xy", (xmlChar *) position);
  return 1;
}

static int
postponedStartActions ()
{
  PageStatus curPageStatus = checkPageStatus ();
  if (prevStyle->action != document)
    {
      if (style->righthand_page)
	{
	  utd_fillPage ();
	  if (ud->interpoint && !(ud->braille_page_number & 1))
	    makeBlankPage ();
	}
      else if (style->newpage_before)
	utd_fillPage ();
      else
	if (style->lines_before > 0
	    && prevStyle->lines_after == 0 && curPageStatus != topOfPage)
	{
	  if (curPageStatus == nearBottom)
	    utd_fillPage ();
	  else if (!utd_makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  else
    {
      if (style->lines_before > 0 && prevStyle->lines_after == 0
	  && prevStyle->action != document && curPageStatus != topOfPage)
	{
	  if (!utd_makeBlankLines (style->lines_before, 0))
	    return 0;
	}
    }
  return 1;
}

static int
utd_startLine ()
{
  PageStatus curPageStatus;
  int availableCells = 0;
  logMessage(LOU_LOG_DEBUG, "Begin utd_startLine");
  if (firstPage)
    {
      firstPage = 0;
      makeNewpage (brlNode);
    }
  if (postponedStart)
    {
      postponedStart = 0;
      postponedStartActions ();
    }
  while (availableCells == 0)
    {
      setNewlineNode ();
      lineWidth = ud->normal_line;
      curPageStatus = checkPageStatus ();
      utd_getPageNumber ();
      if (curPageStatus == topOfPage)
	{
	  if (ud->running_head_length > 0
	      || (style->skip_number_lines && pageNumber.transTextLength > 0))
	    {
	      utd_finishLine (0, 0);
	      setNewlineNode ();
	      continue;
	    }
	  availableCells = ud->cells_per_line - pageNumber.transTextLength;
	}
      else if (curPageStatus == lastLine || curPageStatus == bottomOfPage)
	{
	  if (ud->footer_length > 0 ||
	      (style->skip_number_lines && pageNumber.transTextLength > 0))
	    {
	      utd_finishLine (0, 0);
	      continue;
	    }
	  availableCells = ud->cells_per_line - pageNumber.transTextLength;
	}
      else
	availableCells = ud->cells_per_line;
    }
  logMessage(LOU_LOG_DEBUG, "Finished utd_startLine");
  return availableCells;
}

static int
utd_finishLine (int leadingBlanks, int length)
{
  PageStatus curPageStatus;
  int cellsOnLine = 0;
  int cellsToWrite = 0;
  int k;
  int leaveBlank;
  int horizLinePos = ud->page_left + leadingBlanks * ud->cell_width;
  if (newlineNode == NULL)
    return 1;
  logMessage(LOU_LOG_DEBUG, "Begin utd_finishLine");
  cellsOnLine = leadingBlanks + length;
  for (leaveBlank = -1; leaveBlank < ud->line_spacing; leaveBlank++)
    {
      curPageStatus = checkPageStatus ();
      if (leaveBlank != -1)
	{
	  utd_startLine ();
	  ud->vert_line_pos += ud->normal_line;
	  ud->lines_on_page++;
	  setNewlineProp (0);
	}
      if (cellsOnLine > 0 && pageNumber.transTextLength > 0)
	{
	  cellsToWrite = ud->cells_per_line - pageNumber.transTextLength
	    - cellsOnLine;
	  if (!insertPageNumber (cellsToWrite))
	    return 0;
	}
      else if (curPageStatus == topOfPage)
	{
	  if (ud->running_head_length > 0)
	    utd_centerHeadFoot (ud->running_head, 
	    ud->running_head_length);
	  else
	    {
	      if (pageNumber.transTextLength)
		{
		  cellsToWrite = ud->cells_per_line -
		    pageNumber.transTextLength;
		  if (!insertPageNumber (cellsToWrite))
		    return 0;
		}
	    }
	}
      else if (curPageStatus == lastLine || curPageStatus == bottomOfPage)
	{
	  if (ud->footer_length > 0)
	    utd_centerHeadFoot (ud->footer, ud->footer_length);
	  else
	    {
	      if (pageNumber.transTextLength)
		{
		  horizLinePos = (ud->cells_per_line -
				  pageNumber.transTextLength)
		    * ud->cell_width + ud->page_left;
		  if (!insertPageNumber (0))
		    return 0;
		}
	    }
	}
    }
  setNewlineProp (horizLinePos);
  ud->vert_line_pos += lineWidth;
  ud->lines_on_page++;
  curPageStatus = checkPageStatus ();
  if (curPageStatus == bottomOfPage)
    {
      if (ud->print_page_number[0] != '_')
	{
	  unsigned char holder[MAXNUMLEN];
	  int k;
	  for (k = 0; ud->print_page_number[k]; k++)
	    holder[k] = ud->print_page_number[k];
	  holder[k] = 0;
	  xmlSetProp (newpageNode, (xmlChar *) "printnumber", holder);
	}
      ud->braille_page_number++;
      ud->vert_line_pos = ud->page_top;
      makeNewpage (brlNode);
    }
  logMessage(LOU_LOG_DEBUG, "Finish utd_finishLine");
  return 1;
}

static int
utd_doOrdinaryText ()
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
  logMessage(LOU_LOG_DEBUG, "Begin utd_doOrdinaryText");
  while (brlNode)
    {
      logMessage(LOU_LOG_DEBUG, "Finding brlNode content");
      do
	{
	  if (newLineNeeded)
	    {
	      newLineNeeded = 0;
	      if (translatedBuffer[charactersWritten] == SPACE)
		charactersWritten++;
	      origAvailableCells = availableCells = utd_startLine ();
	      if (ud->style_format == leftJustified)
		{
		  if (styleSpec->status == startBody)
		    leadingBlanks =
		      ud->style_left_margin + ud->style_first_line_indent;
		  else
		    leadingBlanks = ud->style_left_margin;
		  availableCells -= leadingBlanks;
		}
	      styleSpec->status = resumeBody;
	    }
	  lastSpace = 0;
	  for (cellsToWrite = 0;
	       cellsToWrite < availableCells
	       && (charactersWritten + cellsToWrite) <
	       translatedLength && (dots =
				    translatedBuffer[charactersWritten +
						     cellsToWrite]) !=
	       LOU_ENDSEGMENT; cellsToWrite++)
	    if (dots == SPACE)
	      lastSpace = cellsToWrite;
	  if (cellsToWrite == availableCells)
	    newLineNeeded = 1;
	  if (dots != LOU_ENDSEGMENT && lastSpace != 0)
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
      while (dots != LOU_ENDSEGMENT && charactersWritten < translatedLength);
      charactersWritten++;
      prevBrlNode = brlNode;
      brlNode = brlNode->_private;
      prevBrlNode->_private = NULL;
    }
  brlNode = prevBrlNode;	/*for utd_finishStyle */
  logMessage(LOU_LOG_DEBUG, "Finish utd_doOrdinaryText");
  return 1;
}
static int
utd_makeBlankLines (int number, int beforeAfter)
{
  int k;
  PageStatus curPageStatus;
  if (number <= 0)
    return 1;
  curPageStatus = checkPageStatus ();
  if (beforeAfter == 0 && (curPageStatus == topOfPage ||
			   prevStyle->lines_after > 0
			   || prevStyle->action == document))
    return 1;
  else if (beforeAfter == 1 && curPageStatus == nearBottom)
    return 1;
  for (k = 0; k < number; k++)
    {
      utd_startLine ();
      if (!utd_finishLine (0, 0))
	return 0;
    }
  return 1;
}

static int
utd_fillPage ()
{
  PageStatus curPageStatus = checkPageStatus ();
  logMessage(LOU_LOG_DEBUG, "Begin utd_fillPage");
  if (curPageStatus == topOfPage)
    {
      utd_startLine ();
      utd_finishLine (0, 0);
    }
  ud->vert_line_pos = ud->page_bottom - ud->normal_line;
  utd_startLine ();
  utd_finishLine (0, 0);
  logMessage(LOU_LOG_DEBUG, "Finish utd_fillPage");
  return 1;
}

static int
utd_doComputerCode ()
{
  int charactersWritten = 0;
  int cellsToWrite = 0;
  int availableCells = 0;
  int k;
  logMessage(LOU_LOG_DEBUG, "Begin utd_doComputerCode");
  while (translatedBuffer[charactersWritten] == CR)
    charactersWritten++;
  while (charactersWritten < translatedLength)
    {
      int lineTooLong = 0;
      availableCells = utd_startLine ();
      for (cellsToWrite = 0; cellsToWrite < availableCells; cellsToWrite++)
	if ((charactersWritten + cellsToWrite) >=
	    translatedLength
	    || translatedBuffer[charactersWritten + cellsToWrite] == CR)
	  break;
      if ((charactersWritten + cellsToWrite) > translatedLength)
	cellsToWrite--;
      if (cellsToWrite <= 0 && translatedBuffer[charactersWritten] != CR)
	break;
      if (cellsToWrite == availableCells
	  && translatedBuffer[charactersWritten + cellsToWrite] != CR)
	{
	  cellsToWrite = availableCells - strlen (ud->comp_hyphen);
	  lineTooLong = 1;
	}
      if (translatedBuffer[charactersWritten + cellsToWrite] == CR)
	translatedBuffer[charactersWritten + cellsToWrite] = SPACE;
      if (!insertTextFragment
	  (&translatedBuffer[charactersWritten], cellsToWrite))	return 0;
      charactersWritten += cellsToWrite;
      if (translatedBuffer[charactersWritten] == SPACE)
	charactersWritten++;
      if (lineTooLong)
	{
	  if (!utd_insertCharacters (brlNode, ud->comp_hyphen, strlen
				     (ud->comp_hyphen)))
	    return 0;
	}
      utd_finishLine (0, cellsToWrite);
    }
  return 1;
}

static int
utd_doAlignColumns ()
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
  logMessage(LOU_LOG_DEBUG, "Begin utd_doAlignColumns");
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
	      // Protect against first row being short on entries
	      if (colNum > numCols)
		{
		  numCols = colNum;
		}
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
	  // Remember end of row escape sequence needs parsing
	  for (colNum = 0; colNum <= numCols; colNum++)
	    {
	      while (rowLength < MAXROWSIZE
		     && translatedBuffer[bufPos] != ESCAPE)
		rowBuf[rowLength++] = translatedBuffer[bufPos++];
	      // Check for end of row
	      if (translatedBuffer[bufPos + 1] == RDOTS)
	        {
		  // Pad when not enough columns
		  colNum = numCols;
		}
	      bufPos += 2;
	      if (colNum < (numCols - 1))
		{
		  while (rowLength < MAXROWSIZE && rowLength <
			 colSize[colNum + 1])
		    rowBuf[rowLength++] = ' ';
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
	      if (!utd_insertCharacters (brlNode, ud->lit_hyphen, strlen
					 (ud->lit_hyphen)))
		return 0;
	    }
	  utd_finishLine (0, cellsToWrite);
	}
    }
  logMessage(LOU_LOG_DEBUG, "Finish utd_doAlignColumns");
  return 1;
}

static int
utd_doListColumns ()
{
  utd_doOrdinaryText ();
  return 1;
}

static int
utd_startStyle ()
{
  firstBrlNode = NULL;
  if (styleSpec->node != NULL && style->action == document)
    {
      documentNode = styleSpec->node;
      ud->vert_line_pos = ud->page_top;
      return 1;
    }
  if (style->topBoxline[0] && styleSpec->node)
  {
    utd_addBoxline(style->topBoxline, -1);
  }
  if (!ud->paragraphs)
    return 1;
  if ((style->lines_before ||
       style->newpage_before || style->righthand_page)
      && styleSpec->node != NULL)
    {
      postponedStart = 1;
    }
  return 1;
}

static int
utd_editTrans ()
{
  int translationLength;
  if (ud->needs_editing && !(ud->contents == 2)
      && !(style->format == computerCoded)
      && ud->edit_table_name && (ud->has_math || ud->has_chem
				 || ud->has_music))
    {
      lou_dotsToChar (ud->edit_table_name, ud->translated_buffer,
		      ud->text_buffer, ud->translated_length, ud->louis_mode);
      translationLength = ud->translated_length;
      translatedLength = MAX_TRANS_LENGTH;
      if (!lou_translate
	  (ud->edit_table_name, ud->text_buffer, &translationLength,
	   ud->translated_buffer, &translatedLength, NULL, NULL, NULL,
	   NULL, NULL, dotsIO))
	{
	  logMessage
	    (LOU_LOG_FATAL, "edit table '%s' could not be found or contains errors",
	     ud->edit_table_name);
	  ud->edit_table_name = NULL;
	  return 0;
	}
    }
  else
    {
      translatedLength = ud->translated_length;
    }
  translatedBuffer = ud->translated_buffer;
  return 1;
}

static int
utd_styleBody ()
{
  sem_act action;
  logMessage(LOU_LOG_DEBUG, "Begin utd_styleBody");
  if (!utd_editTrans ())
    return 0;
  if (!ud->paragraphs)
    assignTranslations ();
  else
    {
      cellsOnLine = 0;
      action = style->action;
      if (action == contentsheader && ud->contents != 2)
	{
	  initialize_contents ();
	  start_heading (action, translatedBuffer, translatedLength);
	  finish_heading (action);
	  ud->text_length = ud->translated_length = ud->sync_text_length = 0;
	  ud->in_sync = ud->hyphenate;
	  return 1;
	}
      if (ud->contents == 1)
	{
	  if (ud->braille_page_number_at
	      && (action == heading1 || action == heading2
		  || action == heading3 || action == heading4))
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
	  break;
	}
      if (ud->contents == 1)
	finish_heading (action);
      styleSpec->status = resumeBody;
    }
  ud->translated_length = 0;
  ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  firstBrlNode = NULL;
  logMessage(LOU_LOG_DEBUG, "Finish utd_styleBody");
  return 1;
}

static int
utd_finishStyle ()
{
  PageStatus curPageStatus = checkPageStatus ();
  logMessage(LOU_LOG_DEBUG, "Begin utd_finishStyle");
  if (style->bottomBoxline[0])
  {
    utd_addBoxline(style->bottomBoxline, 1);
  }
  if (!ud->paragraphs)
    return 1;
  if (style->newpage_after)
    utd_fillPage ();
  else if (style->lines_after > 0)
    {
      if (curPageStatus == nearBottom)
	utd_fillPage ();
      else
	{
	  if (!utd_makeBlankLines (style->lines_after, 1))
	    return 0;
	}
    }
  brlNode = firstBrlNode = NULL;
  logMessage(LOU_LOG_DEBUG, "Finish utd_finishStyle");
  return 1;
}

void
do_utdxxxximg (xmlNode * node)
{
  sem_act action;
  char *cwidth;
  char *cheight;
  xmlChar *src;
  int width;
  int height;
  int maxWidth;
  int maxHeight;
  xmlNode *curBrl;
  if (node == NULL)
    return;
  action = ud->stack[ud->top];
  cwidth = xmlGetProp (node, (xmlChar *) "twidth");
  cheight = xmlGetProp (node, (xmlChar *) "twidth");
  src = xmlGetProp (node, (xmlChar *) "tsrc");
  if (cwidth == NULL || cheight == NULL || src == NULL)
    return;
  width = atoi (cwidth);
  height = atoi (cheight);
  maxWidth = ud->page_right - ud->page_left;
  curBrl = xmlNewNode (NULL, (xmlChar *) "brl");
  link_brl_node (xmlAddNextSibling (node, curBrl));
  xmlNewProp (brlNode, (xmlChar *) "img", src);
  if (action == utddispimg)
    {
      if (!ud->paragraphs)
	return;
      maxHeight = ud->page_bottom - ud->page_top - (2 * ud->wide_line);
      /* Don't use first and last lines on page. */
      if ((ud->vert_line_pos + height) > maxHeight)
	do_righthandpage ();
      makeNewline (brlNode, 0);
      lineWidth += height;
      makeNewline (brlNode, 0);
    }
  else if (action == utdinlnimg)
    {
      if (height > (ud->wide_line - 2) || (width > maxWidth))
	return;
//  horizLinePos += width;
    }
  else
    return;
}

void
output_xml (xmlDoc * doc)
{
  if (ud->outFile)
    xmlDocDump (ud->outFile, doc);
  else
    {
      xmlChar *dumpLoc;
      int dumpSize;
      xmlDocDumpMemory (doc, &dumpLoc, &dumpSize);
      if (dumpSize > (CHARSIZE * ud->outlen))
	{
	  logMessage (LOU_LOG_ERROR, "output buffer too small");
	  ud->outlen_so_far = 0;
	}
      else
	{
	  memcpy (ud->outbuf, dumpLoc, dumpSize);
	  ud->outlen_so_far = dumpSize;
	}
      xmlFree (dumpLoc);
    }
  return;
}

static int
utd_finish ()
{
  xmlNode *newNode;
  logMessage(LOU_LOG_DEBUG, "Begin utd_finish");
  if (ud->paragraphs)
    {
      newNode = xmlNewNode (NULL, (xmlChar *) "brl");
      brlNode = xmlAddChild (documentNode, newNode);
      if (ud->style_top < 0)
	ud->style_top = 0;
      if (ud->text_length != 0)
	insert_translation (ud->main_braille_table);
      if (ud->translated_length != 0)
	write_paragraph (para, NULL);
      if (style == NULL)
	style = lookup_style ("para");
      utd_fillPage ();
    }
  if (ud->head_node)
    {
      newNode = xmlNewNode (NULL, (xmlChar *) "meta");
      xmlNewProp (newNode, (xmlChar *) "name", (xmlChar *) "utd");
      sprintf (utilStringBuf, "braillePageNumber=%d \
firstTableName=%s \
dpi=%d \
paperWidth=%d \
paperHeight=%d \
leftMargin=%d \
rightMargin=%d \
topMargin=%d \
bottomMargin=%d", ud->braille_page_number, firstTableName, (int) ud->dpi, ud->paper_width, ud->paper_height, ud->left_margin, ud->right_margin, ud->top_margin, ud->bottom_margin);
      xmlNewProp (newNode, (xmlChar *) "content", (xmlChar *) utilStringBuf);
      xmlAddChild (ud->head_node, newNode);
    }
  if (ud->orig_format_for != utd)
    convert_utd ();
  else
    output_xml (ud->doc);
  logMessage(LOU_LOG_DEBUG, "Finish utd_finish");
  return 1;
}
