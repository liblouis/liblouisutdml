/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   Abilitiessoft, Inc. www.abilitiessoft.org
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

package org.liblouis;

import java.io.File;

public final class LibLouisUTDML {
/** 
* Bindings for the liblouisutdml and liblouis C libraries.
* These bindings include enough functionality so that it should never 
* be necessary to call liblouis directly. This saves the hassle of 
* dealing with two sets of bindings. liblouisutdml can produce a variety 
* of output types according to the value of the formatFor configuration 
* setting. To get UTDML formatFor is set to utd.
*/

/** 
* Definitions of mode bits 
*
* The bits for liblouis are included because it is sometimes necessary 
* to pass a liblouis mode to a liblouisutdml function. The liblouis bits 
* take up the low-order bits of the mode integer, while the 
* liblouisutdml bits take up the high-order bits, except that the 
* sign bit is skipped.
*/
public static final int NoContractions = 1;
public static final int CompbrlAtCursor = 1<<1;
public static final int DotsIO = 1<<2;
public static final int Comp8Dots = 1<<3;
public static final int Pass1Only = 1<<4;
public static final int CompbrlLeftCursor = 1<<5;
public static final int OtherTrans = 1<<6;
public static final int UcBrl = 1<<7;
public static final int DontInit = 1<<30;
public static final int HtmlDoc = 1<<29;
public static final int notUC = 1<<28;

/**
* This class is a singleton.
*/

private static LibLouisUTDML singleInstance = new LibLouisUTDML();
private static boolean libraryLoaded = false;

  private LibLouisUTDML () {
  }
  
  public static LibLouisUTDML getInstance()
  {
    return singleInstance;
  }
  
  public static native void initialize (String dataPath, String 
  writeablePath, String logFile);
  
  public static void loadLibrary (String libraryPath, String 
  librarySuffix) throws Exception {
    if (libraryPath == null || librarySuffix == null)
    {
      System.out.println("prefix or suffix is null");
      throw new Exception (
      "Could not load libraries. libraryPath or librarySuffix undefined.");
    }
    LibLouis.loadLibrary(libraryPath, librarySuffix);
  }
  
  
  /** Return a string giving the versions of both liblouisutdml and 
   * liblouis */
  public native String version ();

  /**
   * Load a XML catalog into LibLouisUTDML.
   */
  public native void loadXMLCatalog(String filename);
/** Make a braille translation of the UTF-8 characters in inbuf 
according to configuration settings to UTF-8 characters in outbuf. The 
latter will be in Unicode braille. The return value is true if the 
translation is successful and false if not. Any errors are recorded in 
logFile. If this is null, they are printed on stderr. There may be 
errors even if the return value is true. If there are none, the log file 
will be empty. settingsstring may be used to pass in configuration 
settings. */

  public native boolean translateString (String configFileList,
                                         byte[]inbuf,
                                         byte[]outbuf, int[]outlen,
                                         String logFilename,
                                         String settingsString, int mode);
  
  /** the brf characters in inbuf are translated to print characters in 
      outbuf according to the settings in the configuration files and 
      setingsStrring. The translation will be in UTF-8.
  */
  
  public native boolean backTranslateString (String configFileList,
                                             byte[]inbuf,
                                             byte[]outbuf, 
                                             int[]outlen,
                                             String logFilename,
                                             String settingsSrting,
                                             int mode);
  
  /** The xml document in inputFile is translated into braille and the 
      translation is placed 
      in outputFile. The return value and log file are as described above.
  */
  
  public native boolean translateFile (String configFileList, 
                                       String inputFileName,
                                       String outputFileName,
                                       String logFileName,
                                       String settingsString, int mode);
  
  /** The plain-text file in inFile is translated to braille and the 
      translation placed in outputFile as described for the previous method. 
      If the text contains blank lines they are treated as paragraph breaks.
  */

  public native boolean translateTextFile (String configFileList,
					       String inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

/* Back-translate the brf file in inFile into braille in outFile 
according to configuration specifications.
*/

  public native boolean backTranslateFile (String configFileList,
					       String inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

/** Convert the utf8 character string in inbuf to Unicode braille dot 
patterns and place the result as a utf8 string in outbuf. */

public native boolean
charToDots (String tableList, byte[]inbuf,
		byte[]outbuf, String logFile,
		int mode);

/** Convert the utf8 string of dot patterns in inbuf to characters and 
place the result as a utf8 string in outbuf. */

public native boolean
dotsToChar (String tableList, byte[]inbuf,
		byte[]outbuf, String logFile,
		int mode);

/** See if the table in tableList exists and is valid. If no errors are 
found logFile will be empty. */

public native boolean
checkTable (String tableList, String logFile, int mode);

/**
* Path on which liblouis tables and liblouisutdml files can be found. 
*/

public void setDataPath(String path)
{
    LibLouis.getInstance().setDataPath(path);
}

/**
* This method performs the functions of the file2brl program in a more 
* contrrolled environment.
*/

public native boolean file2brl (String[] args);

/**
* Set the path to which temporary files will be written.
*/
public native void setWriteablePath (String path);

/** You must call free at the end of your application to free all 
memory used by liblouisutdml and liblouis. Do NOT call it after every 
call to a liblouisutdml method. This will result in great 
inefficiency. The memory used by liblouisutdml for each document is 
freed wen the method completes, but some memory holding configuration 
settings is held. The memory used by liblouis is freed only when the 
free method is called.
*/

  public native void free ();

public native void setLogFile (String fileName);

public native void logEnd();

public native void registerLogCallback(LogCallback cb);

public native void setLogLevel(int level);
}
