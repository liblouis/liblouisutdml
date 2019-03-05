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
#include <libxml/catalog.h>

UserData *ud = NULL;

LBUAPI char *EXPORT_CALL
lbu_version ()
{
  static char *version = PACKAGE_VERSION;
  static char bothVersions[60];
  strcpy (bothVersions, version);
  strcat (bothVersions, " ");
  strcat (bothVersions, lou_version ());
  return bothVersions;
}

LBUAPI void EXPORT_CALL lbu_loadXMLCatalog(const char *filename)
{
  xmlLoadCatalog(filename);
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
  logMessage (LOU_LOG_ERROR, "%s", buffer);
}

static xmlParserCtxt *ctxt;
static int libxml2_initialized = 0;

static void
cleanupLibxml ()
{
  destroy_semantic_table ();
  if (ud->doc != NULL)
    xmlFreeDoc (ud->doc);
  if (!libxml2_initialized)
    return;
  xmlCleanupParser ();
  initGenericErrorDefaultFunc (NULL);
  xmlFreeParserCtxt (ctxt);
}

static void
freeEverything ()
{
  lbu_logEnd ();
  cleanupLibxml ();
  lbu_free ();
}

static int
processXmlDocument (const char *inputDoc, int length)
{
  /*This function does all processing of xml documents as such.
   * If length is 0 the document is assumed to be a file.
   * If length is not 0 it is assumed to be in memory.
   * Sort of hackish, but only hackers will see it. */
  xmlNode *rootElement = NULL;
  int haveSemanticFile;
  if (!libxml2_initialized)
    {
      libxml2_initialized = 1;
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
      if ((ud->mode & htmlDoc))
	ud->doc = htmlParseFile (inputDoc, NULL);
      else
	{
	  if (ud->internet_access)
	    ud->doc = xmlCtxtReadFile (ctxt, inputDoc, NULL,
				       XML_PARSE_DTDVALID | XML_PARSE_NOENT);
	  else
	    ud->doc = xmlParseFile (inputDoc);
	  if (ud->doc == NULL)
	    {
	      logMessage
		(LOU_LOG_FATAL, "Document could not be processed; may be \
malformed or contain illegal characters");
	      cleanupLibxml ();
	      return 0;
	    }
	}
    }
  else
    ud->doc = xmlParseMemory (inputDoc, length);
  if (ud->doc == NULL)
    {
      logMessage (LOU_LOG_FATAL, "Document could not be processed, probably  malformed");
      cleanupLibxml ();
      return 0;
    }
  if (ud->doc->encoding == NULL)
    {
      logMessage (LOU_LOG_ERROR, "Encoding, preferably UTF-8,  must be specified");
      cleanupLibxml ();
      return 0;
    }
  if (ud->format_for >= utd && ignore_case_comp (ud->doc->encoding,
						 "UTF-8", 5) != 0)
    {
      logMessage (LOU_LOG_ERROR, "UTDML requires UTF-8 encoding, not '%s'",
		    ud->doc->encoding);
      cleanupLibxml ();
      return 0;
    }
  rootElement = xmlDocGetRootElement (ud->doc);
  if (rootElement == NULL)
    {
      logMessage (LOU_LOG_ERROR, "Document is empty");
      cleanupLibxml ();
      return 0;
    }
  if (ud->mode & convertOnly)
    convert_utd ();
  else
    {
      haveSemanticFile = compile_semantic_table (rootElement);
      do_xpath_expr ();
      examine_document (rootElement);
      append_new_entries ();
      /* This will generate a new semantic-action file if none exists 
       * and newEntries is  yes in the configuration file.
       * Otherwise it generates a new_enhtries file.*/
      if (!haveSemanticFile)
	{
	  cleanupLibxml ();
	  return 0;
	}
      if (!transcribe_document (rootElement))
	{
	  logMessage (LOU_LOG_ERROR, "Document could not be transcribed");
	  cleanupLibxml ();
	  return 0;
	}
    }
  cleanupLibxml ();
  return 1;
}

