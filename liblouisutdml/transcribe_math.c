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

static int mathTrans ();
static void mathCreateBrlNode ();
static void mathText (xmlNode * node, int action);
static xmlNode *curLink;

int
transcribe_math (xmlNode * node, int action)
{
  StyleType *style;
  xmlNode *child;
  int branchCount = 0;
  logMessage(LOU_LOG_INFO, "Begin transcribe_math");
  if (node == NULL)
    return 0;
  if (action == 0)
    {
      logMessage(LOU_LOG_DEBUG, "Math node action==0");
      insert_translation (ud->main_braille_table);
      curLink = node;
      if (ud->format_for == utd)
        {
          mathCreateBrlNode ();
        }
    }
  else
    {
      logMessage(LOU_LOG_DEBUG, "Math node action!=0");
      push_sem_stack (node);
    }
  switch (ud->stack[ud->top])
    {
    case skip:
      logMessage(LOU_LOG_DEBUG, "Math node skip");
      pop_sem_stack ();
      return 1;
    case reverse:
      logMessage(LOU_LOG_DEBUG, "Math node reverse");
      do_reverse (node);
      break;
    default:
      break;
    }
  if ((style = is_style (node)) != NULL)
    {
      logMessage(LOU_LOG_DEBUG, "Math node start style");
      mathTrans ();
      start_style (style, node);
    }
  child = node->children;
  while (child)
    {
      insert_code (node, branchCount);
      branchCount++;
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  transcribe_math (child, 1);
	  break;
	case XML_TEXT_NODE:
	  mathText (child, 1);
	  break;
	case XML_CDATA_SECTION_NODE:
	  transcribe_cdataSection (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  if (style)
    {
      logMessage(LOU_LOG_DEBUG, "Math node end style");
      mathTrans ();
      end_style ();
    }
  pop_sem_stack ();
  if (action == 0)
    mathTrans ();
  logMessage(LOU_LOG_INFO, "Finish transcribe_math");
  return 1;
}

static int
mathTrans ()
{
  int translationLength;
  int translatedLength;
  int k;
  if (ud->text_length == 0)
    return 1;
  ud->needs_editing = 1;
  translatedLength = MAX_TRANS_LENGTH - ud->translated_length;
  translationLength = ud->text_length;
  if (ud->format_for == utd)
    {
      ud->text_buffer[ud->text_length++] = ENDSEGMENT;
      translationLength++;
      k = lou_translate (ud->mathexpr_table_name,
			 ud->text_buffer,
			 &translationLength,
			 &ud->
			 translated_buffer[ud->translated_length],
			 &translatedLength,
			 ud->typeform, NULL, NULL,
			 NULL, NULL, dotsIO);
      ud->in_sync = 0;
      memset (ud->typeform, 0, sizeof (ud->typeform));
      ud->text_length = 0;
      if (!k)
	{
	  logMessage (LOU_LOG_ERROR, "Could not open table %s", ud->mathexpr_table_name);
	  ud->mathexpr_table_name = NULL;
	  return 0;
	}
      if ((ud->translated_length + translatedLength) < MAX_TRANS_LENGTH)
	  ud->translated_length += translatedLength;
      else
	ud->translated_length = MAX_TRANS_LENGTH;
    }
  else
    insert_translation (ud->mathexpr_table_name);
  return 1;
}

static void
mathText (xmlNode * node, int action)
{
    insert_utf8 (node->content);
}

static void
mathCreateBrlNode ()
{
  xmlNode *curBrlNode;
  xmlNode *newNode = xmlNewNode (NULL, (xmlChar *) "brl");
  xmlSetProp (newNode, (xmlChar *) "modifiers", (xmlChar *) "notext");
  curBrlNode = xmlAddNextSibling (curLink, newNode);
  link_brl_node (curBrlNode);
  curLink = curBrlNode;
}
