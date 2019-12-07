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
#include <ctype.h>
#include "louisutdml.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "sem_names.h"

typedef struct
{
  const char *fileName;
  FILE *in;
  int lineNumber;
  int numEntries;
  int unedited;
  char line[5 * MAXNAMELEN];
}
lbu_FileInfo;

#define HASHSIZE 383
#define MAXINSERTS 256
typedef struct
{
  short int numInserts;
  short int lastInsert;
  short int numChars;
  widechar charInserts[MAXINSERTS];
} InsertsType;

typedef enum
{
  styleEntry = 1,
  xpathEntry = 2,
  actionEntry = 4,
  macroEntry = 8
} EntryType;

typedef struct
{
  void *next;
  unsigned char *key;
  EntryType type;
  int semNum;
  InsertsType *inserts;
  StyleType *style;
  unsigned char *macro;
} HashEntry;

typedef struct
{
  int curBucket;
  HashEntry *curEntry;
  HashEntry *entries[HASHSIZE];
} HashTable;

static int notFound = -1;
static HashTable *semanticTable = NULL;
static HashTable *newEntriesTable = NULL;
static int errorCount = 0;
static const xmlChar *rootName;
static xmlXPathContext *xpathCtx = NULL;
static int registerNamespaces (lbu_FileInfo * Nested, xmlXPathContextPtr
			       xpathCtx, const xmlChar * nsList);
static void
semanticError (lbu_FileInfo * nested, char *format, ...)
{
  char buffer[MAXNAMELEN];
  va_list arguments;
  va_start (arguments, format);
#ifdef WIN32
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

static unsigned int
semStringHash (const unsigned char *s)
{
  int k;
  unsigned int h = 0, g;
  for (k = 0; s[k]; k++)
    {
      h = (h << 4) + s[k];
      if ((g = h & 0xf0000000))
	{
	  h = h ^ (g >> 24);
	  h = h ^ g;
	}
    }
  return h;
}

static HashTable *
hashNew ()
{
  HashTable *table;
  table = malloc (sizeof (HashTable));
  if (!table)
    memoryError ();
  memset (table, 0, sizeof (HashTable));
  table->curBucket = -1;
  return table;
}

static void
hashFree (HashTable * table)
{
  int i;
  HashEntry *e, *next;
  if (table == NULL)
    return;
  for (i = 0; i < HASHSIZE; i++)
    for (e = table->entries[i]; e; e = next)
      {
	next = e->next;
	free (e->key);
	if (e->inserts != NULL)
	  free (e->inserts);
	if (e->type & styleEntry)
	  free (e->style);
	free (e);
      }
  free (table);
}

static void hashInsert (HashTable * table, const unsigned char *key, int
			type, int
			semNum, InsertsType * inserts, StyleType *
			style, unsigned char *macro);

void
clean_semantic_table ()
{
  int i;
  HashEntry *e, *next;
  HashTable *newSemanticTable;
  if (semanticTable == NULL)
    return;
  newSemanticTable = hashNew ();
  for (i = 0; i < HASHSIZE; i++)
    for (e = semanticTable->entries[i]; e; e = next)
      {
	next = e->next;
	if (!(e->type == 0 || e->type == xpathEntry))
	  hashInsert (newSemanticTable, e->key, e->type, e->semNum, NULL,
		      e->style, e->macro);
	free (e->key);
	if (e->inserts != NULL)
	  free (e->inserts);
	free (e);
      }
  free (semanticTable);
  semanticTable = newSemanticTable;
}

static HashEntry *
hashScan (HashTable * table)
{
/* This function should be called in a statement like
* while ((curEnt=y = hashScan (tablename)))
* where
* HasEntry *curEntry;
/ It returns a table entry at each call, in no order. When it reaches 
* the end of the table it resets itself and returns NULL. */
  HashEntry *e;
  if (table == NULL)
    return NULL;
  if (table->curBucket == -1)
    table->curEntry = NULL;
  while (table->curBucket < HASHSIZE)
    {
      if (table->curEntry != NULL)
	{
	  e = table->curEntry;
	  table->curEntry = e->next;
	  return e;
	}
      else
	table->curBucket++;
      while (table->curBucket < HASHSIZE && table->entries[table->curBucket]
	     == NULL)
	table->curBucket++;
      if (table->curBucket < HASHSIZE)
	table->curEntry = table->entries[table->curBucket];
    }
  table->curBucket = -1;
  table->curEntry = NULL;
  return NULL;
}

static HashEntry *latestEntry;

/* assumes that key is not already present! */
static void
hashInsert (HashTable * table, const unsigned char *key, int type, int
	    semNum, InsertsType * inserts, StyleType * style, unsigned
	    char *macro)
{
  int i;
  if (table == NULL || key == NULL || *key == 0)
    return;
  i = semStringHash (key) % HASHSIZE;
  latestEntry = malloc (sizeof (HashEntry));
  if (!latestEntry)
    memoryError ();
  latestEntry->next = table->entries[i];
  latestEntry->key = malloc (strlen ((char *) key) + 2);
  if (!latestEntry->key)
    memoryError ();
  strcpy ((char *) latestEntry->key, (char *) key);
  latestEntry->type = type;
  latestEntry->semNum = semNum;
  latestEntry->inserts = inserts;
  latestEntry->style = style;
  latestEntry->macro = macro;
  table->entries[i] = latestEntry;
}

static int
hashLookup (HashTable * table, const unsigned char *key)
{
  int i;
  int keyLength;
  int entryKeyLength;
  int k;
  if (table == NULL || key == NULL || *key == 0)
    return notFound;
  keyLength = strlen ((char *) key);
  i = semStringHash (key) % HASHSIZE;
  for (latestEntry = table->entries[i]; latestEntry; latestEntry =
       latestEntry->next)
    {
      entryKeyLength = strlen ((char *) latestEntry->key);
      if (entryKeyLength != keyLength)
	continue;
      for (k = 0; k < keyLength; k++)
	if (key[k] != latestEntry->key[k])
	  break;
      if (k == keyLength)
	return latestEntry->semNum;
    }
  return notFound;
}

static char firstFileName[MAXNAMELEN];
static int haveSemanticFile = 1;

#define SEMANTICSUF " mes"
sem_act
find_semantic_number (const char *name)
{
  static const char *pseudoActions[] = {
    "include",
    "newentries",
    "namespaces",
    NULL
  };
  int k;
  char realName[MAXNAMELEN];
  if (semanticTable == NULL)
    semanticTable = hashNew ();
  /* See if semantic names and numbers are already in table. */
  strcpy (realName, "no");
  strcat (realName, SEMANTICSUF);
  if (hashLookup (semanticTable, (xmlChar *) realName) == notFound)
    {
      /* Enter names and numbers in table before checking for this one. */
      for (k = 0; k < end_all; k++)
	{
	  strcpy (realName, semNames[k]);
	  strcat (realName, SEMANTICSUF);
	  hashInsert (semanticTable, (xmlChar *) realName, actionEntry, k,
		      NULL, NULL, NULL);
	}
      k = 0;
      while (pseudoActions[k] != NULL)
	{
	  strcpy (realName, pseudoActions[k]);
	  strcat (realName, SEMANTICSUF);
	  hashInsert (semanticTable, (xmlChar *) realName,
		      actionEntry, k + end_all + 1, NULL, NULL, NULL);
	  k++;
	}
    }
  /* Now look up the name we called this function with. */
  for (k = 0; name[k]; k++)
    realName[k] = name[k] | 32;
  realName[k] = 0;
  strcat (realName, SEMANTICSUF);
  return (hashLookup (semanticTable, (xmlChar *) realName));
}

void
destroy_semantic_table ()
{
  hashFree (semanticTable);
  semanticTable = NULL;
  hashFree (newEntriesTable);
  newEntriesTable = NULL;
  if (xpathCtx != NULL)
    xmlXPathFreeContext (xpathCtx);
  xpathCtx = NULL;
}

static widechar
hexValue (lbu_FileInfo * nested, const xmlChar * digits, int length)
{
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
	  semanticError (nested, "invalid %d-digit hexadecimal number",
			 length);
	  return (widechar) 0xffffffff;
	}
      binaryValue |= hexDigit << (4 * (length - 1 - k));
    }
  return (widechar) binaryValue;
}

