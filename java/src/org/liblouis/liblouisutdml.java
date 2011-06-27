package org.liblouis;

/**
* Bindings for liblouisutdml
*/

public final class liblouisutdml
{
/** 
* These bindings include enough functionality so that it should never 
be necessary to call liblouis directly. This saves the hassle of dealing 
with two sets of bindings. liblouisutdml can produce a variety of output 
types according to the value of the formatFor configuration setting. To 
get UTDML formatFor is set to utd.
*/

/**
* This class is a singleton.
*/

private static liblouisutdml singleInstance = new liblouisutdml();
private static boolean libraryLoaded = false;

private liblouisutdml () {
}

public static liblouisutdml getInstance()
{
return singleInstance;
}

/**
* The libary name must be a complete path.
*/

public static void loadLibrary ()
throws Exception {
if (!libraryLoaded)
{
System.loadLibrary ("louisutdml");
libraryLoaded = true;
}
else
{
throw new Exception ("Attempt to reload library.");
}
}

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
  public native String version ();

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
					     char[]outbuf, int[]outlen,
					     String logFilename,
					     String settingsSrting, int mode);

/** the brf characters in inbuf are translated to print characters in 
outbuf according to the settings in the configuration files and 
setingsStrring. The translation will be in UTF-8.
*/

  public native boolean backTranslateString (String configFileList,
						 byte[]inbuf,
						 char[]outbuf, int[]outlen,
						 String logFilename,
						 String settingsSrting,
						 int mode);

/** The xml document in inputFile is translated into braille and the 
translation is placed 
in outputFile. The return value and log file are as described above.
*/

  public native boolean translateFile (String configFileList, String
					   inputFileName,
					   String outputFileName,
					   String logFileName,
					   String settingsString, int mode);

/** The plain-text file in inFile is translated to braille and the 
translation placed in outputFile as described for the previous method. 
If the text contains blank lines they are treated as paragraph breaks.
*/

  public native boolean translateTextFile (String configFileList,
					       String
					       inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

/* Back-translate the brf file in inFile into braille in outFile 
according to configuration specifications.
*/

  public native boolean backTranslateFile (String configFileList,
					       String
					       inputFileName,
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
* Add a new entrry to a table.
*/
public native boolean compileString (String tableList, String newEntry, 
String logFile);

/**
* Path on which liblouis tables and liblouisutdml files can be found. 
*/

public native void setDataPath(String path);

/** Return the character size used internally by liblouis and 
liblouisutdml. */

public native int charSize ();

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

public native void logMessage (String message);

Public native void logEnd ();

}
