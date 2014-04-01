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
transcribe_cdataSection (xmlNode * node)
{
  switch (get_sem_attr (node))
    {
    case no:
      insert_utf8 (node->content);
      return 1;
    case skip:
      return 1;
    case code:
      if (ud->text_length > 0 || ud->translated_length > 0)
	{
	  StyleType *style;
	  insert_translation (ud->main_braille_table);
	  style = find_current_style ();
	  if (style != NULL)
	    write_paragraph (style->action, node);
	  else
	    write_paragraph (para, node);
	}
      insert_utf8 (node->content);
      memset (ud->typeform, computer_braille, ud->text_length);
      insert_translation (ud->compbrl_table_name);
      write_paragraph (code, node);
      return 1;
    default:
      insert_utf8 (node->content);
      return 1;
    }
}
