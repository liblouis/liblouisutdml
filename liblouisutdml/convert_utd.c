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

static int
nullPrivate (xmlNode * node)
{
  xmlNode *child;
  if (node == NULL)
    return 0;
  node->_private = NULL;
  child = node->children;
  while (child)
    {
      child->_private = NULL;
      nullPrivate (child);
      child = child->next;
    }
  return 1;
}

int
convert_utd ()
{
  xmlNode *rootElement = xmlDocGetRootElement (ud->doc);
  int haveSemanticFile;
  if (rootElement == NULL)
    {
      logMessage (LOU_LOG_ERROR, "Document is empty");
      return 0;
    }
  clean_semantic_table ();
  ud->format_for = ud->orig_format_for;
  ud->semantic_files = ud->converter_sem;
  haveSemanticFile = compile_semantic_table (rootElement);
  nullPrivate (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  switch (ud->format_for)
    {
    case dsbible:
      utd2dsBible (rootElement);
      break;
    case brf:
      utd2brf (rootElement);
      break;
    case transinxml:
      utd2transinxml (rootElement);
      break;
    case pef:
      utd2pef (rootElement);
      break;
    case volumes:
    case volumesPef:
    case volumesBrf:
      utd2volumes (rootElement);
      break;
    default:
      break;
    }
  return 1;
}

int
pass2_conv ()
{
  xmlNode *rootElement = xmlDocGetRootElement (ud->doc);
  int haveSemanticFile;
  xmlNode *child;
  if (rootElement == NULL)
    {
      logMessage (LOU_LOG_ERROR, "Document is empty");
      return 0;
    }
  clean_semantic_table ();
  ud->semantic_files = ud->pass2_conv_sem;
  haveSemanticFile = compile_semantic_table (rootElement);
  nullPrivate (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  if (!haveSemanticFile)
    return 0;
  ud->format_for = utd;
  ud->top = 0;
  ud->stack[0] = no;
  ud->style_top = -1;
  ud->text_length = 0;
  ud->translated_length = 0;
  ud->sync_text_length = 0;
  ud->in_sync = ud->hyphenate;
  child = rootElement->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  transcribe_paragraph (child, 0);
	  break;
	case XML_TEXT_NODE:
	  insert_text (child);
	  break;
	case XML_CDATA_SECTION_NODE:
	  transcribe_cdataSection (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  return 1;
}
