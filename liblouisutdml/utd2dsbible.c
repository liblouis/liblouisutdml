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
#include <string.h>
#include "louisutdml.h"

static int findBrlNodes (xmlNode * node);
static int dsBibleDoBrlNode (xmlNode * node, int action);
static int beginDocument ();
static int finishBrlNode ();
static int finishDocument ();
static int doUtdbrlonly (xmlNode * node, int action);
static int doUtdnewpage (xmlNode * node);
static int doUtdnewline (xmlNode * node);
static int doUtddispimg (xmlNode * node);
static int firstPage;
static int firstLineOnPage;
static int startOfLastLine;
static widechar firstVerse[MAXNUMLEN];
static int firstVerseLength;
static widechar lastVerse[MAXNUMLEN];
static int lastVerseLength;

static int
doVerseNumber (widechar * line, int length)
{
  widechar number[MAXNUMLEN];
  int numberLength = 0;
  int k;
  int kk;
  for (k = 0; k < length; k++)
    if (line[k] >= 16 && line[k] <= 25)
      break;
  if (k == length)
    return length;
  number[numberLength++] = line[k] + 32;
  if (line[k + 1] >= 16 && line[k + 1] <= 25)
    number[numberLength++] = line[k + 1] + 32;
  if (line[k + 2] >= 16 && line[k + 2] <= 25)
    number[numberLength++] = line[k + 2] + 32;
  line[k] = 39;
  if (numberLength > 1)
    {
      k++;
      for (kk = 0; kk < length; kk++)
	line[k + kk] = line[k + kk + numberLength - 1];
      length -= numberLength - 1;
    }
  if (numberLength == 1)
  kk = 1;
  else kk = 0;
  for (k = 0; k < numberLength; k++)
  if (number[k] == 48)
    line[kk++] = number[k] + 58;
    else  
    line[kk++] = number[k] + 48;
  if (firstVerseLength == 0)
    {
      firstVerseLength = numberLength;
      memcpy (firstVerse, number, numberLength * CHARSIZE);
    }
  else
    {
      lastVerseLength = numberLength;
      memcpy (lastVerse, number, numberLength * CHARSIZE);
    }
  return length;
}

static void
makeFooter ()
{
  int translationLength;
  int translatedLength;
  int k;
  int kk;
  firstVerse[firstVerseLength++] = ud->lit_hyphen[0];
  for (k = 0; k < lastVerseLength; k++)
    firstVerse[firstVerseLength++] = lastVerse[k];
  translationLength = firstVerseLength;
  translatedLength = MAXNUMLEN;
  lou_translate (ud->main_braille_table, firstVerse, &translationLength,
		 lastVerse, &translatedLength, NULL, NULL,
		 NULL, NULL, NULL, 0);
  firstVerseLength = 0;
  kk = startOfLastLine + ((ud->cells_per_line - translatedLength) / 2);
  for (k = 0; k < translatedLength; k++)
  ud->outbuf1[kk++] = lastVerse[k];
}

int
utd2dsBible (xmlNode * node)
{
  ud->top = -1;
  ud->style_top = -1;
  firstPage = 1;
  firstLineOnPage = 1;
  beginDocument ();
  findBrlNodes (node);
  pass2_conv ();
  finishDocument ();
  return 1;
}

static int
beginDocument ()
{
  return 1;
}

static int
finishDocument ()
{
    return 1;
  write_buffer (1, 0);
  ud->outbuf1_len_so_far = 0;
  return 1;
}

static int
doUtddispimg (xmlNode *node)
{
  return 1;
}

static int
findBrlNodes (xmlNode * node)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case utdmeta:
      return 1;
    case utdbrl:
      dsBibleDoBrlNode (node, 0);
      pop_sem_stack ();
      return 1;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  findBrlNodes (child);
	  break;
	case XML_TEXT_NODE:
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  pop_sem_stack ();
  return 1;
}

static char *blanks =
  "                                                            ";
static int
insertCharacters (const char *text, int length)
{
  int k;
  if ((ud->outbuf1_len_so_far + length) >= ud->outbuf1_len)
    return 0;
  for (k = 0; k < length; k++)
    ud->outbuf1[ud->outbuf1_len_so_far++] = text[k];
  return 1;
}

static int
doDotsText (xmlNode * node)
{
  ud->text_length = 0;
  insert_utf8 (node->content);
  if (!lou_dotsToChar (ud->main_braille_table, ud->text_buffer,
		       &ud->outbuf1[ud->outbuf1_len_so_far],
		       ud->text_length, 0))
    return 0;
  ud->outbuf1_len_so_far += ud->text_length;
  return 1;
}

static int
doUtdbrlonly (xmlNode * node, int action)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  if (ud->top == 0)
    action = 1;
  if (action != 0)
    push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case utdnewpage:
      doUtdnewpage (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewline:
      doUtdnewline (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utddispimg:
      transcribe_graphic (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case changetable:
      change_table (node);
      return 1;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  doUtdbrlonly (child, 1);
	  break;
	case XML_TEXT_NODE:
	  doDotsText (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  if (action != 0)
    pop_sem_stack ();
  return 1;
}

static int lastLinepos;

static int
doUtdnewpage (xmlNode * node)
{
  static int prevBrlPageNum = 1;
  lastLinepos = ud->page_top;
  firstLineOnPage = 1;
  if (firstPage)
    {
      firstPage = 0;
      return 1;
    }
  if ((prevBrlPageNum & 1))
    makeFooter ();
  insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
  insertCharacters (ud->pageEnd, strlen (ud->pageEnd));
  write_buffer (1, 0);
  prevBrlPageNum = atoi (xmlGetProp (node, (xmlChar *) "brlnumber"));
  return 1;
}

static int
doUtdnewline (xmlNode * node)
{
  char *xy;
  int k;
  int leadingBlanks;
  int linepos;
  if (!firstLineOnPage)
  {
    ud->outbuf1_len_so_far = startOfLastLine + 
    doVerseNumber 
    (&ud->outbuf1[startOfLastLine], ud->outbuf1_len_so_far 
   - 
    startOfLastLine);
    insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
    }
  xy = (char *) xmlGetProp (node, (xmlChar *) "xy");
  for (k = 0; xy[k] != ','; k++);
  leadingBlanks = (atoi (xy) - ud->left_margin) / ud->cell_width;
  linepos = (atoi (&xy[k + 1]) - ud->page_top) / ud->normal_line;
  startOfLastLine = ud->outbuf1_len_so_far;
  insertCharacters (blanks, leadingBlanks);
  if (firstLineOnPage)
    firstLineOnPage = 0;
  return 1;
}

int
dsBibleDoBrlNode (xmlNode * node, int action)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  if (action != 0)
    push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case markhead:
      if (ud->head_node == NULL)
	ud->head_node = node;
      pop_sem_stack ();
      break;
    case utdbrlonly:
      doUtdbrlonly (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewpage:
      doUtdnewpage (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewline:
      doUtdnewline (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utddispimg:
      transcribe_graphic (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case changetable:
      change_table (node);
      return 1;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  dsBibleDoBrlNode (child, 1);
	  break;
	case XML_TEXT_NODE:
	  doDotsText (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  if (action != 0)
    {
      pop_sem_stack ();
      return 1;
    }
  finishBrlNode ();
  return 1;
}

static int
finishBrlNode ()
{
  return 1;
}
