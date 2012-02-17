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
static int brfDoBrlNode (xmlNode * node, int action);
static int beginDocument ();
static int finishBrlNode ();
static int finishDocument ();
static int doUtdbrlonly (xmlNode * node, int action);
static int doUtdnewpage (xmlNode * node);
static int doUtdnewline (xmlNode * node);
static int doUtdgraphic (xmlNode * node);

int
utd2brf (xmlNode * node)
{
  ud->top = -1;
  ud->style_top = -1;
  beginDocument ();
  findBrlNodes (node);
  finishDocument ();
  return 1;
}

static int
beginDocument ()
{
}

static int
finishDocument ()
{
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
      brfDoBrlNode (node, 0);
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
writeCharacters (const char *text, int length)
{
  int k;
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
    case utdgraphic:
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
    {
      pop_sem_stack ();
      return 1;
    }
  return 1;
}

static int skipFirstNew = 0;
static int newpagePending = 0;
static int lastLinepos;

static int
doUtdnewpage (xmlNode * node)
{
  lastLinepos = 0;
  if (skipFirstNew)
    return 1;
  newpagePending = 1;
  return 1;
}

static int
doUtdnewline (xmlNode * node)
{
  char *xy;
  int k;
  int leadingBlanks;
  int linepos;
  if (skipFirstNew)
    skipFirstNew = newpagePending = 0;
  else
    writeCharacters (ud->lineEnd, strlen (ud->lineEnd));
  if (newpagePending)
    {
      writeCharacters (ud->pageEnd, strlen (ud->pageEnd));
      newpagePending = 0;
    }
  xy = (char *) xmlGetProp (node, (xmlChar *) "xy");
  for (k = 0; xy[k] != ','; k++);
  leadingBlanks = (atoi (xy) - ud->left_margin) / ud->cell_width;
  linepos = (atoi (&xy[k + 1]) - ud->page_top) / ud->normal_line;
  writeCharacters (blanks, leadingBlanks);
  return 1;
}

int
brfDoBrlNode (xmlNode * node, int action)
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
    case utdgraphic:
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
	  brfDoBrlNode (child, 1);
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
  if (ud->outbuf1_len_so_far == 0)
    return 1;
  write_buffer (1, 0);
  return 1;
}