static InsertsType *
encodeInsertions (lbu_FileInfo * nested, xmlChar * insertions, int length)
{
  int k = 0;
  int prevk = 0;
  xmlChar *oneIns = insertions;
  int oneLength = 0;
  InsertsType inserts;
  int insertsSize = sizeof (InsertsType) - (MAXINSERTS * CHARSIZE);
  InsertsType *insertsPtr;
  int kk;
  int startInsert;
  int sizeInsert;
  widechar ch;
  if (*insertions == 0 || length == 0)
    return NULL;
  inserts.numInserts = 0;
  inserts.numChars = 1;
  inserts.lastInsert = 0;
  k = 0;
  prevk = 0;
/*Inserjions are placed in inserts.charInserts and consist of a length 
* followed by the characters to be inserted. The length is one more than 
* the numbr of characters to make it simpler to step through the 
* insertions in the next function.*/
  while (k < length)
    {
      if (insertions[k] == '\\' && insertions[k + 1] == ',')
	{
	  k += 2;
	  continue;
	}
      if (insertions[k] == ',' || k == (length - 1))
	{
	  oneLength = k - prevk;
	  if (k == (length - 1))
	    oneLength++;
	  if (insertions[prevk] == '\\' && insertions[prevk + 1] == '*')
	    {
	      inserts.lastInsert = inserts.numChars;
	      if ((prevk + oneLength) < length)
		semanticError (nested,
			       "an insertion with \\* must be the last.");
	    }
	  else
	    inserts.numInserts++;
	  startInsert = inserts.numChars++;
	  sizeInsert = 0;
	  kk = 0;
	  while (kk < oneLength)
	    {
	      ch = (widechar) oneIns[kk++];
	      if (ch == '\\')
		switch (oneIns[kk])
		  {
		  case 's':
		    inserts.charInserts[inserts.numChars++] = ' ';
		    kk++;
		    break;
		  case 'e':
		    inserts.charInserts[inserts.numChars++] = 0x1b;
		    kk++;
		    break;
		  case ',':
		    inserts.charInserts[inserts.numChars++] = ',';
		    kk++;
		    break;
		  case 'x':
		  case 'X':
		    inserts.charInserts[inserts.numChars++] = hexValue
		      (nested, &oneIns[kk + 1], 4);
		    kk += 5;
		    break;
		  case 'y':
		  case 'Y':
		    if (CHARSIZE == 2)
		      {
		      not32:
			semanticError (nested,
				       "liblouisutdml has not been compiled for 32-bit Unicode");
			break;
		      }
		    if (oneLength - k > 5)
		      {
			inserts.charInserts[inserts.numChars++] =
			  hexValue (nested, &oneIns[k + 1], 5);
			k += 6;
		      }
		    break;
		  case 'z':
		  case 'Z':
		    if (CHARSIZE == 2)
		      goto not32;
		    if (oneLength - k > 8)
		      {
			inserts.charInserts[inserts.numChars++] =
			  hexValue (nested, &oneIns[k + 1], 8);
			k += 9;
		      }
		    break;
		  case '*':
		    kk++;
		    sizeInsert--;
		    break;
		  default:
		    kk++;
		    semanticError (nested, "invalid escape sequence.");
		    break;
		  }
	      else
		inserts.charInserts[inserts.numChars++] = ch;
	      sizeInsert++;
	    }
	  inserts.charInserts[startInsert] = sizeInsert + 1;
	  oneIns = &insertions[k + 1];
	  prevk = k + 1;
	}
      k++;
    }
  insertsSize += inserts.numChars * CHARSIZE;
  insertsPtr = malloc (insertsSize);
  if (!insertsPtr)
    memoryError ();
  memcpy (insertsPtr, &inserts, insertsSize);
  return insertsPtr;
}

static int insertFromMacro (int which);

