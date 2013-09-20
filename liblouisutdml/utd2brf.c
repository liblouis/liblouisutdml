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

   Maintained by John J. Boyer john.boyer@abilitiessoft.com
   */

#include <stdio.h>
#include <string.h>
#include "louisutdml.h"

static int findBrlNodes (xmlNode * node);
static int brf_doBrlNode (xmlNode * node, int action);
static int beginDocument ();
static int brf_finishBrlNode ();
static int finishDocument ();
static int brf_doUtdbrlonly (xmlNode * node, int action);
static int doUtdnewpage (xmlNode * node);
static int doUtdnewline (xmlNode * node);
static int firstPage;
static int firstLineOnPage;

int
utd2brf (xmlNode * node)
{
  ud->top = -1;
  ud->style_top = -1;
  firstPage = 1;
  firstLineOnPage = 1;
  beginDocument ();
  findBrlNodes (node);
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
brf_insertCharacters (const char *text, int length)
{
  int k;
  for (k = 0; k < length; k++)
    ud->outbuf1[ud->outbuf1_len_so_far++] = text[k];
  return 1;
}

static int
brf_doDotsText (xmlNode * node)
{
  ud->text_length = 0;
  insert_utf8 (node->content);
  for (; ud->text_buffer[ud->text_length] == 0x2800; ud->text_length--);
  if (!lou_dotsToChar (ud->main_braille_table, ud->text_buffer,
		       &ud->outbuf1[ud->outbuf1_len_so_far],
		       ud->text_length, 0))
    return 0;
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

static int lastLinepos;

static int
doUtdnewpage (xmlNode * node)
{
  lastLinepos = ud->page_top;
  firstLineOnPage = 1;
  if (firstPage)
    {
      firstPage = 0;
      return 1;
    }
  brf_insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
  brf_insertCharacters (ud->pageEnd, strlen (ud->pageEnd));
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
    brf_insertCharacters (ud->lineEnd, strlen (ud->lineEnd));
  xy = (char *) xmlGetProp (node, (xmlChar *) "xy");
  for (k = 0; xy[k] != ','; k++);
  leadingBlanks = (atoi (xy) - ud->left_margin) / ud->cell_width;
  linepos = (atoi (&xy[k + 1]) - ud->page_top) / ud->normal_line;
  brf_insertCharacters (blanks, leadingBlanks);
  if (firstLineOnPage)
    firstLineOnPage = 0;
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
      pop_sem_stack ();
      break;
    case utdbrlonly:
      brf_doUtdbrlonly (node, 0);
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
  brf_finishBrlNode ();
  return 1;
}

static int
brf_finishBrlNode ()
{
  if (ud->outbuf1_len_so_far == 0)
    return 1;
  write_buffer (1, 0);
  return 1;
}
