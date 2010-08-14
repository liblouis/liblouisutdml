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

static char *blanks =
  "                                                            ";
static int
writeCharacters (const char *text, int length)
{
  int k;
  for (k = 0; k < length; k++)
    ud->outbuf[k] = text[k];
  ud->outlen_so_far = length;
  write_outbuf ();
  return 1;
}

static int
doDotsText (xmlNode * node)
{
  ud->text_length = 0;
  insert_utf8 (node->content);
  if (!lou_dotsToChar (ud->main_braille_table, ud->text_buffer,
		       ud->outbuf, ud->text_length))
    return 0;
  ud->outlen_so_far = ud->text_length;
  write_outbuf ();
  return 1;
}

static int
doUtdbrlonly (xmlNode * node)
{
  interpret_utd (node, skipChoicesBefore);
  return 1;
}

static int skipFirstNew = 0;

static int
doUtdmeta (xmlNode * node)
{
  xmlChar *attrValue = xmlGetProp (node, (xmlChar *) "content");
  int k;
  int kk = 0;
  xmlChar configString[2 * MAXNAMELEN];
  skipFirstNew = 1;
  configString[kk++] = ud->string_escape;
  for (k = 0; attrValue[k] != 0; k++)
    {
      if (attrValue[k] == '=')
	configString[kk++] = ' ';
      else if (attrValue[k] == ',')
	configString[kk++] = '\n';
      else
	configString[kk++] = (xmlChar) attrValue[k];
    }
  configString[kk] = 0;
  if (!config_compileSettings ((char *) configString))
    return 0;
  return 1;
}

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
interpret_utd (xmlNode * node, NodeAction action)
{
  xmlNode *child;
  if (node == NULL || ud->format_for == utd)
    return 0;
  if (!(action == skipChoicesBefore))
    {
      if (ud->top == 0)
	action = otherCall;
      if (action != firstCall)
	push_sem_stack (node);
      switch (ud->stack[ud->top])
	{
	case markhead:
	  ud->head_node = node;
	  pop_sem_stack ();
	  break;
	case utdmeta:
	  doUtdmeta (node);
	  if (action != firstCall)
	    pop_sem_stack ();
	  return 1;
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
	default:
	  break;
	}
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  interpret_utd (child, 1);
	  break;
	case XML_TEXT_NODE:
	  doDotsText (child);
	default:
	  break;
	}
      child = child->next;
    }
  if (action != firstCall)
    pop_sem_stack ();
  return 1;
}
