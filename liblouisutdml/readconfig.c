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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "louisutdml.h"
#include "sem_names.h"
#include <ctype.h>

void
memoryError ()
{
  fprintf (stderr,
  "liblouisutdml: insufficient memory\n");
  //"liblouisutdml: Insufficient memory: %s", strerror (errno), "\n");
  exit (3);
}

typedef struct
{
  const char *fileName;
#define configString fileName
  FILE *in;
  int stringPos;
  int lineNumber;
  char line[5 * MAXNAMELEN];
  char *action;
  int actionLength;
  char *value;
  int valueLength;
  char *value2;
  int value2Length;
}
lbu_FileInfo;

static char pathEnd[2];
static double paperWidth;
static double paperHeight;
static double leftMargin;
static double rightMargin;
static double topMargin;
static double bottomMargin;
static int errorCount = 0;
static int fatalErrorCount = 0;

static void
configureError (lbu_FileInfo * nested, char *format, ...)
{
  char buffer[1024];
  va_list arguments;
  va_start (arguments, format);
#ifdef _WIN32
  _vsnprintf (buffer, sizeof (buffer), format, arguments);
#else
  vsnprintf (buffer, sizeof (buffer), format, arguments);
#endif
  va_end (arguments);
  if (nested)
    logMessage (LOU_LOG_ERROR, "%s:%d: %s", nested->fileName, nested->lineNumber, buffer);
  else
    logMessage (LOU_LOG_ERROR, "%s", buffer);
  errorCount++;
}

char *
alloc_string (const char *inString)
{
  int length;
  char *newString;
  if (inString == NULL)
    return NULL;
  length = strlen (inString);
  if ((length + ud->string_buf_len) >= sizeof (ud->string_buffer))
    return NULL;
  newString = &ud->string_buffer[ud->string_buf_len];
  strcpy (newString, inString);
  ud->string_buf_len += length + 1;
  return newString;
}

char *
alloc_string_if_not (const char *inString)
{
  int alreadyStored;
  if (inString == NULL)
    return NULL;
  alreadyStored = inString - ud->string_buffer;
  if (alreadyStored >= 0 && alreadyStored < ud->string_buf_len)
    return (char *) inString;
  return (char *) alloc_string (inString);
}

int
file_exists (const char *completePath)
{
  struct stat statInfo;
  if (stat (completePath, &statInfo) != -1)
    return 1;
  return 0;
}

int
find_file (const char *fileName, char *filePath)
{
  struct stat statInfo;
  char trialPath[MAXNAMELEN];
  int listLength;
  int k;
  int currentListPos = 0;
  int nameLength = strlen (fileName);
  int pathLength;
  filePath[0] = 0;
  for (k = 0; k < nameLength && fileName[k] != ud->file_separator; k++);
  if (k != nameLength)
    {
      /* This name contains a path */
      if (stat (fileName, &statInfo) != -1)
	{
	  strcpy (filePath, fileName);
	  return 1;
	}
      else
	return 0;
    }
  /*Process path list */
  listLength = strlen (ud->path_list);
  for (k = 0; k < listLength; k++)
    if (ud->path_list[k] == ',')
      break;
  if (k == listLength)
    {				/* Only one path */
      strcpy (trialPath, ud->path_list);
      if (trialPath[strlen (trialPath) - 1] != ud->file_separator)
	strcat (trialPath, pathEnd);
      pathLength = strlen (trialPath);
      strcat (trialPath,  fileName);
      if (stat (trialPath, &statInfo) != -1)
	{
	  strcpy (&trialPath[pathLength], fileName);
	  strcpy (filePath, trialPath);
	  return 1;
	}
    }
  else
    {				/* Search a list of paths */
      strncpy (trialPath, ud->path_list, k);
      trialPath[k] = 0;
      if (trialPath[strlen (trialPath) - 1] != ud->file_separator)
	strcat (trialPath, pathEnd);
      pathLength = strlen (trialPath);
      strcat (trialPath,  fileName);
      if (stat (trialPath, &statInfo) != -1)
	{
	  strcpy (&trialPath[pathLength], fileName);
	  strcpy (filePath, trialPath);
	  return 1;
	}
      currentListPos = k + 1;
      while (currentListPos < listLength)
	{
	  for (k = currentListPos; k < listLength; k++)
	    if (ud->path_list[k] == ',')
	      break;
	  strncpy (trialPath, &ud->path_list[currentListPos],
		   k - currentListPos);
	  trialPath[k - currentListPos] = 0;
	  if (trialPath[strlen (trialPath) - 1] != ud->file_separator)
	    strcat (trialPath, pathEnd);
	  pathLength = strlen (trialPath);
	  strcat (trialPath,  fileName);
	  if (stat (trialPath, &statInfo) != -1)
	    {
	      strcpy (&trialPath[pathLength], fileName);
	      strcpy (filePath, trialPath);
	      return 1;
	    }
	  currentListPos = k + 1;
	}
    }
  return 0;
}

