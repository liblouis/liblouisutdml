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
change_table (xmlNode * node)
{
  xmlNode *child;
  int branchCount = 0;
  const char *oldTable;
  const char *newTable;
  char completePath[MAXNAMELEN];
  newTable = (char *) get_attr_value (node);
  if (strlen ((char *) newTable) < 5)
    return 0;
  if (!find_file (newTable, completePath))
    strcpy(completePath, newTable);
  if (!lou_getTable (completePath))
    {
      logMessage (LOU_LOG_ERROR, "Table %s cannot be found", newTable);
      return 0;
    }
  insert_translation (ud->main_braille_table);
  oldTable = ud->main_braille_table;
  ud->main_braille_table = completePath;
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
	  break;
	case XML_TEXT_NODE:
	  insert_text (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  insert_code (node, branchCount);
  insert_code (node, -1);
  insert_translation (ud->main_braille_table);
  ud->main_braille_table = oldTable;
  pop_sem_stack ();
  return 1;
}
