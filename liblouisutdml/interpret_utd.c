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
      lou_logPrint ("Document is empty");
      return 0;
    }
  clean_semantic_table ();
  ud->format_for = ud->orig_format_for;
  ud->contains_utd = 1;
  switch (ud->format_for)
    {
    case pef:
      ud->semantic_files = ud->pef_sem;
      break;
    case transinxml:
      ud->semantic_files = ud->transinxml_sem;
      break;
    case volumes:
      ud->semantic_files = ud->volume_sem;
      break;
    case brf:
      ud->semantic_files = ud->brf_sem;
      break;
    default:
      break;
    }
  if (ud->semantic_files == NULL)
    {
      lou_logPrint ("Missing semantic file");
      return 0;
    }
  haveSemanticFile = compile_semantic_table (rootElement);
  nullPrivate (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  transcribe_document (rootElement);
  switch (ud->format_for)
    {
    case brf:
      break;
    case transinxml:
      output_xml (ud->doc);
      break;
    case pef:
      break;
    case volumes:
      break;
    default:
      break;
    }
  return 1;
}

int
interpret_utd (xmlNode * node, NodeAction action)
{
  switch (ud->format_for)
    {
    case transinxml:
      utd2transinxml (node, action);
      break;
    case brf:
      utd2brf (node, action);
      break;
    case volumes:
      utd2volumes (node, action);
      break;
    case pef:
      utd2pef (node, action);
      break;
    default:
      break;
    }
  return 1;
}