static char *
findTable (lbu_FileInfo * nested)
{
  char trialPath[MAXNAMELEN];
  char filePath[MAXNAMELEN];
  struct stat statInfo;
  filePath[0] = 0;
  if (ud->config_path != NULL)
    {
      strcpy (trialPath, ud->config_path);
      if (trialPath[strlen (trialPath) -1] != ud->file_separator)
        strcat (trialPath, pathEnd);
      strcat (trialPath, nested->value);
      if (stat (trialPath, &statInfo) != -1)
	strcpy (filePath, trialPath);
    }
  else if (filePath[0] == 0)
    {
      strcpy (trialPath, ud->lbu_files_path);
      strcat (trialPath, nested->value);
      if (stat (trialPath, &statInfo) != -1)
	strcpy (filePath, trialPath);
    }
  strcpy (trialPath, filePath);
  if (trialPath[0] == 0)
    {
      if (lou_getTable (nested->value) != NULL)
	strcpy (trialPath, nested->value);
    }
  if (trialPath[0] == 0)
    {
      configureError (nested, "Table '%s' cannot be found.", nested->value);
      fatalErrorCount++;
      return NULL;
    }
  return alloc_string_if_not (trialPath);
}

static int
controlCharValue (lbu_FileInfo * nested)
{
/*Decode centrol characters*/
  int k = 0;
  char decoded[100];
  int decodedLength = 0;
  while (k < nested->valueLength)
    {
      if (nested->value[k] == '~' || nested->value[k] == '^')
	{
	  decoded[decodedLength++] = (nested->value[k + 1] | 32) - 96;
	  k += 2;
	}
      else if (nested->value[k] == '\\')
	{
	  k++;
	  switch (nested->value[k] | 32)
	    {
	    case 'f':
	      decoded[decodedLength++] = 12;
	      break;
	    case 'n':
	      decoded[decodedLength++] = 10;
	      break;
	    case 'r':
	      decoded[decodedLength++] = 13;
	      break;
	    default:
	      configureError (nested, "invalid value '%s'", nested->value);
	      return 0;
	    }
	  k++;
	}
      else
	decoded[decodedLength++] = nested->value[k++];
    }
  decoded[decodedLength] = 0;
  strcpy (nested->value, decoded);
  nested->valueLength = decodedLength;
  return 1;
}

static int compileConfig (lbu_FileInfo * nested);

int
config_compileSettings (const char *fileName)
{
/*Compile an input file or string */
  lbu_FileInfo nested;
  char completePath[MAXNAMELEN];
  if (!*fileName)
    return 1;			/*Probably run with defaults */
  nested.fileName = fileName;
  nested.lineNumber = 0;
  nested.stringPos = 1;
  if (nested.fileName[0] == ud->string_escape)
    return compileConfig (&nested);
  if (!find_file (fileName, completePath))
    {
      configureError (NULL, "Can't find configuration file '%s'", fileName);
      return 0;
    }
  if ((nested.in = fopen ((char *) completePath, "rb")))
    {
      compileConfig (&nested);
      fclose (nested.in);
    }
  else
    {
      configureError (NULL, "Can't open configuration file '%s'", fileName);
      return 0;
    }
  return 1;
}

static int
getLine (lbu_FileInfo * nested)
{
  int lineLen = 0;
  int ch;
  if (nested->fileName[0] != ud->string_escape)
    {
      int pch = 0;
      while ((ch = fgetc (nested->in)) != EOF)
	{
	  if (ch == 13)
	    continue;
	  if (pch == '\\' && ch == 10)
	    {
	      lineLen--;
	      continue;
	    }
	  if (ch == 10 || lineLen > (sizeof (nested->line) - 2))
	    break;
	  nested->line[lineLen++] = ch;
	  pch = ch;
	}
      nested->line[lineLen] = 0;
      if (ch == EOF)
	return 0;
      return 1;
    }
  if (nested->configString[nested->stringPos] == 0)
    return 0;
  while ((ch = nested->configString[nested->stringPos]))
    {
      nested->line[lineLen++] = ch;
      nested->stringPos++;
      if (ch == 10 || ch == 13)
	break;
    }
  nested->line[lineLen] = 0;
  return 1;
}