int
insert_code (xmlNode * node, int which)
{
  int entryStart;
  widechar *insertStart;
  int insertLength = 0;
  int k;
  int sumLength;
  InsertsType *inserts;
  HashEntry *nodeEntry;
  if (node == NULL)
    return 0;
  nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry == NULL)
    return 0;
  if (nodeEntry->macro)
    {
      insertFromMacro (which);
      return 1;
    }
  if (nodeEntry->inserts == NULL)
    return 1;
  inserts = nodeEntry->inserts;
  if (which == -1)
    {
      if ((entryStart = inserts->lastInsert) == 0)
	return 1;
      insertStart = &inserts->charInserts[entryStart + 1];
      insertLength = inserts->charInserts[entryStart] - 1;
    }
  else
    {
      if (inserts->numInserts == 0 || inserts->numInserts <= which)
	return 1;
      sumLength = 1;
      for (k = 0; k < which; k++)
	sumLength += inserts->charInserts[sumLength];
      insertStart = &inserts->charInserts[sumLength + 1];
      insertLength = inserts->charInserts[sumLength] - 1;
      if (insertLength <= 0)
	return 0;
    }
  if ((ud->text_length + insertLength) > MAX_LENGTH)
    return 0;
  memcpy (&ud->text_buffer[ud->text_length], insertStart, CHARSIZE *
	  insertLength);
  ud->text_length += insertLength;
  return 1;
}

#define NUMCOUNTS 1024
#define MAXNUMVAL 5
static int *attrValueCounts = NULL;
static HashTable *attrValueCountsTable;

static int
countAttrValues (xmlChar * key)
{
  int k;
  int numItems = 1;
  int lastComma = 0;
  static int curCount = 0;
  int thisCount = notFound;
  if (!ud->new_entries)
    return 0;
  if (attrValueCounts == NULL)
    {
      attrValueCounts = malloc (NUMCOUNTS * sizeof (int));
      if (!attrValueCounts)
	memoryError ();
      attrValueCountsTable = hashNew ();
      curCount = 0;
    }
  for (k = 0; key[k]; k++)
    if (key[k] == ',')
      {
	lastComma = k;
	numItems++;
      }
  switch (numItems)
    {
    case 1:
      return 1;
    case 2:
      if (hashLookup (attrValueCountsTable, key) != notFound)
	return 1;
      if (curCount >= NUMCOUNTS)
	return 0;
      hashInsert (attrValueCountsTable, key, 0, curCount, NULL, NULL, NULL);
      curCount++;
      return 1;
    case 3:
      if (curCount >= NUMCOUNTS)
	return 0;
      key[lastComma] = 0;
      thisCount = hashLookup (attrValueCountsTable, key);
      if (thisCount == notFound)
	{
	  attrValueCounts[curCount]++;
	  hashInsert (attrValueCountsTable, key, 0, curCount, NULL,
		      NULL, NULL);
	  curCount++;
	}
      key[lastComma] = ',';
      if (thisCount == notFound)
	return 1;
      if (attrValueCounts[thisCount] >= MAXNUMVAL)
	return 0;
      attrValueCounts[thisCount]++;
      return 1;
    default:
      return 0;
    }
  return 0;
}

static void
destroyattrValueCountsTable ()
{
  if (attrValueCounts == NULL)
    return;
  hashFree (attrValueCountsTable);
  free (attrValueCounts);
  attrValueCounts = NULL;
}
static int sem_compileFile (const char *fileName);

