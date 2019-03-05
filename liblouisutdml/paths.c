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
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"

static char *lastPath = NULL;

static int
addPath (const char *path)
{
  int k;
  int curLen = strlen (ud->path_list);
  if ((curLen + 1 + strlen (path)) >= sizeof (ud->path_list))
    return 0;
  if (path[0] == 0)
    return 1;
  for (k = 0; path[k]; k++)
    if (path[k] == ',')
      {
	logMessage (LOU_LOG_ERROR, "Path %s must not contain a comma", path);
	return 0;
      }
  if (curLen > 0)
    {
      ud->path_list[curLen++] = ',';
      ud->path_list[curLen] = 0;
    }
  lastPath = &ud->path_list[strlen (ud->path_list)];
  strcpy (lastPath, path);
  return 1;
}

int
set_paths (const char *configPath)
{
  char currentPath[MAXNAMELEN];
  char *dataPath = NULL;
  char *writePath = NULL;

/*Set configuration path first*/
  ud->config_path = NULL;
  if (configPath != NULL && configPath[0] != 0)
    {
      ud->config_path = alloc_string (configPath);
      if (!addPath (configPath))
	return 0;
    }

/*Set other paths here*/

/* First, see if the program has set a path. */
  dataPath = lou_getDataPath ();
  if (dataPath)
    {
#ifdef _Win32
      strcpy (currentPath, dataPath);
      strcat (currentPath, "\\liblouis\\tables\\");
      if (!addPath (currentPath))
	return 0;
      strcpy (currentPath, dataPath);
      strcat (currentPath, "\\liblouisutdml\\lbu_files\\");
      ud->lbu_files_path = alloc_string (currentPath);
      if (!addPath (currentPath))
	return 0;
#else
      strcpy (currentPath, dataPath);
      strcat (currentPath, "/liblouis/tables/");
      if (!addPath (currentPath))
	return 0;
      strcpy (currentPath, dataPath);
      strcat (currentPath, "/liblouisutdml/lbu_files/");
      ud->lbu_files_path = alloc_string (currentPath);
      if (!addPath (currentPath))
	return 0;
#endif
    }
  else
    {
#ifdef _WIN32
/* Set Windows Paths */
      {
	char louisPath[MAXNAMELEN];
	char lbuPath[MAXNAMELEN];
	strcpy (currentPath, lou_getProgramPath ());
	strcat (currentPath, "..\\");
	if (!addPath (currentPath))
	  return 0;
	strcpy (lbuPath, currentPath);
	strcat (lbuPath, "share\\liblouisutdml\\lbu_files\\");
      ud->lbu_files_path = alloc_string (lbuPath);
	strcpy (louisPath, currentPath);
	strcat (louisPath, "share\\liblouis\\tables\\");
	addPath (louisPath);
	addPath (lbuPath);
      }
#else
/* Set Unix paths (Linux, Mac OS X, etc.) */
      addPath (LIBLOUIS_TABLES_PATH);
      addPath (LBU_PATH);
      ud->lbu_files_path = alloc_string (LBU_PATH);
#endif /*WWIN32 */
    }

/* set current directory last*/
  currentPath[0] = '.';
  currentPath[1] = ud->file_separator;
  currentPath[2] = 0;
  if (!addPath (currentPath))
    return 0;
  if ((writePath = lbu_getWriteablePath ()) == NULL)
    lbu_setWriteablePath (lastPath);
  else
    {
      currentPath[0] = ud->file_separator;
      currentPath[1] = 0;
      strcat (writePath, currentPath);
    }
  return 1;
}