static int
parseLine (lbu_FileInfo * nested)
{
  char *curchar = NULL;
  int ch = 0;
  nested->action = "";
  nested->value = "";
  nested->value2 = "";
  while (getLine (nested))
    {
      nested->lineNumber++;
      curchar = nested->line;
      while ((ch = *curchar++) <= 32 && ch != 0);
      if (ch == 0 || ch == '#' || ch == '<')
	continue;
      nested->action = curchar - 1;
      while ((ch = *curchar++) > 32 && ch != '=');
      nested->actionLength = curchar - nested->action - 1;
      nested->action[nested->actionLength] = 0;
      while (((ch = *curchar++) <= 32 || ch == '=') && ch != 0);
      if (ch == 0)
	{
	  nested->value = "";
	  return 1;
	}
      else
	{
	  nested->value = curchar - 1;
	  if (*nested->value == 34)	/*quote */
	    {
	      nested->value++;
	      while (*curchar && *curchar != 34)
		curchar++;
	      nested->valueLength = curchar - nested->value;
	    }
	  else
	    {
	      while (*curchar++ > 32);
	      nested->valueLength = curchar - nested->value - 1;
	    }
	  nested->value[nested->valueLength] = 0;
	}
      while ((ch = *curchar++) <= 32 && ch != 0);
      if (ch != 0)
	{
	  nested->value2 = curchar - 1;
	  if (*nested->value2 == 34)	/*quote */
	    {
	      nested->value2++;
	      while (*curchar && *curchar != 34)
		curchar++;
	      nested->value2Length = curchar - nested->value2;
	    }
	  else
	    {
	      while (*curchar++ > 32);
	      nested->value2Length = curchar - nested->value2 - 1;
	    }
	  nested->value2[nested->value2Length] = 0;
	}
      else
	nested->value2 = "";
      return 1;
    }
  return 0;
}

#define NOTFOUND 1000
static int mainActionNumber = NOTFOUND;
static int subActionNumber;
static int entities = 0;

int
ignore_case_comp (const char *str1, const char *str2, int length)
{
/* Replaces strncasecmp, which some compilers don't support */
  int k;
  for (k = 0; k < length; k++)
    if ((str1[k] | 32) != (str2[k] | 32))
      break;
  if (k != length)
    return 1;
  return 0;
}

int
find_action (const char **actions, const char *action)
{
  int actionLength = strlen (action);
  int k;
  for (k = 0; actions[k]; k += 2)
    if (actionLength == strlen (actions[k])
	&& ignore_case_comp (actions[k], action, actionLength) == 0)
      break;
  if (actions[k] == NULL)
    return -1;
  return atoi (actions[k + 1]);
}

static int
checkActions (lbu_FileInfo * nested, const char **actions)
{
  int actionNum = find_action (actions, nested->action);
  if (actionNum == -1)
    return NOTFOUND;
  return actionNum;
}

static int
checkValues (lbu_FileInfo * nested, const char **values)
{
  int k;
  for (k = 0; values[k]; k += 2)
    if (nested->valueLength == strlen (values[k]) &&
	ignore_case_comp (values[k], nested->value, nested->valueLength) == 0)
      break;
  if (values[k] == NULL)
    {
      configureError (nested, "word '%s' in column 2 not recognized",
		      nested->value);
      return NOTFOUND;
    }
  return atoi (values[k + 1]);
}

static unsigned int
hexValue (lbu_FileInfo * nested, const char *digits)
{
  int length = strlen (digits);
  int k;
  unsigned int binaryValue = 0;
  for (k = 0; k < length; k++)
    {
      unsigned int hexDigit = 0;
      if (digits[k] >= '0' && digits[k] <= '9')
	hexDigit = digits[k] - '0';
      else if (digits[k] >= 'a' && digits[k] <= 'f')
	hexDigit = digits[k] - 'a' + 10;
      else if (digits[k] >= 'A' && digits[k] <= 'F')
	hexDigit = digits[k] - 'A' + 10;
      else
	{
	  configureError (nested, "invalid %d-digit hexadecimal number",
			  length);
	  return 0xffffffff;
	}
      binaryValue |= hexDigit << (4 * (length - 1 - k));
    }
  return binaryValue;
}

static unsigned int
convertValue (lbu_FileInfo * nested, const char *number)
{
  if (number[0] == '0' && number[1] == 'x')
    return hexValue (nested, &number[2]);
  else if (number[0] == '1' && number[1] == '<')
    {
      int shift = atoi (&number[3]);
      return 1 << shift;
    }
  else
    return atoi (number);
}

static int
orValues (lbu_FileInfo * nested, const char **values)
{
  int result = 0;
  int k;
  int word = 0;
  int wordLength;
  while (word < nested->valueLength)
    {
      for (wordLength = 0; (word + wordLength) < nested->valueLength &&
	   nested->value[word + wordLength]
	   > ' ' && nested->value[word + wordLength] != ','; wordLength++);
      for (k = 0; values[k]; k += 2)
	if (wordLength == strlen (values[k]) &&
	    ignore_case_comp (values[k], &nested->value[word], wordLength) == 0)
	  {
	    result |= convertValue (nested, values[k + 1]);
	    break;
	  }
      word += wordLength + 1;
    }
  if (result == 0)
    {
      configureError (nested, "word '%s' in column 2 not recognized",
		      nested->value);
      return NOTFOUND;
    }
  return result;
}