LBUAPI void *EXPORT_CALL
lbu_initialize (const char *configFileList,
		const char *logFileName, const char *settingsString)
{
  if (!read_configuration_file (configFileList, logFileName,
				settingsString, 0))
    return NULL;
  return (void *) ud;
}

LBUAPI int EXPORT_CALL
lbu_translateString (const char *configFileList,
		     const char *inbuf, int inlen, widechar * outbuf,
		     int *outlen,
		     const char *logFileName, const char *settingsString,
		     unsigned int mode)
{
/* Translate the well-formed xml expression in inbuf into braille 
* according to the specifications in configFileList. If the expression 
* is not well-formed or there are oteer errors, return 0.*/
  int k;
  char *xmlInbuf;
  if (!read_configuration_file
      (configFileList, logFileName, settingsString, mode))
    return 0;
  if (inbuf == NULL || outbuf == NULL || outlen == NULL)
    return 0;
  ud->inbuf = inbuf;
  ud->inlen = inlen;
  ud->outbuf = outbuf;
  ud->outlen = *outlen;
  ud->inFile = ud->outFile = NULL;
  for (k = 0; k < inlen; k++)
    if (inbuf[k] > ' ')
      break;
  if (inbuf[k] != '<')
    {
      if (ud->format_for == utd)
	k = utd_transcribe_text_string ();
      else
	k = transcribe_text_string ();
      *outlen = ud->outlen_so_far;
      lbu_logEnd ();
      return k;
    }
  if (inbuf[k + 1] == '?')
    xmlInbuf = (char *) inbuf;
  else
    {
      inlen += strlen (ud->xml_header);
      if (!(xmlInbuf = malloc (inlen + 4)))
	{
	  logMessage (LOU_LOG_FATAL, "Not enough memory");
	  return 0;
	}
      strcpy (xmlInbuf, ud->xml_header);
      strcat (xmlInbuf, "\n");
      strcat (xmlInbuf, inbuf);
    }
  k = processXmlDocument (xmlInbuf, inlen);
  *outlen = ud->outlen_so_far;
  if (xmlInbuf != inbuf)
    free (xmlInbuf);
  lbu_logEnd ();
  return k;
}

LBUAPI int EXPORT_CALL lbu_translateFile
  (const char *configFileList, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Translate the well-formed xml expression in inFileName into 
* braille according to the specifications in configFileList. If the 
* expression is not well-formed or there are other errors, 
* return 0. */
  int k;
  if (!read_configuration_file
      (configFileList, logFileName, settingsString, mode))
    return 0;
  if (inFileName == NULL || outFileName == NULL)
    return 0;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "wb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open output file %s.", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  k = processXmlDocument (inFileName, 0);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lbu_logEnd ();
  return k;
}

LBUAPI int EXPORT_CALL lbu_translateTextFile
  (const char *configFileList, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Translate the text file in inFileName into braille according to
* the specifications in configFileList. If there are errors, print 
* an error message and return 0.*/
  int k;
  widechar outbuf[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileList, logFileName, settingsString, mode))
    return 0;
  if (inFileName == NULL || outFileName == NULL)
    return 0;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "rb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open input file %s.", inFileName);
	  return 0;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "wb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open output file %s.", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  if (ud->format_for == utd)
    k = utd_transcribe_text_file ();
  else
    k = transcribe_text_file ();
  if (!k)
    {
      freeEverything ();
      return 0;
    }
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lbu_logEnd ();
  return k;
}