int
find_group_length (const char groupSym[2], const char *groupStart)
{
  int level = 0;
  int k;
  if (*groupStart != groupSym[0])
    return -1;
  for (k = 0; groupStart[k]; k++)
    {
      if (groupStart[k] == groupSym[0])
	level++;
      if (groupStart[k] == groupSym[1])
	level--;
      if (level == 0)
	return k + 1;
    }
  return -1;
}
static int
compileLine (lbu_FileInfo * nested)
{
  char *curchar = NULL;
  int ch = 0;
  int func = 0;
  char *funcName;
  int funcNameLength;
  char *argsStart;
  int argsLength;
  EntryType type = 0;
  char *action = NULL;
  int actionLength = 0;
  char *lookFor = NULL;
  int lookForLength = 0;
  char *insertions = NULL;
  int insertionsLength = 0;
  InsertsType *inserts = NULL;
  unsigned char *macro = NULL;
  StyleType *style = NULL;
  int actionNum = 0;
  if (semanticTable == NULL)
    semanticTable = hashNew ();
  curchar = nested->line;
  while ((ch = *curchar++) <= 32 && ch != 0);
  if (ch == 0 || ch == '#' || ch == '<')
    return 1;
  action = curchar - 1;
  while ((ch = *curchar++) > 32 || ch == '=');
  actionLength = curchar - action - 1;
  action[actionLength] = 0;
  if (!(actionLength == 2 && action[0] == 'n' && action[1] == 'o'))
    nested->unedited = 0;
  while (((ch = *curchar++) <= 32 || ch == '=') && ch != 0);
  if (ch == 0)
    {
      semanticError (nested, "Nothing to look for");
      return 0;
    }
  lookFor = curchar - 1;
  if (*lookFor == '&')
    {
      /*xpath or other special case */
      static const char *funcNames[] = {
	"xpath",
	"1",
	"now",
	"2",
	NULL
      };
      while ((ch = *curchar++) <= 32 && ch != 0);
      funcName = curchar - 1;
      while ((ch = *curchar++) > 32 && ch != '(');
      funcNameLength = curchar - funcName - 1;
      funcName[funcNameLength] = 0;
      func = find_action (funcNames, funcName);
      if (func < 0)
	{
	  semanticError (nested,
			 "function name '%s' in column 2 not recognized",
			 funcName);
	  return 0;
	}
    }
  if (!(func == 0 || func == 2))
    {
      funcName[funcNameLength] = ch;
      if (ch != '(')
	while ((ch = *curchar++) <= 32 && ch != 0);
      argsStart = curchar - 1;
      argsLength = find_group_length ("()", argsStart);
      if (argsLength < 0)
	{
	  semanticError (nested, "unmatched parentheses in column 2 '%s'",
			 lookFor);
	  return 0;
	}
      switch (func)
	{
	case 1:
	  type |= xpathEntry;
	  break;
	default:
	  break;
	}
      lookFor = argsStart;
      *lookFor = '&';
      lookForLength = argsLength - 1;
      curchar = &lookFor[argsLength];
    }
  else
    {
      while (*curchar++ > 32);
      lookForLength = curchar - lookFor - 1;
    }
  lookFor[lookForLength] = 0;
  actionNum = find_semantic_number (action);
  style = lookup_style (action);
  macro = lookup_macro (action);
  if (actionNum == notFound && style == NULL && macro == NULL)
    {
      semanticError (nested,
		     "Action or style or macro '%s' in column 1 not recognized",
		     action);
      return 0;
    }
  if (actionNum > end_all)
    {
      /* Handle pseudo actions */
      switch (actionNum)
	{
	case end_all + 1:	/*include */
	  if (!sem_compileFile (lookFor))
	    return 0;
	  break;
	case end_all + 2:	/*newentries */
	  ud->doc_new_entries = 0;
	  break;
	case end_all + 3:
	  registerNamespaces (nested, xpathCtx, lookFor);
	  break;
	default:
	  break;
	}
      return 1;
    }
  if (hashLookup (semanticTable, (xmlChar *) lookFor) != notFound)
    {
      if (ud->debug)
	semanticError (nested, "duplicate entry '%s' in column 2", lookFor);
      return 1;
    }
  countAttrValues ((xmlChar *) lookFor);
  inserts = NULL;
  while ((ch = *curchar++) <= 32 && ch != 0);
  if (ch != 0)
    {
      insertions = curchar - 1;
      while (*curchar++ > 32);
      insertionsLength = curchar - insertions - 1;
      insertions[insertionsLength] = 0;
      inserts = encodeInsertions (nested, (xmlChar *) insertions,
				  insertionsLength);
    }
  if (insertionsLength == 0 && (actionNum == configfile || actionNum ==
				configstring))
    {
      semanticError (nested, "This semantic action requires a third column.");
      return 0;
    }
  if (actionNum < 0)
    actionNum = generic;
  if (func == 2)
    {
      int k;
      int kk = 0;
      char configString[2 * MAXNAMELEN];
      switch (actionNum)
	{
	case configfile:
	  if (!find_file (insertions, configString))
	    return 0;
	  config_compileSettings (configString);
	  break;
	case configstring:
	  configString[kk++] = ud->string_escape;
	  for (k = 0; k < insertionsLength; kk++)
	    {
	      if (insertions[k] == '=')
		configString[kk++] = ' ';
	      else if (insertions[k] == ';')
		configString[kk++] = '\n';
	      else
		configString[kk++] = (char) insertions[k];
	    }
	  configString[kk] = 0;
	  config_compileSettings (configString);
	  break;
	default:
	  semanticError (nested,
			 "The 'now' option is not valid with this semantic action.");
	  return 0;
	}
    }
  else
    {
      hashInsert (semanticTable, (xmlChar *) lookFor, type, actionNum,
		  inserts, style, macro);
      nested->numEntries++;
    }
  return 1;
}

static int
getALine (lbu_FileInfo * nested)
{
  /*Read a line of chars from an input file */
  int ch;
  int pch = 0;
  int numchars = 0;
  memset (nested->line, 0, sizeof (nested->line));
  while ((ch = fgetc (nested->in)) != EOF)
    {
      if (ch == 13)
	continue;
      if (pch == '\\' && ch == 10)
	{
	  numchars--;
	  continue;
	}
      if (ch == 10 || numchars >= sizeof (nested->line))
	break;
      nested->line[numchars++] = ch;
      pch = ch;
    }
  if (ch == EOF)
    return 0;
  return 1;
}

static int numEntries = 0;

static int
sem_compileFile (const char *fileName)
{
  /*Compile an input file */
  lbu_FileInfo nested;
  char completePath[MAXNAMELEN];
  int haveAppended = 0;
  logMessage(LOU_LOG_DEBUG, "Begin sem_compileFile: fileName=%s", fileName);
  if (!*fileName)
    return 1;			/*Probably run with defaults */
  if (strncmp (fileName, "appended_", 9) == 0)
    {
      strcpy (completePath, lbu_getWriteablePath ());
      strcat (completePath, fileName);
      if (file_exists (completePath))
	haveAppended = 1;
      else
	return 1;
    }
  if (!haveAppended && !find_file (fileName, completePath))
    {
      semanticError (NULL, "Can't find semantic-action file '%s'", fileName);
      haveSemanticFile = 0;
      strcpy (firstFileName, fileName);
      return 0;
    }
  memset (&nested, 0, sizeof (nested));
  nested.fileName = fileName;
  nested.unedited = 1;
  logMessage(LOU_LOG_INFO, "Loading semantic action file: %s", completePath);
  if ((nested.in = fopen ((char *) completePath, "rb")))
    {
      while (getALine (&nested))
	{
	  nested.lineNumber++;
	  compileLine (&nested);
	}
      fclose (nested.in);
      if (nested.unedited)
	semanticError (NULL,
		       "File '%s' needs editing to produce good results.",
		       nested.fileName);
    }
  else
    {
      semanticError (NULL, "Can't open semantic-action file '%s'", fileName);
      return 0;
    }
  numEntries += nested.numEntries;
  logMessage(LOU_LOG_DEBUG, "Finish sem_compileFile");
  return 1;
}

static void
getRootName (xmlNode * rootElement, char *fileName)
{
  char *curchar = NULL;
  rootName = rootElement->name;
  strcpy (fileName, (char *) rootName);
  curchar = fileName;
  while (*curchar)
    {
      if (*curchar == ':' || *curchar == '/' || *curchar == ','
	  || *curchar == '\\' || *curchar == 34 || *curchar == 39
	  || *curchar == '(' || *curchar == ')' || *curchar < 33 ||
	  *curchar > 126)
	*curchar = '_';
      curchar++;
    }
  strcat (fileName, ".sem");
}
static int moreEntries = 0;

