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

#define MAXHEADINGSIZE 4 * MAXNAMELEN

typedef struct
{
  void *next;
  sem_act action;
  int headingLength;
  widechar headingChars[MAXHEADINGSIZE];
} SaveHeading;
static SaveHeading heading;
static SaveHeading *firstHeading;
static SaveHeading *lastHeading;
static FILE *saved_outFile;
static FILE *tempFile;
static char tempFileName[MAXNAMELEN];
static int saved_udContents;
static int saved_linesOnPage;
static int saved_braillePageNumber;
static widechar saved_printPageNumber[MAXNUMLEN];
static widechar saved_printPageNumberFirst[MAXNUMLEN];
static widechar saved_printPageNumberLast[MAXNUMLEN];
static BrlPageNumFormat saved_braillePageNumberFormat;
static StyleRecord *styleSpec;

int
initialize_contents (void)
{
  int k;
  saved_braillePageNumberFormat = ud->brl_page_num_format;
  for (k = 0; ud->print_page_number[k]; k++)
    saved_printPageNumber[k] = ud->print_page_number[k];
  saved_printPageNumber[k] = 0;
  for (k = 0; ud->print_page_number_first[k]; k++)
    saved_printPageNumberFirst[k] = ud->print_page_number_first[k];
  saved_printPageNumberFirst[k] = 0;
  for (k = 0; ud->print_page_number_last[k]; k++)
    saved_printPageNumberLast[k] = ud->print_page_number_last[k];
  saved_printPageNumberLast[k] = 0;
  ud->after_contents = 1;
  saved_udContents = ud->contents;
  saved_linesOnPage = ud->lines_on_page;
  saved_braillePageNumber = ud->braille_page_number;
  ud->contents = 1;
  firstHeading = NULL;
  lastHeading = &heading;
  if (ud->format_for != utd)
    {
      saved_outFile = ud->outFile;
      strcpy (tempFileName, ud->writeable_path);
      strcat (tempFileName, "lbu_body.temp");
      if (!(tempFile = fopen (tempFileName, "w")))
	{
	  lou_logPrint ("Can't open temporary file.\n");
	  return 0;
	}
      ud->outFile = tempFile;
    }
  ud->lines_on_page = 0;
  if (ud->has_contentsheader)
    ud->braille_page_number = ud->beginning_braille_page_number;
  else
    ud->braille_page_number = 1;
  return 1;
}

int
start_heading (sem_act action, widechar * translatedBuffer, int
	       translatedLength)
{
  int k;
  if (!(ud->contents && (action == heading1 || action == heading2 ||
			 action == heading3 || action == heading4 ||
			 action == contentsheader)))
    return 1;
  if (translatedLength > 3 * MAXNAMELEN)
    translatedLength = 3 * MAXNAMELEN;
  heading.action = action;
  heading.headingLength = 0;
  for (k = 0; k < translatedLength; k++)
    heading.headingChars[heading.headingLength++] = translatedBuffer[k];
  return 1;
}

int
finish_heading (sem_act action)
{
  int k;
  int headingSize = sizeof (heading) - MAXHEADINGSIZE * CHARSIZE;
  int initHeadingLength = heading.headingLength;
  SaveHeading *headingPtr;
  if (!(ud->contents && (action == heading1 || action == heading2 ||
			 action == heading3 || action == heading4 ||
			 action == contentsheader)))
    return 1;
  heading.next = NULL;
  if (action != contentsheader)
    {
      if (*ud->print_page_number != '_')
	{
	  heading.headingChars[heading.headingLength++] = ' ';
	  k = 0;
	  while (ud->print_page_number[k])
	    heading.headingChars[heading.headingLength++] =
	      ud->print_page_number[k++];
	}
      if (*ud->braille_page_string)
	{
	  if (ud->format_for == utd)
	    {
	      if (*ud->print_page_number != '_')
		heading.headingChars[heading.headingLength++] = (B16 | B10);
	      else
		heading.headingChars[heading.headingLength++] = B16;
	    }
	  else
	    {
	      if (*ud->print_page_number != '_')
		heading.headingChars[heading.headingLength++] = 0xa0;
	      else
		heading.headingChars[heading.headingLength++] = ' ';
	    }
	  k = 0;
	  while (ud->braille_page_string[k])
	    heading.headingChars[heading.headingLength++] =
	      ud->braille_page_string[k++];
	}
    }
  if (initHeadingLength == heading.headingLength)
    /* No page numbers */
    {
      if (ud->format_for == utd)
	heading.headingChars[heading.headingLength++] = (B16 | B10);
      else
	heading.headingChars[heading.headingLength++] = 0xa0;
    }
  headingSize += heading.headingLength * CHARSIZE;
  headingPtr = malloc (headingSize);
  memcpy (headingPtr, &heading, headingSize);
  lastHeading->next = headingPtr;
  lastHeading = headingPtr;
  if (firstHeading == NULL)
    firstHeading = headingPtr;
  return 1;
}

