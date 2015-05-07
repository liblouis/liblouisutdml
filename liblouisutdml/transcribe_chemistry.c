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

   Maintained by John J. Boyer john.boyer@abilitiessoft.org
   */

#include <stdio.h>
#include <string.h>
#include "louisutdml.h"

static void chemText (xmlNode * node, int action);
static void chemCdata (xmlNode * node);
static int chemEmptyElement (xmlNode * node);

int
transcribe_chemistry (xmlNode * node, int action)
{
  xmlNode *child;
  int branchCount = 0;
  if (action != 0)
push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case no:
      if (ud->text_length > 0 && ud->text_length < MAX_LENGTH &&
	  ud->text_buffer[ud->text_length - 1] > 32)
	ud->text_buffer[ud->text_length++] = 32;
      break;
    case skip:
      if (action != 0)
	pop_sem_stack ();
      return 0;
    case math:
      transcribe_math (node, 0);
      if (action != 0)
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
insert_code (node, branchCount);
branchCount++;
	  if (child->children)
	    transcribe_paragraph (child, 1);
	  else
	    chemEmptyElement (child);
	  break;
	case XML_TEXT_NODE:
	  chemText (child, action);
	  break;
	case XML_CDATA_SECTION_NODE:
	  chemCdata (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
insert_code (node, branchCount);
  insert_code (node, -1);
  if (action != 0)
    {
      pop_sem_stack ();
      return 1;
    }
  switch (ud->stack[ud->top])
    {
    case para:
      write_paragraph (para, NULL);
      break;
    case heading10:
    case heading9:
    case heading8:
    case heading7:
    case heading6:
    case heading5:
    case heading4:
    case heading3:
      write_paragraph (para, NULL);
      break;
    case heading2:
      write_paragraph (para, NULL);
      break;
    case heading1:
      write_paragraph (para, NULL);
      break;
    default:
      break;
    }
  pop_sem_stack ();
  return 1;
}

static int
chemEmptyElement (xmlNode * node)
{
push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case softreturn:
      insert_code (node, 0);
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

static void
chemText (xmlNode * node, int action)
{
insert_text (node);
}

static void
chemCdata (xmlNode * node)
{
  insert_utf8 (node->content);
}