int
compile_semantic_table (xmlNode * rootElement)
{
  char fileName[MAXNAMELEN];
  attrValueCounts = NULL;
  haveSemanticFile = 1;
  ud->doc_new_entries = 1;
  moreEntries = 0;
  numEntries = 0;
  rootName = rootElement->name;
  xpathCtx = xmlXPathNewContext (rootElement->doc);
  if (ud->semantic_files != NULL)
    {
      /*Process file list */
      int listLength;
      int currentListPos;
      int k;
      listLength = strlen (ud->semantic_files);
      firstFileName[0] = 0;
      for (k = 0; k < listLength; k++)
	if (ud->semantic_files[k] == ',')
	  break;
      if (k == listLength)
	{			/* Only one file */
	  if (ud->semantic_files[0] == '*')
	    getRootName (rootElement, fileName);
	  else
	    strcpy (fileName, ud->semantic_files);
	  if (!sem_compileFile (fileName))
	    return (haveSemanticFile = 0);
	  strcpy (firstFileName, fileName);
	}
      else
	{			/* Compile a list of files */
	  strncpy (fileName, ud->semantic_files, k);
	  fileName[k] = 0;
	  if (fileName[0] == '*')
	    getRootName (rootElement, fileName);
	  if (!sem_compileFile (fileName))
	    return (haveSemanticFile = 0);
	  strcpy (firstFileName, fileName);
	  currentListPos = k + 1;
	  while (currentListPos < listLength)
	    {
	      for (k = currentListPos; k < listLength; k++)
		if (ud->semantic_files[k] == ',')
		  break;
	      strncpy (fileName, &ud->semantic_files[currentListPos],
		       k - currentListPos);
	      fileName[k - currentListPos] = 0;
	      if (fileName[0] == '*')
		getRootName (rootElement, fileName);
	      if (!sem_compileFile (fileName))
		return (haveSemanticFile = 0);
	      currentListPos = k + 1;
	    }
	}
    }
  else
    {
      getRootName (rootElement, fileName);
      strcpy (firstFileName, fileName);
      if (!sem_compileFile (fileName))
	return (haveSemanticFile = 0);
    }
  strcpy (fileName, "appended_");
  strcat (fileName, firstFileName);
  sem_compileFile (fileName);
  if (numEntries == 0)
    {
      destroy_semantic_table ();
      return (haveSemanticFile = 0);
    }
  return 1;
}

static void addNewEntries (const xmlChar * key);

/**
 * registerNamespaces:
 * @xpathCtx:		the pointer to an XPath context.
 * @nsList:		the list of known namespaces in 
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 1 on success and 0 on failure.
 */
static int
registerNamespaces (lbu_FileInfo * nested, xmlXPathContextPtr xpathCtx, const
		    xmlChar * nsList)
{
  xmlChar *nsListDup;
  xmlChar *prefix;
  xmlChar *href;
  xmlChar *next;

  nsListDup = xmlStrdup (nsList);
  if (nsListDup == NULL)
    {
      semanticError (nested, "Error: unable to strdup namespaces list");
      return 0;
    }
  next = nsListDup;
  while (next != NULL)
    {
      if ((*next) == '\0')
	break;
      /* find prefix */
      prefix = next;
      next = (xmlChar *) xmlStrchr (next, '=');
      if (next == NULL)
	{
	  semanticError (nested, "Error: invalid namespaces list format");
	  xmlFree (nsListDup);
	  return 0;
	}
      *(next++) = '\0';
      /* find href */
      href = next;
      next = (xmlChar *) xmlStrchr (next, ',');
      if (next != NULL)
	{
	  *(next++) = '\0';
	}
      /* do register namespace */
      if (xmlXPathRegisterNs (xpathCtx, prefix, href) != 0)
	{
	  semanticError (nested,
			 "Error: unable to register NS with prefix=\"%s\" and href=\"%s\"",
			 prefix, href);
	  xmlFree (nsListDup);
	  return 0;
	}
    }
  xmlFree (nsListDup);
  return 1;
}

static void
printXpathNodes (xmlNodeSetPtr nodes)
{
  xmlNodePtr cur;
  int size;
  int i;
  size = (nodes) ? nodes->nodeNr : 0;
  logMessage (LOU_LOG_INFO, "Result (%d nodes):", size);
  for (i = 0; i < size; ++i)
    {
      if (nodes->nodeTab[i]->type == XML_NAMESPACE_DECL)
	{
	  xmlNsPtr ns;
	  ns = (xmlNsPtr) nodes->nodeTab[i];
	  cur = (xmlNodePtr) ns->next;
	  if (cur->ns)
	    {
	      logMessage (LOU_LOG_INFO,
			     "= namespace \"%s\"=\"%s\" for node %s:%s",
			     ns->prefix, ns->href, cur->ns->href, cur->name);
	    }
	  else
	    {
	      logMessage (LOU_LOG_INFO, "= namespace \"%s\"=\"%s\" for node %s",
			     ns->prefix, ns->href, cur->name);
	    }
	}
      else if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
	{
	  cur = nodes->nodeTab[i];
	  if (cur->ns)
	    {
	      logMessage (LOU_LOG_INFO, "= element node \"%s:%s\"",
			     cur->ns->href, cur->name);
	    }
	  else
	    {
	      logMessage (LOU_LOG_INFO, "= element node \"%s\"", cur->name);
	    }
	}
      else
	{
	  cur = nodes->nodeTab[i];
	  logMessage (LOU_LOG_INFO, "= node \"%s\": type %d", cur->name,
			 cur->type);
	}
    }
}

int
do_xpath_expr ()
{
  xmlXPathObject *xpathObj;
  HashEntry *curEntry;
  while ((curEntry = hashScan (semanticTable)))
    {
      if (curEntry->type & xpathEntry)
	{
	  xmlNodeSet *nodeSet;
	  xmlNode *node;
	  int size;
	  int k;
	  xpathObj = xmlXPathEvalExpression (&curEntry->key[1], xpathCtx);
	  if (xpathObj == NULL || xpathObj->type != XPATH_NODESET)
	    continue;
	  nodeSet = xpathObj->nodesetval;
	  if (ud->debug)
	    printXpathNodes (nodeSet);
	  size = (nodeSet) ? nodeSet->nodeNr : 0;
	  for (k = 0; k < size; k++)
	    {
	      node = nodeSet->nodeTab[k];
	      if (node->_private == NULL)
		node->_private = curEntry;
	    }
	  xmlXPathFreeObject (xpathObj);
	}
    }
  return 1;
}