LBUAPI int EXPORT_CALL
lbu_backTranslateString (const char *configFileList,
			 const char *inbuf, int inlen, widechar
			 * outbuf,
			 int *outlen,
			 const char *logFileName, const char
			 *settingsString, unsigned int mode)
{
  int k;
  logMessage(LOU_LOG_INFO, "Begin lbu_backTranslateString: inbuf=%s", inbuf);
  if (!read_configuration_file
      (configFileList, logFileName, settingsString, mode))
    return 0;
  if (inbuf == NULL || outbuf == NULL || outlen == NULL)
    return 0;
  ud->inbuf = inbuf;
  ud->inlen = inlen;
  ud->outbuf = outbuf;
  ud->outlen = *outlen;
  ud->inFile = ud->outFile = NULL;
  if (ud->format_for == utd)
  {
    logMessage(LOU_LOG_DEBUG, "ud->format_for=utd");
    k = utd_back_translate_braille_string ();
  }
  else
    k = back_translate_braille_string ();
  if (!k)
    {
      freeEverything ();
      return 0;
    }
  *outlen = ud->outlen_so_far;
  logMessage(LOU_LOG_INFO, "Finish lbu_backTranslateString");
  lbu_logEnd ();
  return 1;
}

LBUAPI int EXPORT_CALL lbu_backTranslateFile
  (const char *configFileList, const char *inFileName,
   const char *outFileName, const char *logFileName,
   const char *settingsString, unsigned int mode)
{
/* Back translate the braille file in inFileName into either an 
* xml file or a text file according to
* the specifications in configFileList. If there are errors, print an 
* error message and return 0.*/
  int k;
  widechar outbuf[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileList, logFileName, settingsString, mode))
    return 0;
  if (inFileName == NULL || outFileName == NULL)
    return 0;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "rb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open input file %s.", inFileName);
	  return 0;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "wb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open output file %s.", outFileName);
	  return 0;
	}
    }
  else
    ud->outFile = stdout;
  if (ud->format_for == utd)
    k = utd_back_translate_file ();
  else
    k = back_translate_file ();
  if (!k)
    {
      freeEverything ();
      return 0;
    }
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  lbu_logEnd ();
  return 1;
}

static char writeablePath[MAXNAMELEN];
static char *writeablePathPtr = NULL;

LBUAPI char *EXPORT_CALL
lbu_setWriteablePath (const char *path)
{
  writeablePathPtr = NULL;
  if (path == NULL)
    return NULL;
  strcpy (writeablePath, path);
  writeablePathPtr = writeablePath;
  return writeablePathPtr;
}

LBUAPI char *EXPORT_CALL
lbu_getWriteablePath ()
{
  return writeablePathPtr;
}

LBUAPI int EXPORT_CALL
lbu_charToDots (const char *tableList, const unsigned char *inbuf,
		unsigned char *outbuf, int length, const char *logFile,
		unsigned int mode)
{
  widechar *interBuf;
  int wcLength;
  int utf8Length;
  int result = 0;
  if (tableList == NULL || inbuf == NULL || outbuf == NULL)
    return 0;
  lbu_logFile (logFile);
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
  lbu_logEnd ();
  free (interBuf);
  return result;
}

LBUAPI int EXPORT_CALL
lbu_dotsToChar (const char *tableList, const unsigned char *inbuf,
		unsigned char *outbuf, int length, const char *logFile,
		unsigned int mode)
{
  widechar *interBuf;
  int wcLength;
  int utf8Length;
  int result = 0;
  if (tableList == NULL || inbuf == NULL || outbuf == NULL)
    return 0;
  lbu_logFile (logFile);
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
  lbu_logEnd ();
  free (interBuf);
  return result;
}

LBUAPI int EXPORT_CALL
lbu_checkTable (const char *tableList, const char *logFile, unsigned int mode)
{
  int result = 1;
  lbu_logFile (logFile);
  if (!lou_getTable (tableList))
    result = 0;
  lbu_logEnd ();
  return result;
}

LBUAPI void EXPORT_CALL
lbu_free ()
{
/* Free all memory used by liblouisutdml. You MUST call this function at 
* the END of your application.*/
  lbu_logEnd ();
  lou_free ();
  destroy_semantic_table ();
  if (ud != NULL)
    free (ud);
  ud = NULL;
}
