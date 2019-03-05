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

   Maintained by John J. Boyer john.boyer@abilitiessoft.org
   */

#include <stdio.h>
#include <string.h>
#include "louisutdml.h"

static int brf_beginDocument ();
static int brf_findBrlNodes (xmlNode * node);
static int brf_doBrlNode (xmlNode * node, int action);
static int brf_saveBuffer ();
static int brf_doUtdbrlonly (xmlNode * node, int action);
static int brf_doUtdnewpage (xmlNode * node);
static int brf_doUtdnewline (xmlNode * node);
static int brf_finishDocument ();

static int firstPage;
static int firstLineOnPage;
static int prevLinePos;

int
utd2brf (xmlNode * node)
{
  ud->top = -1;
  ud->style_top = -1;
  firstPage = 1;
  firstLineOnPage = 1;
  brf_beginDocument ();
  brf_findBrlNodes (node);
  brf_finishDocument ();
  return 1;
}

static int
brf_beginDocument ()
{
  return 1;
}

static int
brf_finishDocument ()
{
  return 1;
}

static int
brf_findBrlNodes (xmlNode * node)
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
      logMessage(LOU_LOG_DEBUG, "Processing brl node");
      brf_doBrlNode (node, 0);
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
	  brf_findBrlNodes (child);
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
brf_insertCharacters (const char *text, int length)
{
  int k;
  if (length <= 0 || (ud->outbuf1_len_so_far + length) > ud->outbuf1_len)
    return 0;
  for (k = 0; k < length; k++)
    ud->outbuf1[ud->outbuf1_len_so_far++] = text[k];
  return 1;
}

static int
brf_doDotsText (xmlNode * node)
{
  logMessage(LOU_LOG_DEBUG, "brf_doDotsText %s", node->content);
  ud->text_length = 0;
  insert_utf8 (node->content);
  if ((ud->outbuf1_len_so_far + ud->text_length) > ud->outbuf1_len)
    brf_saveBuffer ();
  logMessage(LOU_LOG_DEBUG, "text_buffer: %s", ud->text_buffer);
  if (!lou_dotsToChar (ud->main_braille_table, ud->text_buffer,
		       &ud->outbuf1[ud->outbuf1_len_so_far],
		       ud->text_length, 0))
    return 0;
  logMessage(LOU_LOG_DEBUG, "ud->textLength %d", ud->text_length);
  logMessage(LOU_LOG_DEBUG, "ud->outbuf1: %s", &ud->outbuf1[ud->outbuf1_len_so_far+1]);
  ud->outbuf1_len_so_far += ud->text_length;
  return 1;
}

static int
brf_doUtdbrlonly (xmlNode * node, int action)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  if (action != 0)
    push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case utdnewpage:
      brf_doUtdnewpage (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewline:
      brf_doUtdnewline (node);
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
      break;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  brf_doUtdbrlonly (child, 1);
	  break;
	case XML_TEXT_NODE:
	  if (ud->stack[ud->top] == utdbrlonly)
	    /* print text */
	    break;
	  brf_doDotsText (child);
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

static int
brf_doUtdnewpage (xmlNode * node)
{
  firstLineOnPage = 1;
  if (firstPage)
    {
      firstPage = 0;
      return 1;
    }
  for (; ud->outbuf1_len_so_far > 0
       && ud->outbuf1[ud->outbuf1_len_so_far - 1] == ' ';
       ud->outbuf1_len_so_far--);
  brf_insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
  brf_insertCharacters (ud->pageEnd, strlen (ud->pageEnd));
  brf_saveBuffer ();
  return 1;
}

static int
brf_doUtdnewline (xmlNode * node)
{
  char *xy;
  int k;
  int leadingBlanks;
  int linePos;
  for (; ud->outbuf1_len_so_far > 0
       && ud->outbuf1[ud->outbuf1_len_so_far - 1] == ' ';
       ud->outbuf1_len_so_far--);
  if (firstLineOnPage)
    {
      prevLinePos = ud->page_top;
      firstLineOnPage = 0;
    }
  xy = (char *) xmlGetProp (node, (xmlChar *) "xy");
  k = atoi (xy);
  leadingBlanks = (k - ud->left_margin) / ud->cell_width;
  if (leadingBlanks < 0 || leadingBlanks > ud->cells_per_line)
    leadingBlanks = 0;
  for (k = 0; xy[k] != ','; k++);
  linePos = atoi (&xy[k + 1]);
  if (linePos < ud->page_top)
    linePos = ud->page_top;
  if (linePos > ud->page_bottom)
    linePos = ud->page_bottom;
  /* The following will need modification for wide lines and graphics. */
  for (k = prevLinePos; k < linePos; k += ud->normal_line)
    brf_insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
  prevLinePos = linePos;
  brf_insertCharacters (blanks, leadingBlanks);
  return 1;
}

int
brf_doBrlNode (xmlNode * node, int action)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  if (action == 0)
    ud->outbuf1_len_so_far = 0;
  else
    push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case markhead:
      if (ud->head_node == NULL)
	ud->head_node = node;
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdbrlonly:
      brf_doUtdbrlonly (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewpage:
      brf_doUtdnewpage (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case utdnewline:
      brf_doUtdnewline (node);
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
      break;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  brf_doBrlNode (child, 1);
	  break;
	case XML_TEXT_NODE:
	  brf_doDotsText (child);
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
  brf_saveBuffer ();
  return 1;
}

static int
brf_saveBuffer ()
{
  if (ud->outbuf1_len_so_far <= 0)
    return 1;
  write_buffer (1, 0);
  ud->outbuf1_len_so_far = 0;
  return 1;
}