sem_act
set_sem_attr (xmlNode * node)
{
  sem_act action = no;
  int actionNum = notFound;
  xmlChar key[MAXNAMELEN];
  int k;
  int oldKeyLength = 0;
  const xmlChar *name;
  if (semanticTable == NULL)
    return no;
  if (node->_private != NULL)
    return get_sem_attr (node);
  if (node->type == XML_CDATA_SECTION_NODE)
    name = (xmlChar *) "cdata-section";
  else
    name = node->name;
  if (node->properties)
    {
      xmlAttr *attributes = node->properties;
      while (attributes)
	{
	  const xmlChar *attrName = attributes->name;
	  xmlChar *attrValue = xmlGetProp (node, attrName);
	  strcpy ((char *) key, (char *) name);
	  strcat ((char *) key, ",");
	  strcat ((char *) key, (char *) attrName);
	  strcat ((char *) key, ",");
	  oldKeyLength = strlen ((char *) key);
	  strncat ((char *) key, (char *) attrValue, sizeof (key) -
		   oldKeyLength - 2);
	  for (k = 0; key[k]; k++)
	    if ((key[k] <= 32 || key[k] > 126)
		|| (k >= oldKeyLength && key[k] == ','))
	      key[k] = '_';
	  actionNum = notFound;
	  if (((actionNum = hashLookup (semanticTable, key)) != notFound) ||
	      countAttrValues (key))
	    {
	      if (actionNum == notFound)
		addNewEntries (key);
	    }
	  if (actionNum == notFound || actionNum == no)
	    {
	      key[oldKeyLength - 1] = 0;
	      actionNum = hashLookup (semanticTable, key);
	      if (actionNum == notFound)
		addNewEntries (key);
	      else if (actionNum == no)
		actionNum = notFound;
	    }
	  if (actionNum != notFound && actionNum != no)
	    break;
	  attributes = attributes->next;
	}
    }
  if (actionNum == notFound)
    {
      strcpy ((char *) key, (char *) name);
      actionNum = hashLookup (semanticTable, key);
    }
  if (actionNum == notFound)
    addNewEntries (name);
  else
    action = actionNum;
  node->_private = latestEntry;
  return action;
}

sem_act
get_sem_attr (xmlNode * node)
{
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry != NULL)
  {
    logMessage(LOU_LOG_DEBUG, "Node %s has nodeEntry", (const char *)node->name);
    return nodeEntry->semNum;
  }
  else
  {
    logMessage(LOU_LOG_DEBUG, "Node %s has no nodeEntry", (const char *)node->name);
    return no;
  }
}

unsigned char *
get_sem_name (xmlNode * node)
{
  int k;
  static char key[50];
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry == NULL)
    return "";
  if (nodeEntry->macro != NULL)
    {
      char *macro = nodeEntry->macro;
      strcpy (key, "macro,");
      for (k = 0; macro[k] != ','; k++);
      k--;
      strncat (key, macro, k);
    }
  else if (nodeEntry->style != NULL)
    {
      strcpy (key, "style,");
      strcat (key, nodeEntry->style->name);
    }
  else
    {
      strcpy (key, "action,");
      strcat (key, semNames[nodeEntry->semNum]);
    }
  return key;
}

StyleType *
is_style (xmlNode * node)
{
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry != NULL)
    return nodeEntry->style;
  else
    return NULL;
}

xmlChar *
get_attr_value (xmlNode * node)
{
  int firstComma = 0, secondComma = 0;
  int k;
  xmlChar attrName[MAXNAMELEN];
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry == NULL || !node->properties)
    return (xmlChar *) "";
  for (k = 0; nodeEntry->key[k]; k++)
    if (!firstComma && nodeEntry->key[k] == ',')
      firstComma = k;
    else if (firstComma && nodeEntry->key[k] == ',')
      secondComma = k;
  if (firstComma == 0)
    return (xmlChar *) "";
  if (secondComma == 0)
    secondComma = strlen ((char *) nodeEntry->key);
  k = 0;
  firstComma++;
  while (firstComma < secondComma)
    attrName[k++] = nodeEntry->key[firstComma++];
  attrName[k] = 0;
  return xmlGetProp (node, attrName);
}

void
shortenStack ()
{
  int k;
  int kk = 0;
  for (k = ud->top / 2; k <= ud->top; k++)
    ud->stack[kk++] = ud->stack[k];
  ud->top = kk - 1;
}

sem_act
push_sem_stack (xmlNode * node)
{
  if (ud->top < -1)
    ud->top = -1;
  if (ud->top > (STACKSIZE - 3))
    shortenStack ();
  return (ud->stack[++ud->top] = get_sem_attr (node));
}

sem_act
push_action (sem_act action)
{
  if (ud->top < -1)
    ud->top = -1;
  if (ud->top > (STACKSIZE - 3))
    shortenStack ();
  return (ud->stack[++ud->top] = action);
}

sem_act
pop_sem_stack ()
{
  if (ud->top < 0)
    {
      ud->top = -1;
      ud->stack[++ud->top] = no;
      return no;
    }
  ud->top--;
  if (ud->top > -1)
    return ud->stack[ud->top];
  return no;
}

static void
addNewEntries (const xmlChar * newEntry)
{
  if (newEntry == NULL || *newEntry == 0 || !ud->new_entries ||
      !ud->doc_new_entries)
    return;
  if (moreEntries == 0)
    {
      moreEntries = 1;
      newEntriesTable = hashNew ();
    }
  if (hashLookup (newEntriesTable, newEntry) != notFound)
    return;
  hashInsert (newEntriesTable, newEntry, 0, 0, NULL, NULL, NULL);
}