int
make_contents (void)
{
  SaveHeading *currentHeading = NULL;
  int old_braillePageNumber;
  sem_act action;
  int bytesRead;
  StyleType *style;
  if (!ud->contents)
    return 1;
  old_braillePageNumber = ud->braille_page_number;
  if (ud->format_for != utd)
    {
      fclose (tempFile);
      ud->outFile = saved_outFile;
    }
  if (firstHeading != NULL)
    {
      int k;
      ud->lines_on_page = saved_linesOnPage;
      ud->braille_page_number = saved_braillePageNumber;
      styleSpec = &ud->style_stack[ud->style_top];
      styleSpec->curBrlNumFormat = saved_braillePageNumberFormat;
      ud->brl_page_num_format = saved_braillePageNumberFormat;
      for (k = 0; saved_printPageNumber[k]; k++)
	{
	  ud->print_page_number[k] = saved_printPageNumber[k];
	}
      ud->print_page_number[k] = 0;
      for (k = 0; saved_printPageNumberFirst[k]; k++)
	{
	  ud->print_page_number_first[k] = saved_printPageNumberFirst[k];
	}
      ud->print_page_number_first[k] = 0;
      for (k = 0; saved_printPageNumberLast[k]; k++)
	{
	  ud->print_page_number_last[k] = saved_printPageNumberLast[k];
	}
      ud->print_page_number_last[k] = 0;
      do_newpage ();
      ud->contents = 2;
      currentHeading = firstHeading;
      while (currentHeading != NULL)
	{
	  switch (currentHeading->action)
	    {
	    case contentsheader:
	    default:
	      action = currentHeading->action;
	      break;
	    case heading1:
	      action = contents1;
	      break;
	    case heading2:
	      action = contents2;
	      break;
	    case heading3:
	      action = contents3;
	      break;
	    case heading4:
	      action = contents4;
	      break;
	    }
	  style = action_to_style (action);
	  start_style (style, NULL);
	  memcpy (ud->translated_buffer, currentHeading->headingChars,
		  currentHeading->headingLength * CHARSIZE);
	  ud->translated_length = currentHeading->headingLength;
	  end_style ();
	  currentHeading = currentHeading->next;
	}
      do_newpage ();
      ud->prelim_pages = ud->braille_page_number;
      ud->braille_page_number = saved_braillePageNumber;
      /*Free headings */
      currentHeading = firstHeading;
      while (currentHeading->next != NULL)
	{
	  lastHeading = currentHeading;
	  currentHeading = currentHeading->next;
	  free (lastHeading);
	}
      ud->contents = saved_udContents;
      ud->braille_page_number = old_braillePageNumber;
    }
  if (ud->format_for == utd)
    return 1;
  if (!(tempFile = fopen (tempFileName, "r")))
    {
      lou_logPrint ("Can't open temporary file.\n");
      return 0;
    }
  do
    {
      bytesRead = fread (ud->translated_buffer, 1, sizeof
			 (ud->translated_buffer), tempFile);
      fwrite (ud->translated_buffer, 1, bytesRead, ud->outFile);
    }
  while (bytesRead != 0);
  fclose (tempFile);
  return 1;
}