static int
checkSubActions (lbu_FileInfo * nested, const char **mainActions, const char
		 **subActions)
{
  int subAction;
  mainActionNumber = NOTFOUND;
  subAction = checkActions (nested, subActions);
  if (subAction != NOTFOUND && nested->value == "")
    {
      configureError (nested, "column 2 is required");
      return NOTFOUND;
    }
  if (subAction == NOTFOUND)
    {
      mainActionNumber = checkActions (nested, mainActions);
      if (mainActionNumber == NOTFOUND)
	configureError (nested,
			"word '%s' in column 1 not recognized",
			nested->action);
      return NOTFOUND;
    }
  return (subActionNumber = subAction);
}

static int
compileConfig (lbu_FileInfo * nested)
{
  static const char *mainActions[] = {
    "outputFormat",
    "0",
    "translation",
    "0",
    "xml",
    "0",
    "cellsPerLine",
    "1",
    "linesPerPage",
    "2",
    "interpoint",
    "3",
    "lineEnd",
    "4",
    "pageEnd",
    "5",
    "beginningPageNumber",
    "6",
    "braillePages",
    "7",
    "paragraphs",
    "8",
    "fileEnd",
    "9",
    "printPages",
    "10",
    "printPageNumberAt",
    "11",
    "braillePageNumberAt",
    "12",
    "hyphenate",
    "13",
    "outputEncoding",
    "14",
    "encoding",
    "14",
    "backFormat",
    "15",
    "backLineLength",
    "16",
    "contractedTable",
    "18",
    "literarytextTable",
    "18",
    "editTable",
    "19",
    "uncontractedTable",
    "20",
    "compbrlTable",
    "21",
    "mathtextTable",
    "22",
    "mathexprTable",
    "23",
    "topMargin",
    "24",
    "xmlHeader",
    "25",
    "entity",
    "26",
    "internetAccess",
    "27",
    "semanticFiles",
    "28",
    "newEntries",
    "29",
    "include",
    "30",
    "formatFor",
    "31",
    "inputTextEncoding",
    "32",
    "contents",
    "33",
    "linefill",
    "34",
    "debug",
    "35",
    "leftMargin",
    "36",
    "pass2convsem",
    "37",
    "converterSem",
    "38",
    "braillePageNumber",
    "39",
    "mergeUnnumberedPages",
    "40",
    "pageNumberTopSeparateLine",
    "41",
    "pageNumberBottomSeparateLine",
    "42",
    "printPageNumberRange",
    "43",
    "pageSeparator",
    "44",
    "pageSeparatorNumber",
    "45",
    "ignoreEmptyPages",
    "46",
    "continuePages",
    "47",
    "emphasis",
    "48",
    "paperWidth",
    "49",
    "paperHeight",
    "50",
    "numberBraillePages",
    "51",
    "printPageNumbersInContents",
    "52",
    "braillePageNumbersInContents",
    "53",
    "rightMargin",
    "54",
    "bottomMargin",
    "55",
    "mode",
    "56",
    "pageNumberTable",
    "57",
	"Endnotes",
	"58",
    "minSyllableLength",
    "59",
    "macro",
    "60",
    "style",
    "90",
    NULL
  };
  static const char *yesNo[] = {
    "no", "0", "yes", "1", NULL
  };
  static const char *topBottom[] = {
    "bottom", "0", "top", "1", NULL
  };
  static const char *encodings[] = {
    "utf8", "0", "utf16", "1", "utf32", "2", "ascii8", "3", NULL
  };
  static const char *backFormats[] = {
    "plain", "0", "html", "1", NULL
  };
  static const char *hyphenationModes[] = {
      "no", "0", "yes", "1", "pre", "2", NULL
  };

  static const char *formatFor[] = {
    "textDevice", "0",
    "browser", "1",
    "utd", "2",
    "pef", "3",
    "transInXml", "4",
    "volumes", "5",
    "brf", "6",
    "volumesPef", "7", 
    "volumesBrf", "8",
    "dsbible", "9",
    NULL
  };

  static const char *typeForms[] = {
    "italic", "1",
    "underline", "2",
    "bold", "4",
    "computer_braille", "8",
    "all", "15",
    "none", "0x100",
    NULL
  };
  static const char *configModes[] = {
    /*liblouis modes */
    "noContractions", "1",
    "compbrlAtCursor", "2",
    "dotsIO", "4",
    "comp8Dots", "8",
    "pass1Only", "16",
    "compbrlLeftCursor", "32",
    "otherTrans", "64",
    "ucBrl", "128",
    /*liblouisutdml modes */
    "doInit", "1<<30",
    "htmlDoc", "1<<29",
    "notUC", "1<<28",
    "notSync", "1<<27",
    "utdInput", "1<<26",
    "convertOnly", "1<<25",
    "louisDots", "1<<24",
    NULL
  };

  int k;
  while (parseLine (nested))
    {
      mainActionNumber = checkActions (nested, mainActions);
      if (mainActionNumber == NOTFOUND)
	{
	  configureError (nested,
			  "word '%s' in column 1 not recognized",
			  nested->action);
	  continue;
	}
    choseMainAction:
      switch (mainActionNumber)
	{
	case NOTFOUND:
	case 0:
	  break;
	case 1:
	  ud->cells_per_line = atoi (nested->value);
	  break;
	case 2:
	  ud->lines_per_page = atoi (nested->value);
	  break;
	case 3:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->interpoint = k;
	  break;
	case 4:
	  if (controlCharValue (nested))
	    memcpy (ud->lineEnd, nested->value, nested->valueLength + 1);
	  break;
	case 5:
	  if (controlCharValue (nested))

	    memcpy (ud->pageEnd, nested->value, nested->valueLength + 1);
	  break;
	case 6:
	  ud->beginning_braille_page_number = atoi (nested->value);
	  break;
	case 7:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->braille_pages = k;
	  break;
	case 8:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->paragraphs = k;
	  break;
	case 9:
	  if (controlCharValue (nested))
	    memcpy (ud->fileEnd, nested->value, nested->valueLength + 1);
	  break;
	case 10:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->print_pages = k;
	  break;
	case 11:
	  if ((k = checkValues (nested, topBottom)) != NOTFOUND)
	    ud->print_page_number_at = k;
	  break;
	case 12:
	  if ((k = checkValues (nested, topBottom)) != NOTFOUND)
	    {
	      if (k)
		k = 0;
	      else
		k = 1;
	      ud->braille_page_number_at = k;
	    }
	  break;
	case 13:
	  if ((k = checkValues (nested, hyphenationModes)) != NOTFOUND) {
	    ud->hyphenate = k;
	    if (ud->hyphenate == 1)
	      // "yes" -> combination of "yes" and "pre"
	      ud->hyphenate += 2;
	  }
	  break;
	case 14:
	  if ((k = checkValues (nested, encodings)) != NOTFOUND)
	    ud->output_encoding = k;
	  break;
	case 15:
	  if ((k = checkValues (nested, backFormats)) != NOTFOUND)
	    ud->back_text = k;
	  break;
	case 16:
	  ud->back_line_length = atoi (nested->value);
	  break;
	case 18:
	  ud->contracted_table_name = findTable (nested);
	  if (ud->contracted_table_name == NULL)
	  {
	  configureError (nested, "invalid literaryTextTable");
	  return 0;
	  }
	  break;
	case 19:
	  ud->edit_table_name = findTable (nested);
	  if (ud->edit_table_name == NULL)
	  {
	  configureError (nested, "invalid editTable");
	  return 0;
	  }
	  break;
	case 20:
	  ud->uncontracted_table_name = findTable (nested);
	  if (ud->uncontracted_table_name == NULL)
	  {
	  configureError (nested, "invalid uncontractedTableName");
	  return 0;
	  }
	  break;
	case 21:
	  ud->compbrl_table_name = findTable (nested);
	  if (ud->compbrl_table_name == NULL)
	  {
	  configureError (nested, "invalid compbrlTableName");
	  return 0;
	  }
	  break;
	case 22:
	  ud->mathtext_table_name = findTable (nested);
	  if (ud->mathtext_table_name == NULL)
	  {
	  configureError (nested, "invalid mathtextTableName");
	  return 0;
	  }
	  break;
	case 23:
	  ud->mathexpr_table_name = findTable (nested);
	  if (ud->mathexpr_table_name == NULL)
	  {
	  configureError (nested, "invalid mathexprTableName");
	  return 0;
	  }
	  break;
	case 24:
	  topMargin = atof (nested->value);
	  break;
	case 25:
	  if (entities)
	    {
	      configureError
		(nested,
		 "The header definition must precede all entity definitions.");
	      break;
	    }
	  strcpy (ud->xml_header, nested->value);
	  break;
	case 26:
	  if (nested->value == "")
	    break;
	  if (!entities)
	    strcat (ud->xml_header, "<!DOCTYPE entities [\n");
	  entities = 1;
	  strcat (ud->xml_header, "<!ENTITY ");
	  strcat (ud->xml_header, nested->value);
	  strcat (ud->xml_header, " \"");
	  strcat (ud->xml_header, nested->value2);
	  strcat (ud->xml_header, "\">\n");
	  break;
	case 27:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->internet_access = k;
	  break;
	case 28:
	  ud->semantic_files = alloc_string (nested->value);
	  break;
	case 29:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->new_entries = k;
	  break;
	case 30:
	  {
	    static const char *actions[] = {
	      NULL
	    };
	    if (nested->value == "")
	      configureError (nested, "a file name in column 2 is required");
	    else
	      config_compileSettings (nested->value);
	    if (!parseLine (nested))
	      break;
	    checkSubActions (nested, mainActions, actions);
	    if (mainActionNumber != NOTFOUND)
	      goto choseMainAction;
	  }
	  break;
	case 31:
	  if ((k = checkValues (nested, formatFor)) != NOTFOUND)
	    ud->format_for = k;
	  break;
	case 32:
	  if ((k = checkValues (nested, encodings)) != NOTFOUND)
	    ud->input_text_encoding = k;
	  break;
	case 33:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->contents = k;
	  break;
	case 34:
	  if (nested->value == "")
	    ud->line_fill = ' ';
	  else
	    ud->line_fill = nested->value[0];
	  break;
	case 35:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->debug = k;
	  break;
	case 36:
	  leftMargin = atof (nested->value);
	  break;
    case 37:
      ud->pass2_conv_sem = alloc_string (nested->value);
      break;
	case 38:
	  ud->converter_sem = alloc_string (nested->value);
	  break;
	case 39:
	  ud->braille_page_number = atoi (nested->value);
	  break;
	case 40:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->merge_unnumbered_pages = k;
	  break;
	case 41:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->page_number_top_separate_line = k;
	  break;
	case 42:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->page_number_bottom_separate_line = k;
	  break;
	case 43:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->print_page_number_range = k;
	  break;
	case 44:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->page_separator = k;
	  break;
	case 45:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->page_separator_number = k;
	  break;
	case 46:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->ignore_empty_pages = k;
	  break;
	case 47:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->continue_pages = k;
	  break;
	case 48:
	  if ((k = orValues (nested, typeForms)) != NOTFOUND)
	    ud->emphasis = k;
	  break;
	case 49:
	  paperWidth = atof (nested->value);
	  break;
	case 50:
	  paperHeight = atof (nested->value);
	  break;
	case 51:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    {
	      ud->number_braille_pages = k;
	    }
	  break;
	case 52:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->print_page_numbers_in_contents = k;
	  break;
	case 53:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->braille_page_numbers_in_contents = k;
	  break;
	case 54:
	  rightMargin = atof (nested->value);
	  break;
	case 55:
	  bottomMargin = atof (nested->value);
	  break;
	case 56:
	  if ((k = orValues (nested, configModes)) != NOTFOUND)
	    ud->config_mode = k;
	  break;
	case 57:
	  ud->pagenum_table_name = findTable (nested);
	  if (ud->pagenum_table_name == NULL)
	  {
	  configureError (nested, "invalid pageNumberTable");
	  return 0;
	  }
	  break;
	case 58:
	  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
	    ud->endnotes = k;
	  break;
	case 59:
	  ud->min_syllable_length = atoi (nested->value);
	  break;
	case 60:
	  new_macro (nested->value, nested->value2);
	  break;
	case 90:
	  {
	    static const char *actions[] = {
	      "linesBefore",
	      "0",
	      "linesAfter",
	      "1",
	      "leftMargin",
	      "2",
	      "firstLineIndent",
	      "3",
	      "translationTable",
	      "6",
	      "skipNumberLines",
	      "7",
	      "format",
	      "8",
	      "newPageBefore",
	      "9",
	      "newPageAfter",
	      "10",
	      "rightHandPage",
	      "11",
	      "braillePageNumberFormat",
	      "12",
	      "rightMargin",
	      "13",
	      "keepWithNext",
	      "14",
	      "dontSplit",
	      "15",
	      "orphanControl",
	      "16",
	      "newlineAfter",
	      "17",
	      "runningHead",
	      "18",
	      "emphasis",
	      "19",
              "topBoxline",
              "20",
              "bottomBoxline",
              "21",
	      NULL
	    };
	    static const char *formats[] = {
	      "inherit",
	      "-100",
	      "leftJustified",
	      "0",
	      "rightJustified",
	      "1",
	      "centered",
	      "2",
	      "alignColumnsLeft",
	      "3",
	      "alignColumnsRight",
	      "4",
	      "listColumns",
	      "5",
	      "listLines",
	      "6",
	      "computerCoded",
	      "7",
	      "contents",
	      "8",
	      NULL
	    };
	    static const char *pageNumFormats[] = {
	      "normal",
	      "0",
	      "blank",
	      "1",
	      "p",
	      "2",
	      "roman",
	      "3",
	      "romancaps",
	      "4",
	      NULL
	    };
	    StyleType *style;
	    sem_act styleAction;
	    if (nested->value == "")
	      {
		configureError (nested,
				"no style name given in second column");
		break;
	      }
	    styleAction = find_semantic_number (nested->value);
	    style = new_style (nested->value);
	    style->action = styleAction;
	    style->newline_after = 1;
	    while (parseLine (nested))
	      {
		checkSubActions (nested, mainActions, actions);
		if (mainActionNumber != NOTFOUND)
		  goto choseMainAction;
		switch (subActionNumber)
		  {
		  case NOTFOUND:
		    break;
		  case 0:
		    style->lines_before = atoi (nested->value);
		    break;
		  case 1:
		    style->lines_after = atoi (nested->value);
		    break;
		  case 2:
		    style->left_margin = atoi (nested->value);
		    break;
		  case 3:
		    style->first_line_indent = atoi (nested->value);
		    break;
		  case 6:
		    style->translation_table = findTable (nested);
		    break;
		  case 7:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->skip_number_lines = k;
		    break;
		  case 8:
		    if ((k = checkValues (nested, formats)) != NOTFOUND)
		      style->format = k;
		    break;
		  case 9:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->newpage_before = k;
		    break;
		  case 10:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->newpage_after = k;
		    break;
		  case 11:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->righthand_page = k;
		    break;
		  case 12:
		    if ((k = checkValues (nested, pageNumFormats)) !=
			NOTFOUND)
		      style->brlNumFormat = k;
		    break;
		  case 13:
		    style->right_margin = atoi (nested->value);
		    break;
		  case 14:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->keep_with_next = k;
		    break;
		  case 15:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->dont_split = k;
		    break;
		  case 16:
		    style->orphan_control = atoi (nested->value);
		    break;
		  case 17:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->newline_after = k;
		    break;
		  case 18:
		    if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		      style->runningHead = k;
		    break;
		  case 19:
		  {
		  sem_act action = find_semantic_number (nested->value);
		  if (action == italicx || action == boldx || action == 
		  underlinex || action == compbrl)
		  style->emphasis = action;
		  else
		  configureError (nested, "invalid emphasis '%s'", 
		  nested->value);
		  }
		  break;
                  case 20:
                    memcpy(style->topBoxline, nested->value, 1);
                    break;
                  case 21:
                    memcpy(style->bottomBoxline, nested->value, 1);
                    break;
		  default:
		    configureError (nested, "Program error in readconfig.c");
		    continue;
		  }
	      }
	    break;
	  }
	default:
	  configureError (nested, "Program error in readconfig.c");
	  continue;
	}
    }
  return 1;
}