void
append_new_entries ()
{
  int items;
  char filePrefix[20];
  char fileMode[8];
  char outFileName[MAXNAMELEN];
  FILE *semOut;
  int numEntries = 0;
  destroyattrValueCountsTable ();
  if (!moreEntries || !*firstFileName)
    return;
  if (haveSemanticFile)
    {
      strcpy (fileMode, "ab");
      strcpy (filePrefix, "appended_");
    }
  else
    {
      strcpy (fileMode, "wb");
      strcpy (filePrefix, "new_");
    }
  strcpy (outFileName, lbu_getWriteablePath ());
  strcat (outFileName, filePrefix);
  strcat (outFileName, firstFileName);
  semOut = fopen ((char *) outFileName, fileMode);
  if (!haveSemanticFile)
    {
      fprintf (semOut,
	       "# This file was produced by liblouisutdml and is considered part of\n");
      fprintf (semOut,
	       "# the code, licensed under the GNU Lesser or Library Public License.\n\n");
      fprintf (semOut,
	       "# You must edit this file as explained in the documentation to get\n");
      fprintf (semOut, "# proper output.\n\n");
    }
  for (items = 1; items < 4; items++)
    {
      HashEntry *curEntry;
      int k;
      while ((curEntry = hashScan (newEntriesTable)))
	{
	  int numItems = 1;
	  for (k = 0; curEntry->key[k]; k++)
	    if (curEntry->key[k] == ',')
	      numItems++;
	  if (numItems != items)
	    continue;
	  fprintf (semOut, "no %s\n", curEntry->key);
	  numEntries++;
	}
    }
  fclose (semOut);
  if (haveSemanticFile)
    logMessage (LOU_LOG_WARN, "%d new entries appended to '%s%s'.", numEntries,
		  filePrefix, firstFileName);
  else
    logMessage (LOU_LOG_WARN, "%d entries written to new semantic-action file '%s%s'.",
		  numEntries, filePrefix, firstFileName);
  moreEntries = 0;
}

void
do_reverse (xmlNode * node)
{
  xmlNode *child = node->children;
  xmlNode *curNext;
  xmlNode *curPrev;
  if (!child || !child->next)
    return;
  while (child)
    {
      curNext = child->next;
      curPrev = child->prev;
      if (curNext == NULL)
	node->children = child;
      child->next = curPrev;
      child->prev = curNext;
      child = curNext;
    }
}

#define STYLESUF " elyts"
StyleType *
new_style (xmlChar * name)
{
  char key[MAXNAMELEN];
  StyleType *style;
  if (!semanticTable)
    semanticTable = hashNew ();
  strcpy (key, name);
  strcat (key, STYLESUF);
  if (hashLookup (semanticTable, key) != notFound)
    return latestEntry->style;
  style = malloc (sizeof (StyleType) + strlen (name) + 3);
  if (!style)
    memoryError ();
  memset (style, 0, sizeof (StyleType));
  strcpy (style->name, name);
  style->newline_after = 1;
  hashInsert (semanticTable, key, styleEntry, 0, NULL, style, NULL);
  style->format = inherit;	/* inherit parent format by default */
  style->right_margin = -100;
  style->left_margin = -100;
  style->first_line_indent = -100;
  return style;
}

StyleType *
lookup_style (xmlChar * name)
{
  char key[MAXNAMELEN];
  strcpy (key, name);
  strcat (key, STYLESUF);
  if (hashLookup (semanticTable, key) != notFound)
    return latestEntry->style;
  return NULL;
}

StyleType *
action_to_style (sem_act action)
{
  if (action < 0 || action >= end_all)
    return NULL;
  return lookup_style ((char *) semNames[action]);
}

/* Beginning of macro processing */

/* Hold macro state */
static char macroName[40];
static char *macro = NULL;
static int macroLength = 0;
static int posInMacro;;
static xmlNode *macroNode;
static HashEntry *isMacroEntry;
static int macroHasStyle;
static char *paramStart = NULL;
static int paramLength = 0;

static void
macroError (char *format, ...)
{
  char buffer[MAXNAMELEN];
  va_list arguments;
  va_start (arguments, format);
#ifdef WIN32
  _vsnprintf (buffer, sizeof (buffer), format, arguments);
#else
  vsnprintf (buffer, sizeof (buffer), format, arguments);
#endif
  va_end (arguments);
  logMessage (LOU_LOG_ERROR, "Macro %s: %s", macroName, buffer);
}

char *
is_macro (xmlNode * node)
{
  HashEntry *nodeEntry = (HashEntry *) node->_private;
  if (nodeEntry != NULL)
    return nodeEntry->macro;
  else
    return NULL;
}

#define MACROSUF " orcam"

unsigned char *
lookup_macro (xmlChar * name)
{
  char key[MAXNAMELEN];
  strcpy (key, name);
  strcat (key, MACROSUF);
  if (hashLookup (semanticTable, key) != notFound)
    return latestEntry->macro;
  return NULL;
}

xmlChar *
new_macro (xmlChar * name, xmlChar * body)
{
  char *storedMacro;
  char key[MAXNAMELEN];
  xmlChar *nameAndBody = (xmlChar *) ud->text_buffer;
  strcpy (nameAndBody, name);
  strcat (nameAndBody, ",");
  strcat (nameAndBody, body);
  storedMacro = alloc_string (nameAndBody);
  if (!semanticTable)
    semanticTable = hashNew ();
  strcpy (key, name);
  strcat (key, MACROSUF);
  if (hashLookup (semanticTable, key) != notFound)
    return NULL;
  hashInsert (semanticTable, key, macroEntry, 0, NULL, NULL, storedMacro);
  return storedMacro;
}

static int
insertFromMacro (int which)
{
  return 1;
}

