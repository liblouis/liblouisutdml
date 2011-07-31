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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisutdml.h"
#include <libxml/HTMLparser.h>

UserData *ud = NULL;

char *EXPORT_CALL
lbu_version ()
{
  static char *version = PACKAGE_VERSION;
  static char bothVersions[60];
  strcpy (bothVersions, version);
  strcat (bothVersions, " ");
  strcat (bothVersions, lou_version ());
  return bothVersions;
}

void
libxml_errors (void *ctx ATTRIBUTE_UNUSED, const char *msg, ...)
{
  va_list args;
  char buffer[MAXNAMELEN];
  va_start (args, msg);
  memset (buffer, 0, sizeof (buffer));
  vsnprintf (buffer, sizeof (buffer) - 4, msg, args);
  va_end (args);
  lou_logPrint ("%s", buffer);
}

static int
processXmlDocument (const char *inputDoc, int length, int mode)
{
  /*This function does all processing of xml documents as such.
   * If length is 0 the document is assumed to be a file.
   * If length is not 0 it is assumed to be in memory.
   * Sort of hackish, but only hackers will see it. */
  xmlNode *rootElement = NULL;
  int haveSemanticFile;
  xmlParserCtxt *ctxt;
  static int initialized = 0;
  if (!initialized)
    {
      initialized = 1;
      LIBXML_TEST_VERSION xmlKeepBlanksDefault (0);
      xmlSubstituteEntitiesDefault (1);
      xmlThrDefIndentTreeOutput (1);
      xmlThrDefKeepBlanksDefaultValue (0);
      xmlThrDefLineNumbersDefaultValue (1);
    }
  ud->doc = NULL;
  ctxt = xmlNewParserCtxt ();
  xmlSetGenericErrorFunc (ctxt, libxml_errors);
  if (length == 0)
    {
      if ((mode & htmlDoc))
	ud->doc = htmlParseFile (inputDoc, NULL);
      else
	{
	  if (ud->internet_access)
	    ud->doc = xmlCtxtReadFile (ctxt, inputDoc, NULL,
				       XML_PARSE_DTDVALID | XML_PARSE_NOENT);
	  else
	    ud->doc = xmlParseFile (inputDoc);
	  if (ud->doc == NULL)
	    ud->doc = htmlParseFile (inputDoc, NULL);
	}
    }
  else
    ud->doc = xmlParseMemory (inputDoc, length);
  if (ud->doc == NULL)
    {
      lou_logPrint ("Document could not be processed");
      return 0;
    }
  rootElement = xmlDocGetRootElement (ud->doc);
  if (rootElement == NULL)
    {
      lou_logPrint ("Document is empty");
      return 0;
    }
  haveSemanticFile = compile_semantic_table (rootElement);
  do_xpath_expr ();
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return 0;
  transcribe_document (rootElement);
  xmlFreeDoc (ud->doc);
  xmlCleanupParser ();
  initGenericErrorDefaultFunc (NULL);
  xmlFreeParserCtxt (ctxt);
  return 1;
}

void *EXPORT_CALL
lbu_initialize (const char *configFileName,
		const char *logFileName, const char *settingsString)
{
  if (!read_configuration_file (configFileName, logFileName,
				settingsString, 0))
    return NULL;
  return (void *) ud;
}

int EXPORT_CALL
lbu_translateString (const char *configFileName,
		     const char *inbuf, int inlen, widechar *outbuf,
		     int *outlen,
		     const char *logFileName, const char *settingsString,
		     unsigned int mode)
{
/* Translate the well-formed xml expression in inbuf into braille 
* according to the specifications in configFileName. If the expression 
* is not well-formed or there are oteer errors, return 0.*/
  int k;
  char *xmlInbuf;
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->inbuf = inbuf;
  ud->inlen = inlen;
  ud->outbuf1 = outbuf;
  ud->outbuf1_len = *outlen;
  ud->inFile = ud->outFile = NULL;
  for (k = 0; k < inlen; k++)
    if (inbuf[k] > ' ')
      break;
  if (inbuf[k] != '<')
    {
      transcribe_text_string ();
      *outlen = ud->outlen_so_far;
      return 1;
    }
  if (inbuf[k + 1] == '?')
    xmlInbuf = (char *) inbuf;
  else
    {
      inlen += strlen (ud->xml_header);
      if (!(xmlInbuf = malloc (inlen + 4)))
	{
	  lou_logPrint ("Not enough memory");
	  return 0;
	}
      strcpy (xmlInbuf, ud->xml_header);
      strcat (xmlInbuf, "\n");
      strcat (xmlInbuf, inbuf);
    }
  ud->inFile = ud->outFile = NULL;
  if (!processXmlDocument (xmlInbuf, inlen, mode))
    return 0;
  *outlen = ud->outlen_so_far;
  if (xmlInbuf != inbuf)
    free (xmlInbuf);
  lou_logEnd ();
  return 1;
}

