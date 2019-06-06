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

   Maintained by John J. Boyer john.boyer@abilitiessoft.org
   */

#ifndef __LIBLOUISUTDML_H_
#define __LIBLOUISUTDML_H_
#include <liblouis.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/* Function prototypes are documented briefly below. For more extensive 
documentation see liblouisutdml.html or type info liblouisutdml. */

#ifdef _WIN32
#ifdef LBULIB
#define LBUAPI __declspec(dllexport)
#else
#define LBUAPI __declspec(dllimport)
#endif
#else
#define LBUAPI
#endif

#ifndef EXPORT_CALL
#ifdef _WIN32
#define EXPORT_CALL __stdcall
  LBUAPI char *EXPORT_CALL lou_getProgramPath (void);
#else
#define EXPORT_CALL
#endif
#endif

  LBUAPI char *EXPORT_CALL lbu_version (void);
/* Returns the version of liblouisutdml and liblouis. */

  LBUAPI void EXPORT_CALL lbu_loadXMLCatalog(const char *fiilename);
  LBUAPI void *EXPORT_CALL lbu_initialize (const char *configFileList,
				    const char *logFileName,
				    const char *settingsString);

/* This function initializes the libxml2 library, runs liblouisutdml.ini and
processes the configuration file given in configFileList, sets up a log
file if logFileName is not NULL and processes the settings in
settingsString if this is not null. It returns a pointer to the UserData
structure. This pointer is void and must be cast to (UserData *) in the
calling program. To access the information in this structure you must
include louisutdml.h */

/* Enumeration values to be used in mode parameter. may be ored.
* liblouis uses the low-order bits, liblouisutdml uses the high-order 
* bits,*/
  typedef enum
  {
    doInit = 1 << 30,
    htmlDoc = 1 << 29,
    notUC = 1 << 28,
    notSync = 1<<27,
    utdInput = 1<<26,
    convertOnly = 1<<25,
    louisDots = 1<<24
  } ProcessingModes;

  LBUAPI int EXPORT_CALL lbu_translateString
    (const char *configFileList,
     const char *inbuf, int inlen, widechar *outbuf, int *outlen,
     const char *logFileName, const char *settingsString, unsigned int mode);

/* This function takes a well-formed xml expression in inbuf and
translates it into a string of 16- or 32-bit braille characters in
outbuf.  The xml expression must be immediately followed by a zero or
null byte. If it does not begin with an xul header, one is added. The
header is specified by the xmlHeader line in the configuration file. If
no such line is present, a default header specifying UTF-8 encoding is
used. The configFileList parameter points to a configuration file.
liblouisutdml.ini is processed before this file. Note that the *outlen
parameter is a pointer to an integer. When the function is called, this
integer contains the maximum output length. When it returns, it is set
to the actual length used.  The mode parameter is used to pass options
to liblouisutdml which are applied before any configuration fpe is
processed. For now, if mode is 0, a full initialization is cone. If it
is 1 only a few things are reset to prepare for a new document. The
function returns 1 if no errors were encountered and a negative number
if a conplete translation could not be done.  */


  LBUAPI int EXPORT_CALL lbu_backTranslateString
    (const char *configFileList,
     const char *inbuf, int inlen, widechar *outbuf, int *outlen,
     const char *logFileName, const char *settingsString, unsigned int mode);

  LBUAPI int EXPORT_CALL lbu_translateFile (const char *configFileList, const char
				     *inputFileName,
				     const char *outputFileName,
				     const char *logFileName,
				     const char *settingsString,
				     unsigned int mode);

  LBUAPI int EXPORT_CALL lbu_translateTextFile (const char *configFileList,
					 const char *inputFileName,
					 const char *outputFileName,
					 const char *logFileName,
					 const char *settingsString,
					 unsigned int mode);
  LBUAPI int EXPORT_CALL lbu_backTranslateFile (const char *configFileList,
					 const char *inputFileName,
					 const char *outputFileName,
					 const char *logFileName,
					 const char *settingsString,
					 unsigned int mode);

  LBUAPI int EXPORT_CALL
    lbu_charToDots (const char *tableList, const unsigned char *inbuf,
		    unsigned char *outbuf, int length, const char 
*logFile,
		    unsigned int mode);

/* Convert the utf8 character string in inbuf to Unicode braille dot 
patterns and place the result as a utf8 string in outbuf. */

  LBUAPI int EXPORT_CALL
    lbu_dotsToChar (const char *tableList, const unsigned char *inbuf,
		    unsigned char *outbuf, int length, const char 
*logFile,
		    unsigned int mode);

/* Convert the utf8 string of dot patterns in inbuf to characters and 
place the result as a utf8 string in outbuf. */

  LBUAPI int EXPORT_CALL
    lbu_checkTable (const char *tableList, const char *logFile, unsigned 
int 
mode);

/* See if the table in tableList exists and is valid. If no errors are 
found logFile will be empty. */

/* Set/get  the path to which temporary files will be written */
LBUAPI char *EXPORT_CALL lbu_setWriteablePath (const char *path);
LBUAPI char *EXPORT_CALL lbu_getWriteablePath ();

LBUAPI void EXPORT_CALL lbu_registerLogCallback(logcallback callback);

LBUAPI void EXPORT_CALL lbu_setLogLevel(logLevels level);

LBUAPI void EXPORT_CALL lbu_logFile(const char *fileName);

LBUAPI void EXPORT_CALL lbu_logEnd();

LBUAPI void EXPORT_CALL /*lbu_*/logMessage(logLevels level, const char *format, ...);

/* This function should be called at the end of the application to free
all memory allocated by liblouisutdml or liblouis. */
LBUAPI void EXPORT_CALL lbu_free (void);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/*__LIBLOUISUTDML_H_ */