static int
doSemanticActions ()
{
  int retVal = 1;
  int semNum = atoi (&macro[posInMacro]);
  for (; isdigit (macro[posInMacro]) && posInMacro < macroLength;
       posInMacro++);
  if (macro[posInMacro] == '(')
    {
      paramStart = &macro[++posInMacro];
      paramLength = find_group_length ("()", paramStart - 1);
      posInMacro += paramLength;
      paramLength -= 2;
    }
  switch (semNum)
    {
    case no:
      if (ud->text_length > 0 && ud->text_length < MAX_LENGTH
	  && ud->text_buffer[ud->text_length - 1] > 32)
	ud->text_buffer[ud->text_length++] = 32;
      break;
    case skip:
      retVal = -1;
      break;
    case markhead:
      ud->head_node = macroNode;
      break;
    case pagebreak:
      do_pagebreak (macroNode);
      break;
    case attrtotext:
      do_attrtotext (macroNode);
      break;
    case configtweak:
      {
	int k;
	int kk = 0;
	xmlChar configString[2 * MAXNAMELEN];
	configString[kk++] = ud->string_escape;
	for (k = 0; k < paramLength; k++)
	  {
	    if (paramStart[k] == '=')
	      configString[kk++] = ' ';
	    else if (paramStart[k] == ';')
	      configString[kk++] = '\n';
	    else
	      configString[kk++] = (xmlChar) paramStart[k];
	  }
	configString[kk++] = '\n';
	configString[kk] = 0;
	if (!config_compileSettings ((char *) configString))
	  return 0;
      }
      ud->main_braille_table = ud->contracted_table_name;
      if (!lou_getTable (ud->main_braille_table))
	{
	  logMessage (LOU_LOG_ERROR, "Cannot open main table %s", ud->main_braille_table);
	  return 0;
	}
      break;
    case htmllink:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (macroNode, 0);
      break;
    case htmltarget:
      if (ud->format_for != browser)
	break;
      insert_linkOrTarget (macroNode, 1);
      break;
    case blankline:
      do_blankline ();
      break;
    case linespacing:
      do_linespacing (macroNode);
      break;
    case softreturn:
      do_softreturn ();
      break;
    case righthandpage:
      do_righthandpage ();
      break;
    case code:
      transcribe_computerCode (macroNode, 0);
      break;
    case math:
      transcribe_math (macroNode, 0);
      break;
    case graphic:
      transcribe_graphic (macroNode, 0);
      break;
    case chemistry:
      transcribe_chemistry (macroNode, 0);
      break;
    case music:
      transcribe_music (macroNode, 0);
      break;
    case changetable:
      change_table (macroNode);
      break;
    case pagenum:
      do_pagenum ();
      break;
    case italicx:
      push_action (italicx);
      break;
    case boldx:
      push_action (boldx);
      break;
    case underlinex:
      push_action (underlinex);
      break;
    case compbrl:
      push_action (compbrl);
      break;
    default:
      macroError ("semantic action %s is not supported in a macro",
		  semNames[semNum]);
      retVal = 0;
    }
  return retVal;
}

static int
compileMacro ()
{
  xmlChar compiledMacro[4 * MAXNAMELEN];
  int unPos = 0;
  int pos = 0;
  memset (compiledMacro, 0, sizeof (compiledMacro));
  while (unPos < macroLength)
    {
      if (isalpha (macro[unPos]))
	{
	  char name[40];
	  int k;
	  StyleType *style;
	  for (k = 0; isalnum (macro[unPos]) && unPos < macroLength; unPos++)
	    name[k++] = macro[unPos];
	  name[k] = 0;
	  if ((style = lookup_style (name)) != NULL)
	    {
	      if (macroHasStyle)
		{
		  macroError ("only one style can be specified");
		  compiledMacro[0] = '!';
		  break;
		}
	      else
		{
		  macroHasStyle = 1;
		  compiledMacro[pos++] = '~';
		  isMacroEntry->style = style;
		}
	    }
	  else
	    {
	      if ((k = find_semantic_number (name)) != notFound)
		{
		  k = sprintf (name, "%d", k);
		  strcpy (&compiledMacro[pos], name);
		  pos += k;
		  if (macro[unPos] == '(')
		    {
		      k = find_group_length ("()", &macro[unPos]);
		      strncpy (&compiledMacro[pos], &macro[unPos], k);
		      pos += k;
		      unPos += k;
		    }
		}
	      else if (strcmp (name, "styleend") == 0)
		compiledMacro[pos++] = '@';
	      else if (strcmp (name, "pause") == 0)
		compiledMacro[pos++] = '#';
	      else
		{
		  macroError
		    ("'%s' is neither a style or semantic action or keyword",
		     name);
		  compiledMacro[0] = '!';
		  break;
		}
	    }
	}
      if (macro[unPos] == ',')
	compiledMacro[pos++] = macro[unPos++];
    }
  compiledMacro[pos] = 0;
  strcpy (macro, compiledMacro);
  macroLength = pos;
  return 1;
}

static int
executeMacro ()
{
  while (posInMacro < macroLength)
    {
      if (isdigit (macro[posInMacro]))
	doSemanticActions ();
      else
	switch (macro[posInMacro])
	  {
	  case ',':
	    posInMacro++;
	    break;
	  case '~':
	    start_style (isMacroEntry->style, macroNode);
	    macroHasStyle = 1;
	    posInMacro++;
	    break;
	  case '@':
	    end_style ();
	    macroHasStyle = 0;
	    posInMacro++;
	    break;
	  case '#':
	    /* Pause for calling function to do something. */
	    posInMacro++;
	    return 1;
	  default:
	    posInMacro++;
	    break;
	  }
    }
  if (posInMacro >= macroLength)
    {
      if (macroHasStyle)
	end_style ();
      macro = NULL;
    }
  return 1;
}

int
start_macro (xmlNode * node)
{
  /* Set macro state */
  int k;
  macroNode = node;
  isMacroEntry = (HashEntry *) node->_private;
  if (isMacroEntry == NULL || isMacroEntry->macro == NULL)
    return 0;
  macro = isMacroEntry->macro;
  for (k = 0; macro[k] != ','; k++)
    macroName[k] = macro[k];
  macroName[k] = 0;
  if (macroLength != 0)
    {
      macroError ("macros cannot be nested");
      return 0;
    }
  macro = &macro[k + 1];
  macroLength = strlen (macro);
  posInMacro = 0;
  macroHasStyle = 0;
  paramStart = NULL;
  paramLength = 0;
  /*compile macro the first time it is used. */
  if (isalpha (macro[0]))
    compileMacro ();
  if (macro[0] == '!')
    /* Contains errors */
    return 0;
  executeMacro ();
  return macroHasStyle;
}

int
end_macro ()
{
  if (macro == NULL || macro[0] == '!')
    {
      macroLength = 0;
      return 0;
    }
  executeMacro ();
  macroLength = 0;
  return 1;
}

/* End of macro processing */
