/* liblouisutdml Braille Transcription Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by the BRLTTY Team

   Copyright (C) 2004, 2005, 2006, 2009
   ViewPlus Technologies, Inc. www.viewplus.com and
   JJB Software, Inc. www.jjb-software.com

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Maintained by John J. Boyer john.boyer@jjb-software.com
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Jliblouisutdml.h"
#include <louisutdml.h>

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_liblouis_liblouisutdml_version
  (JNIEnv * env, jobject this)
{
  return (*env)->NewStringUTF (env, lbu_version ());
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    translateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_liblouisutdml_translateString (JNIEnv * env,
						 jobject this,
						 jstring
						 configFileList,
						 jbyteArray inbuf,
						 jcharArray
						 outbuf,
						 jintArray outlen,
						 jstring
						 logFile,
						 jstring
						 settingsString, jint mode)
{
  const jbyte *cfl = NULL;
  jbyte *inbufx = NULL;
  jint inlen = 0;
  jchar *outbufx = NULL;
  jint *outlenx = NULL;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inbufx = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufx == NULL)
    goto release;
  inlen = (*env)->GetArrayLength (env, inbuf);
  outbufx = (*env)->GetCharArrayElements (env, outbuf, NULL);
  if (outbufx == NULL)
    goto release;
  outlenx = (*env)->GetIntArrayElements (env, outlen, NULL);
  if (outlenx == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
  if (settings == NULL)
    goto release;
  result = lbu_translateString (cfl, inbufx, inlen, outbufx, outlenx,
				logf, settings, mode);
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, inbufx, inbuf, 0);
  if (outbufx != NULL)
    (*env)->ReleaseCharArrayElements (env, outbufx, outbuf, 0);
  if (outlenx != NULL)
    (*env)->ReleaseIntArrayElements (env, outlenx, outlen, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    backTranslateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_liblouisutdml_backTranslateString (JNIEnv * env,
						     jobject this,
						     jstring
						     configFileList,
						     jbyteArray
						     inbuf,
						     jcharArray
						     outbuf,
						     jintArray
						     outlen,
						     jstring
						     logFile,
						     jstring
						     settingsString,
						     jint mode)
{
  const jbyte *cfl = NULL;
  jbyte *inbufx = NULL;
  jint inlen = 0;
  jchar *outbufx = NULL;
  jint *outlenx = NULL;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inbufx = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufx == NULL)
    goto release;
  inlen = (*env)->GetArrayLength (env, inbuf);
  outbufx = (*env)->GetCharArrayElements (env, outbuf, NULL);
  if (outbufx == NULL)
    goto release;
  outlenx = (*env)->GetIntArrayElements (env, outlen, NULL);
  if (outlenx == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
  if (settings == NULL)
    goto release;
  result = lbu_backTranslateString (cfl, inbufx, inlen, outbufx,
				    outlenx, logf, settings, mode);
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, inbufx, inbuf, 0);
  if (outbufx != NULL)
    (*env)->ReleaseCharArrayElements (env, outbufx, outbuf, 0);
  if (outlenx != NULL)
    (*env)->ReleaseIntArrayElements (env, outlenx, outlen, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    translateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_translateFile
  (JNIEnv * env, jobject this, jstring configFileList, jstring inputFileName,
   jstring outputFileName, jstring logFile, jstring settingsString, jint mode)
{
  const jbyte *cfl = NULL;
  const jbyte *inFile = NULL;
  const jbyte *outFile = NULL;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inFile = (*env)->GetStringUTFChars (env, inputFileName, NULL);
  if (inFile == NULL)
    goto release;
  outFile = (*env)->GetStringUTFChars (env, outputFileName, NULL);
  if (outFile == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
  if (settings == NULL)
    goto release;
  result = lbu_translateFile (cfl, inFile, outFile, logf, settings, mode);
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inFile != NULL)
    (*env)->ReleaseStringUTFChars (env, inputFileName, inFile);
  if (outFile != NULL)
    (*env)->ReleaseStringUTFChars (env, outputFileName, outFile);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    translateTextFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_liblouisutdml_translateTextFile (JNIEnv * env,
						   jobject this,
						   jstring
						   configFileList,
						   jstring inputFileName,
						   jstring
						   outputFileName,
						   jstring
						   logFile,
						   jstring
						   settingsString, jint mode)
{
  const jbyte *cfl = NULL;
  const jbyte *inFile = NULL;
  const jbyte *outFile = NULL;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inFile = (*env)->GetStringUTFChars (env, inputFileName, NULL);
  if (inFile == NULL)
    goto release;
  outFile = (*env)->GetStringUTFChars (env, outputFileName, NULL);
  if (outFile == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
  if (settings == NULL)
    goto release;
  result = lbu_translateTextFile (cfl, inFile, outFile, logf, settings, mode);
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inFile != NULL)
    (*env)->ReleaseStringUTFChars (env, inputFileName, inFile);
  if (outFile != NULL)
    (*env)->ReleaseStringUTFChars (env, outputFileName, outFile);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    backTranslateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_liblouisutdml_backTranslateFile (JNIEnv * env,
						   jobject this,
						   jstring
						   configFileList,
						   jstring inputFileName,
						   jstring
						   outputFileName,
						   jstring
						   logFile,
						   jstring
						   settingsString, jint mode)
{
  const jbyte *cfl = NULL;
  const jbyte *inFile = NULL;
  const jbyte *outFile = NULL;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inFile = (*env)->GetStringUTFChars (env, inputFileName, NULL);
  if (inFile == NULL)
    goto release;
  outFile = (*env)->GetStringUTFChars (env, outputFileName, NULL);
  if (outFile == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
  if (settings == NULL)
    goto release;
  result = lbu_backTranslateFile (cfl, inFile, outFile, logf, settings, mode);
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inFile != NULL)
    (*env)->ReleaseStringUTFChars (env, inputFileName, inFile);
  if (outFile != NULL)
    (*env)->ReleaseStringUTFChars (env, outputFileName, outFile);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    charToDots
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_charToDots
  (JNIEnv * env, jobject this, jstring tableList, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  jbyte *inbufx = NULL;
  jbyte *outbufx = NULL;
  jint outlen = 0;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufx = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufx == NULL)
    goto release;
  outbufx = (*env)->GetByteArrayElements (env, outbuf, NULL);
  if (outbufx == NULL)
    goto release;
  outlen = (*env)->GetArrayLength (env, outbuf);
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  result = lbu_charToDots (tableListX, inbufx, outbufx, outlen, logf, mode);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, inbufx, inbuf, 0);
  if (outbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, outbufx, outbuf, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    dotsToChar
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_dotsToChar
  (JNIEnv * env, jobject this, jstring tableList, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  jbyte *inbufx = NULL;
  jbyte *outbufx = NULL;
  jint outlen = 0;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufx = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufx == NULL)
    goto release;
  outbufx = (*env)->GetByteArrayElements (env, outbuf, NULL);
  if (outbufx == NULL)
    goto release;
  outlen = (*env)->GetArrayLength (env, outbuf);
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  result = lbu_dotsToChar (tableListX, inbufx, outbufx, outlen, logf, mode);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, inbufx, inbuf, 0);
  if (outbufx != NULL)
    (*env)->ReleaseByteArrayElements (env, outbufx, outbuf, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    checkTable
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_checkTable
  (JNIEnv * env, jobject this, jstring tableList, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  result = lbu_checkTable (tableListX, logf, mode);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    compileString
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_compileString
  (JNIEnv * env, jobject this, jstring tableList, jstring newEntry,
   jstring logFile)
{
  const jbyte *tableListX = NULL;
  const jbyte *newEntryX = NULL;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  newEntryX = (*env)->GetStringUTFChars (env, newEntry, NULL);
  if (newEntryX == NULL)
    goto release;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    goto release;
  lou_logFile (logf);
  result = lou_compileString (tableListX, newEntryX);
  lou_logEnd ();
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (newEntryX != NULL)
    (*env)->ReleaseStringUTFChars (env, newEntry, newEntryX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    setDataPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_liblouisutdml_setDataPath
  (JNIEnv * env, jobject this, jstring path)
{
  const jbyte *pathX = NULL;
  pathX = (*env)->GetStringUTFChars (env, path, NULL);
  if (pathX == NULL)
    goto release;
  lou_setDataPath ((char *) pathX);
release:
  if (pathX != NULL)
    (*env)->ReleaseStringUTFChars (env, path, pathX);
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    charSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_liblouis_liblouisutdml_charSize
  (JNIEnv * env, jobject this)
{
  return CHARSIZE;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    setWriteablePath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_liblouisutdml_setWriteablePath
  (JNIEnv *env, jobject this, jstring path)
{
  const jbyte *pathX = NULL;
  pathX = (*env)->GetStringUTFChars (env, path, NULL);
  if (pathX == NULL)
    goto release;
  lbu_setWriteablePath ((char *) pathX);
release:
  if (pathX != NULL)
    (*env)->ReleaseStringUTFChars (env, path, pathX);
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_liblouisutdml_free
  (JNIEnv * env, jobject this)
{
  lbu_free ();
  return;
}

/*
 * Class:     org_liblouis_liblouisutdml
 * Method:    file2brl
 * Signature: ([Ljava/lang/String;)Z
 */