int
  EXPORT_CALL lbu_translateFile
  (const char *configFileName, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Translate the well-formed xml expression in inFileName into 
* braille according to the specifications in configFileName. If the 
* expression is not well-formed or there are other errors, 
* return 0. */
  widechar outbuf[2 * BUFSIZE];
  widechar outbuf2[2 * BUFSIZE];
  widechar outbuf3[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->outbuf1 = outbuf;
  ud->outbuf2 = outbuf2;
  ud->outbuf3 = outbuf3;
  ud->outbuf1_len = (sizeof (outbuf) / CHARSIZE) - 4;
  ud->outbuf2_len = (sizeof (outbuf2) / CHARSIZE) - 4;
  ud->outbuf3_len = (sizeof (outbuf3) / CHARSIZE) - 4;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  if (!processXmlDocument (inFileName, 0, mode))
    return 0;
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lou_logEnd ();
  return 1;
}

int
  EXPORT_CALL lbu_translateTextFile
  (const char *configFileName, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Translate the text file in inFileName into braille according to
* the specifications in configFileName. If there are errors, print 
* an error message and return 0.*/
  widechar outbuf[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", inFileName);
	  return 0;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.\n", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  transcribe_text_file ();
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lou_logEnd ();
  return 1;
}

int EXPORT_CALL
lbu_backTranslateString (const char *configFileName,
			 const char *inbuf, int inlen, widechar
			 *outbuf,
			 int *outlen,
			 const char *logFileName, const char
			 *settingsString, unsigned int mode)
{
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->inbuf = inbuf;
  ud->inlen = inlen;
  ud->outbuf1 = outbuf;
  ud->outbuf1_len = *outlen;
  ud->outbuf = (widechar *) outbuf;
  ud->outlen = ud->outlen / CHARSIZE;
  ud->inFile = ud->outFile = NULL;
  back_translate_braille_string ();
  *outlen = ud->outlen_so_far;
  lou_logEnd ();
  return 1;
}

int
  EXPORT_CALL lbu_backTranslateFile
  (const char *configFileName, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Back translate the braille file in inFileName into either an 
* xml file or a text file according to
* the specifications in configFileName. If there are errors, print an 
* error message and return 0.*/
  widechar outbuf[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", inFileName);
	  return 0;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.\n", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  back_translate_file ();
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lou_logEnd ();
  return 1;
}

static char writeablePath[MAXNAMELEN];
static char *writeablePathPtr = NULL;

char *EXPORT_CALL
lbu_setWriteablePath (const char *path)
{
  strcpy (writeablePath, path);
  writeablePathPtr = writeablePath;
  return writeablePathPtr;
}

char *EXPORT_CALL
lbu_getWriteablePath ()
{
  return writeablePathPtr;
}

int EXPORT_CALL
lbu_charToDots (const char *tableList, const unsigned char *inbuf,
		unsigned char *outbuf, int length, const char *logFile,
		unsigned int mode)
{
  widechar *interBuf;
  int wcLength;
  int utf8Length;
  int result = 0;
  lou_logFile (logFile);
  interBuf = malloc (length * CHARSIZE);
  utf8Length = length;
  wcLength = length;
  utf8_string_to_wc (inbuf, &utf8Length, interBuf, &wcLength);
  result = lou_charToDots (tableList, interBuf, interBuf, wcLength, mode
			   | ucBrl);
  if (result)
    {
      utf8Length = length;
      wc_string_to_utf8 (interBuf, &wcLength, outbuf, &utf8Length);
    }
  lou_logEnd ();
  free (interBuf);
  return result;
}

int EXPORT_CALL
lbu_dotsToChar (const char *tableList, const unsigned char *inbuf,
		unsigned char *outbuf, int length, const char *logFile,
		unsigned int mode)
{
  widechar *interBuf;
  int wcLength;
  int utf8Length;
  int result = 0;
  lou_logFile (logFile);
  interBuf = malloc (length * CHARSIZE);
  utf8Length = length;
  wcLength = length;
  utf8_string_to_wc (inbuf, &utf8Length, interBuf, &wcLength);
  result = lou_dotsToChar (tableList, interBuf, interBuf, wcLength, mode);
  if (result)
    {
      utf8Length = length;
      wc_string_to_utf8 (interBuf, &wcLength, outbuf, &utf8Length);
    }
  lou_logEnd ();
  free (interBuf);
  return result;
}

int EXPORT_CALL
lbu_checkTable (const char *tableList, const char *logFile, unsigned int mode)
{
  int result = 1;
  lou_logFile (logFile);
  if (!lou_getTable (tableList))
    result = 0;
  lou_logEnd ();
  return result;
}

void EXPORT_CALL
lbu_free (void)
{
/* Free all memory used by liblouisutdml. You MUST call this function at 
* the END of your application.*/
  lou_free ();
  destroy_semantic_table ();
  if (ud != NULL)
    free (ud);
  ud = NULL;
}