static int
initConfigFiles (const char *firstConfigFile, char *fileName, const
		 char *logFileName)
{
  char configPath[MAXNAMELEN];
  int k;
  strcpy (configPath, firstConfigFile);
  for (k = strlen (configPath); k >= 0; k--)
    if (configPath[k] == ud->file_separator)
      break;
  strcpy (fileName, &configPath[k + 1]);
  if (k < 0)
    k++;
  configPath[k] = 0;
  set_paths (configPath);
  if (logFileName && logFileName[0] != 0)
    {
      strcpy ((char *) ud->typeform, lbu_getWriteablePath ());
      strcat ((char *) ud->typeform, logFileName);
      lbu_logFile ((char *) ud->typeform);
    }
  if (!config_compileSettings ("liblouisutdml.ini"))
    return 0;
  return 1;
}

int
read_configuration_file (const char *configFileList, const char
			 *logFileName,
			 const char *configString, unsigned int mode)
{
/* read the configuration files and perform other initialization*/
  int k;
  char mainFile[MAXNAMELEN];
  char subFile[MAXNAMELEN];
  int listLength;
  int currentListPos = 0;
  logMessage(LOU_LOG_DEBUG, "Begin read_configuration_file");
  errorCount = 0;
  fatalErrorCount = 0;
  /*Process logFileName later, after writeablePath is set */
  paperWidth = 0;
  paperHeight = 0;
  leftMargin = 0;
  rightMargin = 0;
  topMargin = 0;
  bottomMargin = 0;
  destroy_semantic_table ();
  if (fatalErrorCount)
    {
      configureError (NULL, "%d fatal errors found", fatalErrorCount);
      return 0;
    }
  if (ud == NULL)
    {
      if (!(ud = malloc (sizeof (UserData))))
        memoryError ();
        }
  memset (ud, 0, sizeof (UserData));
  ud->outbuf1_len = (sizeof (ud->outbuf1) / CHARSIZE) - 4;
  ud->outbuf2_len = (sizeof (ud->outbuf2) / CHARSIZE) - 4;
  ud->outbuf3_len = (sizeof (ud->outbuf3) / CHARSIZE) - 4;
  entities = 0;
  ud->mode = mode;
  ud->top = -1;
  ud->style_top = -1;
  ud->emphasis = 15;
  /* There will be configuration settings for these values */
  ud->cell_width = 5;
  ud->normal_line = 8;
  ud->wide_line = 10;
  ud->dpi = 20.0;
  strcpy (ud->lit_hyphen, "-");
  strcpy (ud->comp_hyphen, "_&");
  strcpy (ud->letsign, "\\_");
  /*End of values */
  for (k = document; k < notranslate; k++)
    {
      StyleType *style = new_style ((xmlChar *) semNames[k]);
      style->action = k;
    }
  ud->input_encoding = lbu_utf8;
  ud->output_encoding = lbu_ascii8;
  *ud->print_page_number = '_';
  *ud->print_page_number_first = '_';
  ud->string_escape = ',';
#ifdef _WIN32
  ud->file_separator = '\\';
#else
  ud->file_separator = '/';
#endif
  pathEnd[0] = ud->file_separator;
  pathEnd[1] = 0;

/*Process file list*/
  if (configFileList == NULL)
    {
      set_paths (NULL);
  if (logFileName  && logFileName[0] != 0)
    {
      strcpy ((char *) ud->typeform, lbu_getWriteablePath ());
      strcat ((char *) ud->typeform, logFileName);
      lbu_logFile ((char *) ud->typeform);
    }
      if (!(config_compileSettings ("liblouisutdml.ini")))
	return 0;
    }
  else
    {
      listLength = strlen (configFileList);
      for (k = 0; k < listLength; k++)
	if (configFileList[k] == ',')
	  break;
      if (k == listLength || k == 0)
	{			/* Only one file */
	  initConfigFiles (configFileList, mainFile, logFileName);
	  config_compileSettings (mainFile);
	}
      else
	{			/* Compile a list of files */
	  strncpy (subFile, configFileList, k);
	  subFile[k] = 0;
	  initConfigFiles (subFile, mainFile, logFileName);
	  currentListPos = k + 1;
	  config_compileSettings (mainFile);
	  while (currentListPos < listLength)
	    {
	      for (k = currentListPos; k < listLength; k++)
		if (configFileList[k] == ',')
		  break;
	      strncpy (subFile,
		       &configFileList[currentListPos], k - currentListPos);
	      subFile[k - currentListPos] = 0;
	      config_compileSettings (subFile);
	      currentListPos = k + 1;
	    }
	}
    }

/* Process configString */
  if (configString != NULL)
    {
      if (configString[0] == ud->string_escape)
	config_compileSettings (configString);
      else
	{
	  k = 0;
	  ud->typeform[k++] = ud->string_escape;
	  ud->typeform[k] = 0;
	  strcat ((char *) ud->typeform, configString);
	  config_compileSettings ((char *) ud->typeform);
	}
    }
  memset (ud->typeform, 0, sizeof (ud->typeform));
  ud->braille_page_number = ud->beginning_braille_page_number;
  ud->page_number = 1;
  if (entities)
    strcat (ud->xml_header, "]>\n");
  ud->mode = mode | ud->config_mode;
  ud->orig_format_for = ud->format_for;
  if (ud->format_for > utd || (ud->mode & louisDots))
    {
      ud->mode &= ~notUC;
      ud->format_for = utd;
    }
  if (ud->format_for == utd)
    {
      ud->braille_pages = 1;
      ud->paper_width = (int) (paperWidth * ud->dpi);
      ud->paper_height = (int) (paperHeight * ud->dpi);
      ud->left_margin = (int) (leftMargin * ud->dpi);
      ud->right_margin = (int) (rightMargin * ud->dpi);
      ud->top_margin = (int) (topMargin * ud->dpi);
      ud->bottom_margin = (int) (bottomMargin * ud->dpi);
    }
  else
    {
      ud->left_margin = (int) leftMargin;
    }
  ud->page_left = ud->left_margin;
  ud->page_right = ud->paper_width - ud->right_margin;
  ud->page_top = ud->top_margin;
  ud->page_bottom = ud->paper_height - ud->bottom_margin;
  if (ud->format_for == utd)
    {
      if (ud->page_right <= 0 || ud->page_bottom <= 0)
	{
	  logMessage
	    (LOU_LOG_ERROR, "For UTDML paper witdth and paper height must be specified.");
	  lbu_free ();
	  return 0;
	}
      ud->cells_per_line = (ud->page_right - ud->page_left) / ud->cell_width;
      ud->lines_per_page = (ud->page_bottom - ud->page_top) / 
      ud->normal_line;
      ud->back_text = textDevice;
      ud->back_line_length = 70;
    }
  ud->outbuf2_enabled = ud->braille_pages &&
      ud->print_pages &&
      ud->print_page_number_range &&
      ud->print_page_number_at;
  logMessage(LOU_LOG_DEBUG, "Finish read_configuration_file");
  return 1;
}