/* Helper function for this method */
static const jbyte *JNICALL
getArg (JNIEnv * env, jobject this, jobjectArray args, jint index)
{
  static jobject curObj = NULL;
  static const jbyte *curArg = NULL;
  if (args == NULL)
    return NULL;
  if (curObj != NULL || index == -1)
    {
      if (curArg != NULL)
	(*env)->ReleaseStringUTFChars (env, curObj, curArg);
      curArg = NULL;
      curObj = NULL;
    }
if (index >= 0)
{
  curObj = (*env)->GetObjectArrayElement (env, args, index);
  if (curObj == NULL)
    return NULL;
  curArg = (*env)->GetStringUTFChars (env, curObj, NULL);
}
  return curArg;
}

JNIEXPORT jboolean JNICALL Java_org_liblouis_liblouisutdml_file2brl
  (JNIEnv * env, jobject this, jobjectArray args)
{
  jint numArgs = (*env)->GetArrayLength (env, args);
  int mode = dontInit;
  char configFileList[MAXNAMELEN];
  char inputFileName[MAXNAMELEN];
  char outputFileName[MAXNAMELEN];
  char tempFileName[MAXNAMELEN];
  char logFileName[MAXNAMELEN];
  char whichProc = 0;
  char *configSettings = NULL;
  FILE *inputFile = NULL;
  FILE *tempFile;
  int ch = 0;
  int pch = 0;
  int nch = 0;
  int charsRead = 0;
  int k;
  const char *curArg = NULL;
  strcpy (configFileList, "preferences.cfg");
  strcpy (inputFileName, "stdin");
  strcpy (outputFileName, "stdout");
  strcpy (tempFileName, "file2brl.temp");
  strcpy (logFileName, "file2brl.log");
  UserData *ud;
  if (numArgs != 0)
    {
      getArg (env, this, args, -1);
      k = 0;
      while (k < numArgs)
	{
	  curArg = getArg (env, this, args, k);
	  if (curArg == NULL)
	    break;
	  if (curArg[0] == '-')
	    {
	      switch (curArg[1])
		{
		case 'l':
		  strcpy (logFileName, getArg (env, this, args, k + 1));
		  k += 2;
		  break;
		case 't':
		  mode |= htmlDoc;
		  k++;
		  break;
		case 'f':
		  strcpy (configFileList, getArg (env, this, args, k + 1));
		  k += 2;
		  break;
		case 'b':
		case 'p':
		case 'r':
		case 'x':
		  whichProc = curArg[1];
		  k++;
		  break;
		case 'C':
		  if (configSettings == NULL)
		    {
		      configSettings = malloc (BUFSIZE);
		      configSettings[0] = 0;
		    }
		  strcat (configSettings, getArg (env, this, args, k + 1));
		  k += 2;
		  strcat (configSettings, "\n");
		  break;
		case '-':
		  /* Input file is stdin but output file is not stdout. */
		  k++;
		  break;
		default:
		  lou_logPrint ("invalid argument%s", curArg);
		  return JNI_FALSE;
		}
	      continue;
	    }
	  if (k < numArgs)
	    {
	      if (k == numArgs - 1)
		{
		  strcpy (inputFileName, curArg);
		  k++;
		}
	      else if (k == numArgs - 2)
		{
		  strcpy (inputFileName, curArg);
		  strcpy (outputFileName, getArg (env, this, args, k + 1));
		  k += 2;
		}
	      else
		{
		  lou_logPrint ("extra operand: %s\n",
				getArg (env, this, args, k + 2));
		  return JNI_FALSE;
		}
	    }
	  k++;
	}
      getArg (env, this, args, -1);
    }
  lou_logFile (logFileName);
  if (whichProc == 0)
    whichProc = 'x';
  if (configSettings != NULL)
    for (k = 0; configSettings[k]; k++)
      if (configSettings[k] == '=' && configSettings[k - 1] != ' ')
	configSettings[k] = ' ';
  if ((ud =
       lbu_initialize (configFileList, logFileName, configSettings)) == NULL)
    {
      lou_logEnd ();
      return JNI_FALSE;
    }
  if (strcmp (inputFileName, "stdin") != 0)
    {
      if (!(inputFile = fopen (inputFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", inputFileName);
	  lou_logEnd ();
	  return JNI_FALSE;
	}
    }
  else
    inputFile = stdin;
  /*Create somewhat edited temporary file to facilitate use of stdin. */
  strcpy (tempFileName, ud->writeable_path);
  strcat (tempFileName, "file2brl.temp");
  if (!(tempFile = fopen (tempFileName, "w")))
    {
      lou_logPrint ("Can't open temporary file.\n");
      lou_logEnd ();
      return JNI_FALSE;
    }
  if (whichProc == 'p')
    {
      int ppch = 0;
      int firstCh = 0;
      int skipit = 0;
      while ((ch = fgetc (inputFile)) != EOF)
	{
	  if (firstCh == 0)
	    firstCh = ch;
	  if (ch == 12 || ch == 13)
	    continue;
	  if (ch == '<' && firstCh == '<')
	    {
	      skipit = 1;
	      continue;
	    }
	  if (skipit)
	    {
	      if (ch == '>')
		skipit = 0;
	      continue;
	    }
	  if (ch == '-')
	    {
	      nch = fgetc (inputFile);
	      if (nch == 10)
		continue;
	      ungetc (nch, inputFile);
	    }
	  if (!((pch == 10 && ch == 10) || (ppch == 10 && pch == 10)))
	    {
	      if (ch <= 32 && pch <= 32)
		continue;
	      if (!
		  (pch == 10 && ((ppch >= 97 && ppch <= 122) || ppch == ',')))
		{
		  if (pch == 10 && ch < 97)
		    fputc (10, tempFile);
		}
	    }
	  ppch = pch;
	  pch = ch;
	  fputc (ch, tempFile);
	  charsRead++;
	}
    }
  else
    while ((ch = fgetc (inputFile)) != EOF)
      {
	if (charsRead == 0 && ch <= ' ')
	  continue;
	if (ch == 13)
	  continue;
	if (charsRead == 0)
	  {
	    if (ch != '<' && whichProc == 'x')
	      whichProc = 't';
	    nch = fgetc (inputFile);
	    if (!(mode & htmlDoc) && whichProc == 'x' && nch != '?')
	      fprintf (tempFile, "%s\n", ud->xml_header);
	  }
	if (pch == '>' && ch == '<')
	  fputc (10, tempFile);
	fputc (ch, tempFile);
	pch = ch;
	charsRead++;
	if (charsRead == 1)
	  {
	    fputc (nch, tempFile);
	    charsRead++;
	  }
      }
  fclose (tempFile);
  if (inputFile != stdin)
    fclose (inputFile);
  if (charsRead > 2)
    switch (whichProc)
      {
      case 'b':
	lbu_backTranslateFile (configFileList, tempFileName, outputFileName,
			       NULL, NULL, mode);
	break;
      case 'r':
	{
	  char temp2FileName[MAXNAMELEN];
	  strcpy (temp2FileName, ud->writeable_path);
	  strcat (temp2FileName, "file2brl2.temp");
	  if ((lbu_backTranslateFile
	       (configFileList, tempFileName, temp2FileName, NULL,
		NULL, mode)) != 1)
	    {
	      lou_logEnd ();
	      return JNI_FALSE;
	    }
	  if (ud->back_text == html)
	    lbu_translateFile (configFileList,
			       temp2FileName,
			       outputFileName, NULL, NULL, mode);
	  else
	    lbu_translateTextFile (configFileList, temp2FileName,
				   outputFileName, NULL, NULL, mode);
	}
	break;
      case 't':
      case 'p':
	lbu_translateTextFile (configFileList, tempFileName, outputFileName,
			       NULL, NULL, mode);
	break;
      case 'x':
	lbu_translateFile (configFileList, tempFileName, outputFileName, NULL,
			   NULL, mode);
	break;
      default:
	lou_logPrint ("Program bug %c\n", whichProc);
	break;
      }
  if (configSettings != NULL)
    free (configSettings);
  lou_logEnd ();
  return JNI_TRUE;
}
