//package org.brailleblaster.jllouislibs;

public class Jliblouisutdml
{
  public static final int dontInit = 1;
  public static final int htmlDoc = 2;

  public native String lbu_version ();

  public native boolean lbu_translateString (String configFileList,
					     byte[]inbuf,
					     byte[]outbuf, int[]outlen,
					     String logFilename,
					     String settingsSrting, int mode);

  public native boolean lbu_backTranslateString (String configFileList,
						 byte[]inbuf,
						 byte[]outbuf, int[]outlen,
						 String logFilename,
						 String settingsSrting,
						 int mode);

  public native boolean lbu_translateFile (String configFileList, String
					   inputFileName,
					   String outputFileName,
					   String logFileName,
					   String settingsString, int mode);

  public native boolean lbu_translateTextFile (String configFileList,
					       String
					       inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

  public native boolean lbu_backTranslateFile (String configFileList,
					       String
					       inputFileName,
					       String outputFileName,
					       String logFileName,
					       String settingsString,
					       int mode);

  public native void lbu_free ();

  public Jliblouisutdml ()
  {
  }

  static
  {
    System.loadLibrary ("liblouisutdml");
  }
}
