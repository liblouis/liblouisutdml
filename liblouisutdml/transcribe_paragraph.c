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

int
transcribe_paragraph (xmlNode * node, int action)
{
  StyleType *style;
  xmlNode *child;
  int branchCount = 0;
  if (node == NULL)
    return 0;
  if (ud->top == 0)
    action = 1;
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
    case markhead:
      ud->head_node = node;
      pop_sem_stack ();
      break;
    case utdbrl:
    case utdmeta:
      interpret_utd (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case configtweak:
      do_configstring (node);
      break;
    case htmllink:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case htmltarget:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (node, 1);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case boxline:
      do_boxline (node);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case blankline:
      do_blankline ();
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case linespacing:
      do_linespacing;
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case softreturn:
      do_softreturn ();
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case righthandpage:
      do_righthandpage ();
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case code:
      transcribe_computerCode (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case math:
      transcribe_math (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case graphic:
      transcribe_graphic (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case chemistry:
      transcribe_chemistry (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case music:
      transcribe_music (node, 0);
      if (action != 0)
	pop_sem_stack ();
      return 1;
    case changetable:
      change_table (node);
      return 1;
    default:
      break;
    }
  if ((style = is_style (node)) != NULL)
    start_style (style, node);
  child = node->children;
  while (child)
    {
      insert_code (node, branchCount);
      branchCount++;
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  transcribe_paragraph (child, 1);
	  break;
	case XML_TEXT_NODE:
	  insert_text (child);
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
      if (child != NULL)
	child = child->next;
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  if (style)
    end_style ();
  else
    switch (ud->stack[ud->top])
      {
      case runninghead:
	insert_translation (ud->main_braille_table);
	if (ud->translated_length > (ud->cells_per_line - 9))
	  ud->running_head_length = ud->cells_per_line - 9;
	else
	  ud->running_head_length = ud->translated_length;
	memcpy (&ud->running_head[0], &ud->translated_buffer[0],
		ud->running_head_length * CHARSIZE);
	break;
      case footer:
	insert_translation (ud->main_braille_table);
	if (ud->translated_length > (ud->cells_per_line - 9))
	  ud->footer_length = ud->cells_per_line - 9;
	else
	  ud->footer_length = ud->translated_length;
	memcpy (&ud->footer[0], &ud->translated_buffer[0],
		ud->footer_length * CHARSIZE);
	break;
      default:
	break;
      }
  if (action != 0)
    pop_sem_stack ();
  else
    {
      insert_translation (ud->main_braille_table);
      write_paragraph (para, node);
    }
  return 1;
}
