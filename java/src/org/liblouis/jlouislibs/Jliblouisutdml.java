/* BrailleBlaster Braille Transcription Application
  *
  * Copyright (C) 2010, 2012
  * ViewPlus Technologies, Inc. www.viewplus.com
  * and
  * Abilitiessoft, Inc. www.abilitiessoft.com
  * All rights reserved
  *
  * This file may contain code borrowed from files produced by various
  * Java development teams. These are gratefully acknowledged.
  *
  * This file is free software; you can redistribute it and/or modify it
  * under the terms of the Apache 2.0 License, as given at
  * http://www.apache.org/licenses/
  *
  * This file is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
  * See the Apache 2.0 License for more details.
  *
  * You should have received a copy of the Apache 2.0 License along with
  * this program; see the file COPYING.
  * If not, see
  * http://www.apache.org/licenses/
  *
  * Maintained by John J. Boyer john.boyer@abilitiessoft.com
*/

package org.liblouis.jlouislibs;

public class Jliblouisutdml
{
/** Bindings for liblouisutdml 
* These bindings include enough functionality so that it should never 
be necessary to call liblouis directly. This saves the hassle of dealing 
with two sets of bindings. liblouisutdml can produce a variety of output 
types according to the value of the formatFor configuration setting. To 
get UTDML formatFor is set to utd.
*/

/** Definitions of mode bits */

/** The bits for liblouis are included because it is sometimes necessary 
to pass a liblouis mode to a liblouisutdml function. The liblouis bits 
take up the low-order bits of the mode integer, while the liblouisutdml 
bits take up the high-order bits, except that the sign bit is skipped.
*/

public static final int noContractions = 1;
public static final int compbrlAtCursor = 1<<1;
public static final int dotsIO = 1<<2;
public static final int comp8Dots = 1<<3;
public static final int pass1Only = 1<<4;
public static final int compbrlLeftCursor = 1<<5;
public static final int otherTrans = 1<<6;
public static final int ucBrl = 1<<7;
/** liblouisutdml mode bits*/
  public static final int dontInit = 1<<30;
  public static final int htmlDoc = 1<<29;

/** Return a string giving the versions of both liblouisutdml and 
* liblouis */
  public native String lbu_version ();

/** Make a braille translation of the UTF-8 characters in inbuf 
according to configuration settings to UTF-8 characters in outbuf. The 
latter will be in Unicode braille. The return value is true if the 
translation is successful and false if not. Any errors are recorded in 
logFile. If this is null, they are printed on stderr. There may be 
errors even if the return value is true. If there are none, the log file 
will be empty. settingsstring may be used to pass in configuration 
settings. */

  public native boolean lbu_translateString (String configFileList,
					     byte[]inbuf,
					     byte[]outbuf, int[]outlen,
					     String logFilename,
					     String settingsSrting, int mode);

/** the brf characters in inbuf are translated to print characters in 
outbuf according to the settings in the configuration files and 
setingsStrring. The translation will be in UTF-8.
*/

  public native boolean lbu_backTranslateString (String configFileList,
						 byte[]inbuf,
						 byte[]outbuf, int[]outlen,
						 String logFilename,
						 String settingsSrting,
						 int mode);

/** The xml document in inputFile is translated into braille and the 
translation is placed 
in outputFile. The return value and log file are as described above.
*/

  public native boolean lbu_translateFile (String configFileList, String
					   inputFileName,
					   String outputFileName,
					   String logFileName,
					   String settingsString, int mode);

/** The plain-text file in inFile is translated to braille and the 
translation placed in outputFile as described for the previous method. 
If the text contains blank lines they are treated as paragraph breaks.
*/

  public native boolean lbu_translateTextFile (String configFileList,
					       String
					       inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

/* Back-translate the brf file in inFile into braille in outFile 
according to configuration specifications.
*/

  public native boolean lbu_backTranslateFile (String configFileList,
					       String
					       inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

/** Convert the utf8 character string in inbuf to Unicode braille dot 
patterns and place the result as a utf8 string in outbuf. */

public native void
lbu_charToDots (String trantab, byte[]inbuf,
		byte[]outbuf, String logFile,
		int mode);

/** Convert the utf8 string of dot patterns in inbuf to characters and 
place the result as a utf8 string in outbuf. */

public native void
lbu_dotsToChar (String trantab, byte[]inbuf,
		byte[]outbuf, String logFile,
		int mode);

/** See if the table in trantab exists and is valid. If no errors are 
found logFile will be empty. */

public native void
lbu_checkTable (String trantab, String logFile, int mode);


/** Return the character size used internally by liblouis and 
liblouisutdml. */

public native int lbu_charSize ();

/** You must call lbu_free at the end of your application to free all 
memory used by liblouisutdml and liblouis. Do NOT call it after every 
call to a liblouisutdml method. This will result in great 
inefficiency. The memory used by liblouisutdml for each document is 
freed wen the method completes, but some memory holding configuration 
settings is held. The memory used by liblouis is freed only when the 
free method is called.
*/

  public native void lbu_free ();

/** Default donstructor. We may want it to do something in the future.
*/

  public Jliblouisutdml ()
  {
  }

// This is an initializer which loads the liblouisutdml library.

  static
  {
    System.loadLibrary ("louisutdml");
  }
}
