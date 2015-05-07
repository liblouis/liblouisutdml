/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   abilitiessoft, Inc. www.abilitiessoft.org
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

   Maintained by John J. Boyer john.boyer@abiliriessoft.com
   */

#include <stdio.h>
#include <string.h>
#include "louisutdml.h"

static int computerCodeEmptyElement (xmlNode * node, int action);

int
transcribe_computerCode (xmlNode * node, int action)
{
  xmlNode *child;
  int branchCount = 0;
  if (action == 0 && (ud->text_length > 0 || ud->translated_length > 0))
    {
      StyleType *style;
      insert_translation (ud->main_braille_table);
      ud->top--;
      style = find_current_style ();
      ud->top++;
      if (style != NULL)
	write_paragraph (style->action, node);
      else
	write_paragraph (para, node);
    }
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case skip:
      pop_sem_stack ();
      return 0;
    case code:
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
	  insert_code (node, branchCount);
	  branchCount++;
	  if (child->children)
	    transcribe_computerCode (child, 1);
	  else
	    computerCodeEmptyElement (child, 1);
	  break;
	case XML_TEXT_NODE:
	  /*Is there already a <brl> node? */
	  if (!(ud->format_for == utd && child->next != NULL
		&& strcmp ((char *) child->next->name, "brl") == 0))
	    insert_text (child);
	  /*Is there now a <brl>node? */
	  if (ud->format_for == utd && child->next != NULL
	      && strcmp ((char *) child->next->name, "brl") == 0)
	    child = child->next;	/*skip <brl> node */
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
  pop_sem_stack ();
  if (action == 0)
    {
      memset (ud->typeform, computer_braille, ud->text_length);
      insert_translation (ud->compbrl_table_name);
      write_paragraph (code, node);
    }
  return 1;
}

static int
computerCodeEmptyElement (xmlNode * node, int action)
{
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case softreturn:
      break;
    case blankline:
      break;
    case newpage:
      break;
    case righthandpage:
      break;
    default:
      break;
    }
  pop_sem_stack ();
  return 1;
}
