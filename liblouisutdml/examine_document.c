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

static void examText (xmlNode * node);
static void examCdataa (xmlNode * node);
static int doConfigfile (xmlNode * node);

int
examine_document (xmlNode * node)
{
/*Examine the parse tree, add semantic attributes and set indicators.*/
  xmlNode *child;
  logMessage(LOU_LOG_DEBUG, "Begin examine_document: node->name=%s", node->name);
  if (node == NULL)
    return 0;
  ud->stack[++ud->top] = set_sem_attr (node);
  if (ud->format_for == utd)
  {
  unsigned char *name = get_sem_name (node);
  if (name[0] != 0)
  xmlNewProp (node, (xmlChar *)"semantics", (xmlChar *)name);
  }
  switch (ud->stack[ud->top])
    {
    case skip:
      pop_sem_stack ();
      return 1;
    case configfile:
      doConfigfile (node);
      break;
    case configstring:
      do_configstring (node);
      break;
    case code:
      ud->has_comp_code = 1;
      break;
    case contentsheader:
      ud->has_contentsheader = 1;
      break;
    case math:
      ud->has_math = 1;
      break;
    case chemistry:
      ud->has_chem = 1;
      break;
    case graphic:
      ud->has_graphics = 1;
      break;
    case music:
      ud->has_music = 1;
      break;
    case pagebreak:
      ud->has_pagebreak = 1;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  examine_document (child);
	  break;
	case XML_TEXT_NODE:
	  examText (child);
	  break;
	case XML_CDATA_SECTION_NODE:
	  examCdataa (child);
	  examine_document (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  ud->top--;
  return 1;
}

static void
examText (xmlNode * node)
/*We may want to examine text content in the future*/
{
  logMessage(LOU_LOG_DEBUG, "Begin examText: node->content=%s", node->content);
  switch (ud->stack[ud->top])
    {
    case pagenum:
      break;
    default:
      break;
    }
}

static void
examCdataa (xmlNode * node)
{
  logMessage(LOU_LOG_DEBUG, "Begin examCdata");
  ud->has_cdata = 1;
}

static int
doConfigfile (xmlNode * node)
{
  int k;
  char filePath[MAXNAMELEN];
  ud->text_length = 0;
  insert_code (node, 0);
  for (k = 0; k < ud->text_length; k++)
    ud->typeform[k] = (xmlChar) ud->text_buffer[k];
  ud->typeform[k] = 0;
  if (!find_file ((char *) ud->typeform, filePath))
    return 0;
  if (!config_compileSettings (filePath))
    return 0;
  return 1;
}

int
do_configstring (xmlNode * node)
{
  int k;
  int kk = 0;
  xmlChar configString[2 * MAXNAMELEN];
  ud->text_length = 0;
  insert_code (node, 0);
  configString[kk++] = ud->string_escape;
  for (k = 0; k < ud->text_length; k++)
    {
      if (ud->text_buffer[k] == '=')
	configString[kk++] = ' ';
      else if (ud->text_buffer[k] == ';')
	configString[kk++] = '\n';
      else
	configString[kk++] = (xmlChar) ud->text_buffer[k];
    }
  configString[kk++] = '\n';
  configString[kk] = 0;
  ud->text_length = 0;
  if (!config_compileSettings ((char *) configString))
    return 0;
  return 1;
}
