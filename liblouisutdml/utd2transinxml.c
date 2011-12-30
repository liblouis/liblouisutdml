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
static int doBrlNode (xmlNode * node, int action);
static int beginDocument ();
static int finishBrlNode ();
static int finishDocument ();

int
utd2transinxml (xmlNode * node)
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
  output_xml (ud->doc);
}

static xmlNode *curNode;
static int useNextNode;
static xmlNode *nextNode;

static int
findBrlNodes (xmlNode * node)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  useNextNode = 0;
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case utdmeta:
      return 1;
    case utdbrl:
      curNode = node;
      doBrlNode (node, 0);
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
      if (useNextNode)
      child = nextNode;
      else
      child = child->next;
    }
  pop_sem_stack ();
  useNextNode = 0;
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
		       ud->text_length, ud->louis_mode))
    return 0;
  ud->outbuf1_len_so_far += ud->text_length;
  return 1;
}

static int
doUtdbrlonly (xmlNode * node)
{
  utd2transinxml (node);
  return 1;
}

static int skipFirstNew = 0;
static int newpagePending = 0;

static int
doUtdnewpage (xmlNode * node)
{
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
  leadingBlanks = (atoi (&xy[k + 1]) - ud->left_margin) / CELLWIDTH;
  writeCharacters (blanks, leadingBlanks);
  return 1;
}

int
doBrlNode (xmlNode * node, int action)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  ud->outbuf1_len_so_far = 0;
  if (ud->top == 0)
    action = 1;
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
      doUtdbrlonly (node);
      if (action != firstCall)
	pop_sem_stack ();
      return 1;
    case utdnewpage:
      doUtdnewpage (node);
      if (action != firstCall)
	pop_sem_stack ();
      return 1;
    case utdnewline:
      doUtdnewline (node);
      if (action != firstCall)
	pop_sem_stack ();
      return 1;
    case utdgraphic:
      transcribe_graphic (node, firstCall);
      if (action != firstCall)
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
	  doBrlNode (child, 1);
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
  int wcLength;
  int utf8Length;
  unsigned char *transText = (unsigned char *) ud->outbuf2;
  xmlNode *transNode;
  xmlNode *oldText;
  xmlNode *oldNode;
  wcLength = ud->outbuf1_len_so_far;
  utf8Length = ud->outbuf2_len;
  wc_string_to_utf8 (ud->outbuf1, &wcLength, transText, &utf8Length);
  transText[utf8Length] = 0;
  transNode = xmlNewText (transText);
  oldText = curNode->prev;
  if (oldText != NULL && strcmp (oldText->name, "text") == 0)
  {
  xmlUnlinkNode (oldText);
  xmlFree (oldText);
  }
  xmlAddPrevSibling (curNode, transNode);
  nextNode = curNode->next;
  useNextNode = 1;
  oldNode = curNode;
  xmlUnlinkNode (oldNode);
  xmlFree (oldNode);
  return 1;
}
