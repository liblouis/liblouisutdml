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

#define EMPTY -1000

// A pointer to the JVM for callbacks
static JavaVM *jvm;
static void execJavaLogCallback(jobject cb, int level, const char *message)
{
  JNIEnv *env;
  jint rs;
  jstring jstrMsg;
  jclass cls;
  jmethodID mid;
  if ((jvm == NULL) || (cb == NULL))
    return;
  rs = (*jvm)->AttachCurrentThread(jvm, (void **)&env, NULL);
  if (rs != JNI_OK)
    return;
  cls = (*env)->GetObjectClass(env, cb);
  if (cls == NULL)
    return;
  mid = (*env)->GetMethodID(env, cls, "logMessage", "(ILjava/lang/String;)V");
  if (mid == NULL)
    return;
  jstrMsg = (*env)->NewStringUTF(env, message);
  (*env)->CallVoidMethod(env, cb, mid, level, jstrMsg);
}
/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    initialize
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_initialize
  (JNIEnv *env, jclass class, jstring dataPath, jstring
writeablePath, jstring logfile)
{
  const jbyte *dataPathX = NULL;
  const jbyte *writeablePathX = NULL;
  const jbyte *logfileX = NULL;
  char *tmpDataPath;
  dataPathX = (*env)->GetStringUTFChars (env, dataPath, NULL);
  if (dataPathX == NULL)
    goto release;
  writeablePathX = (*env)->GetStringUTFChars (env, writeablePath, NULL);
  if (writeablePathX == NULL)
    goto release;
  if (logfile != NULL)
  {
    logfileX = (*env)->GetStringUTFChars (env, logfile, NULL);
    if (logfileX == NULL)
      goto release;
  }
  tmpDataPath = strdup(dataPathX);
  lou_setDataPath (tmpDataPath);
  free(tmpDataPath);
  lbu_setWriteablePath (writeablePathX);
  read_configuration_file (NULL, logfileX, NULL, 0);
release:
  if (dataPathX != NULL)
    (*env)->ReleaseStringUTFChars (env, dataPath, dataPathX);
  if (writeablePathX != NULL)
    (*env)->ReleaseStringUTFChars (env, writeablePath, writeablePathX);
  if (logfileX != NULL)
    (*env)->ReleaseStringUTFChars (env, logfile, logfileX);
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_liblouis_LibLouisUTDML_version
  (JNIEnv * env, jobject obj)
{
  return (*env)->NewStringUTF (env, lbu_version ());
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    loadXMLCatalog
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_loadXMLCatalog
  (JNIEnv *env, jobject this, jstring filename)
{
  const char *cFilename = (*env)->GetStringUTFChars(env, filename, 0);
  lbu_loadXMLCatalog(cFilename);
  (*env)->ReleaseStringUTFChars(env, filename, cFilename);
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    translateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_LibLouisUTDML_translateString (JNIEnv * env,
						 jobject obj,
						 jstring
						 configFileList,
						 jbyteArray inbuf,
						 jbyteArray
						 outbuf,
						 jintArray outlen,
						 jstring
						 logFile,
						 jstring
						 settingsString, jint mode)
{
  const jbyte *cfl = NULL;
  jbyte *inbufX = NULL;
  jint inlen = 0;
  jbyte *outbufY = NULL;
  widechar *outbufX = NULL;
  jint outlenX = EMPTY;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inbufX = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufX == NULL)
    goto release;
  inlen = (*env)->GetArrayLength (env, inbuf);
  if (outbuf == NULL)
    goto release;
  (*env)->GetIntArrayRegion (env, outlen, 0, 1, &outlenX);
  if (outlenX == EMPTY)
    goto release;
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  if (settingsString != NULL)
    {
      settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
      if (settings == NULL)
	goto release;
    }
  outbufX = malloc ((outlenX + 4) * CHARSIZE);
  result = lbu_translateString (cfl, inbufX, inlen, outbufX, &outlenX,
				logf, settings, mode);
  if (result)
    {
      int wcLength;
      int utf8Length;
      if (ud->format_for == utd)
	{
	  (*env)->SetByteArrayRegion (env, outbuf, 0, outlenX,
				      (jbyte *) outbufX);
	  utf8Length = outlenX;
	}
      else
	{
	  wcLength = outlenX;
	  utf8Length = outlenX * CHARSIZE;
	  wc_string_to_utf8 (outbufX, &wcLength, outbufY, &utf8Length);
	  (*env)->SetByteArrayRegion (env, outbuf, 0, utf8Length, outbufY);
	}
      (*env)->SetIntArrayRegion (env, outlen, 0, 1, &utf8Length);
    }
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inbufX != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufX, 0);
  if (outbufX != NULL)
    free (outbufX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    backTranslateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_LibLouisUTDML_backTranslateString (JNIEnv * env,
						     jobject obj,
						     jstring
						     configFileList,
						     jbyteArray
						     inbuf,
						     jbyteArray
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
  jbyte *inbufX = NULL;
  jint inlen = 0;
  jbyte *outbufY = NULL;
  widechar *outbufX = NULL;
  jint outlenX = EMPTY;
  const jbyte *logf = NULL;
  const jbyte *settings = NULL;
  jboolean result = JNI_FALSE;
  cfl = (*env)->GetStringUTFChars (env, configFileList, NULL);
  if (cfl == NULL)
    goto release;
  inbufX = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufX == NULL)
    goto release;
  inlen = (*env)->GetArrayLength (env, inbuf);
  logMessage(LOU_LOG_DEBUG, "inlen=%d", inlen);
  if (outbuf == NULL)
    goto release;
  (*env)->GetIntArrayRegion (env, outlen, 0, 1, &outlenX);
  logMessage(LOU_LOG_DEBUG, "outlenX=%d", outlenX);
  if (outlenX == EMPTY)
    goto release;
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  if (settingsString != NULL)
    {
      settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
      if (settings == NULL)
	goto release;
    }
  outbufX = malloc ((outlenX + 4) * CHARSIZE);
  result = lbu_backTranslateString (cfl, inbufX, inlen, outbufX,
				    &outlenX, logf, settings, mode);
  if (result)
    {
      int wcLength;
      int utf8Length;
      logMessage(LOU_LOG_DEBUG, "After backTranslate outlenX=%d", outlenX);
      if (ud->format_for == utd)
	{
          logMessage(LOU_LOG_DEBUG, "Preparing to return UTD");
	  (*env)->SetByteArrayRegion (env, outbuf, 0, outlenX,
				      (jbyte *) outbufX);
	  utf8Length = outlenX;
	}
      else
	{
          logMessage(LOU_LOG_DEBUG, "Preparing to return non-UTD");
	  wcLength = outlenX;
	  utf8Length = outlenX;
          outbufY = (jbyte *)malloc(utf8Length);
	  wc_string_to_utf8 (outbufX, &wcLength, outbufY, &utf8Length);
	  (*env)->SetByteArrayRegion (env, outbuf, 0, utf8Length, outbufY);
          free(outbufY);
	}
      (*env)->SetIntArrayRegion (env, outlen, 0, 1, &utf8Length);
    }
release:
  if (cfl != NULL)
    (*env)->ReleaseStringUTFChars (env, configFileList, cfl);
  if (inbufX != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufX, 0);
  if (outbufX != NULL)
    free (outbufX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    translateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouisUTDML_translateFile
  (JNIEnv * env, jobject obj, jstring configFileList, jstring inputFileName,
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
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  if (settingsString != NULL)
    {
      settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
      if (settings == NULL)
	goto release;
    }
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
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    translateTextFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_LibLouisUTDML_translateTextFile (JNIEnv * env,
						   jobject obj,
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
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  if (settingsString != NULL)
    {
      settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
      if (settings == NULL)
	goto release;
    }
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
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    backTranslateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_LibLouisUTDML_backTranslateFile (JNIEnv * env,
						   jobject obj,
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
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  if (settingsString != NULL)
    {
      settings = (*env)->GetStringUTFChars (env, settingsString, NULL);
      if (settings == NULL)
	goto release;
    }
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
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    charToDots
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouisUTDML_charToDots
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  jbyte *inbufX = NULL;
  jbyte *outbufX = NULL;
  jint outlenX = 0;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufX = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufX == NULL)
    goto release;
  if (outbuf == NULL)
    goto release;
  outlenX = (*env)->GetArrayLength (env, outbuf);
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  outbufX = malloc (outlenX);
  result = lbu_charToDots (tableListX, inbufX, outbufX, outlenX, logf, 
mode);
  if (result)
    (*env)->SetByteArrayRegion (env, outbuf, 0, outlenX, outbufX);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufX != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufX, 0);
  if (outbufX != NULL)
    free (outbufX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    dotsToChar
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouisUTDML_dotsToChar
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  jbyte *inbufX = NULL;
  jbyte *outbufX = NULL;
  jint outlenX = 0;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufX = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufX == NULL)
    goto release;
  if (outbuf == NULL)
    goto release;
  outlenX = (*env)->GetArrayLength (env, outbuf);
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  outbufX = malloc (outlenX);
  result = lbu_dotsToChar (tableListX, inbufX, outbufX, outlenX, logf, mode);
  if (result)
    (*env)->SetByteArrayRegion (env, outbuf, 0, outlenX, outbufX);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufX != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufX, 0);
  if (outbufX != NULL)
    free (outbufX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    checkTable
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouisUTDML_checkTable
  (JNIEnv * env, jobject obj, jstring tableList, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  const jbyte *logf = NULL;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
    }
  result = lbu_checkTable (tableListX, logf, mode);
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    compileString
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouis_compileString
  (JNIEnv * env, jobject obj, jstring tableList, jstring newEntry,
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
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
      lou_logFile (logf);
    }
  result = lou_compileString (tableListX, newEntryX);
  if (logFile != NULL)
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
 * Class:     org_liblouis_LibLouis
 * Method:    setDataPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouis_setDataPath
  (JNIEnv * env, jclass obj, jstring path)
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
 * Class:     org.liblouis.LibLouis
 * Method:    setLogFile
 * Signature: (Ljava/lang/String)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouis_setLogFile
  (JNIEnv * env, jobject obj, jstring fileName)
{
  const char *logf = NULL;
  logf = (*env)->GetStringUTFChars(env, fileName, NULL);
  if (logf == NULL)
    return;
  lou_logFile(logf);
  (*env)->ReleaseStringUTFChars(env, fileName, logf);
}

/*
 * Class:     org.liblouis.LibLouis
 * Method:    logEnd
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouis_logEnd
  (JNIEnv * env, jobject obj)
{
  lou_logEnd();
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    charSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_liblouis_LibLouis_charSize
  (JNIEnv * env, jobject this)
{
  return CHARSIZE;
}

/*
 * Class:     org.liblouis.LibLouis
 * Method:    setLogLevel
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouis_setLogLevel
  (JNIEnv * env, jobject this, jint level)
{
  lou_setLogLevel(level);
}

static jobject louLogCBFunc;
static void javaLouLogCallback(int level, const char *message)
{
  execJavaLogCallback(louLogCBFunc, level, message);
}
/*
 * Class:     org.liblouis.LibLouis
 * Method:    registerLogCallback
 * Signature: (Lorg/liblouis/LogCallback)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouis_registerLogCallback
  (JNIEnv * env, jobject this, jobject cb)
{
  // Ensure we have a reference to the JVM
  if (jvm == NULL)
  {
    jint rs = (*env)->GetJavaVM(env, &jvm);
    if (rs != JNI_OK)
      return;
  }
  // Remove existing references to the callback
  if (louLogCBFunc != NULL)
  {
    (*env)->DeleteGlobalRef(env, louLogCBFunc);
    louLogCBFunc = NULL;
  }
  // Now set the callback according to what is passed in cb
  if (cb != NULL)
  {
    louLogCBFunc = (*env)->NewGlobalRef(env, cb);
  }
  if (louLogCBFunc != NULL)
  {
    lou_registerLogCallback(javaLouLogCallback);
  }
  else
  {
    lou_registerLogCallback(NULL);
  }
}
/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    setWriteablePath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_setWriteablePath
  (JNIEnv * env, jclass obj, jstring path)
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
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_free
  (JNIEnv * env, jobject this)
{
  lbu_free ();
  return;
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    file2brl
 * Signature: ([Ljava/lang/String;)Z
 */

/* Helper function for this method */
static const jbyte *JNICALL
getArg (JNIEnv * env, jobject obj, jobjectArray args, jint index)
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

JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouisUTDML_file2brl
  (JNIEnv * env, jobject obj, jobjectArray args)
{
  jint numArgs = (*env)->GetArrayLength (env, args);
  int mode = 0;
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
  logFileName[0] = 0;
  if (numArgs != 0)
    {
      getArg (env, obj, args, -1);
      k = 0;
      while (k < numArgs)
	{
	  curArg = getArg (env, obj, args, k);
	  if (curArg == NULL)
	    break;
	  if (curArg[0] == '-')
	    {
	      switch (curArg[1])
		{
		case 'l':
		  strcpy (logFileName, lbu_getWriteablePath ());
		  strcat (logFileName, getArg (env, obj, args, k + 1));
		  k += 2;
		  break;
		case 't':
		  mode |= htmlDoc;
		  k++;
		  break;
		case 'f':
		  strcpy (configFileList, getArg (env, obj, args, k + 1));
		  k += 2;
		  break;
		case 'b':
		case 'p':
		case 'r':
		case 'T':
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
		  strcat (configSettings, getArg (env, obj, args, k + 1));
		  k += 2;
		  strcat (configSettings, "\n");
		  break;
		case '-':
		  /* Input file is stdin but output file is not stdout. */
		  k++;
		  break;
		default:
		  logMessage (LOU_LOG_ERROR, "invalid argument '%s'", curArg);
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
		  strcpy (outputFileName, getArg (env, obj, args, k + 1));
		  k += 2;
		}
	      else
		{
		  logMessage (LOU_LOG_ERROR, "extra operand: '%s'\n",
				getArg (env, obj, args, k + 2));
		  return JNI_FALSE;
		}
	    }
	  k++;
	}
      getArg (env, obj, args, -1);
    }
  if (logFileName[0] != 0)
    lbu_logFile (logFileName);
  if (whichProc == 0)
    whichProc = 'x';
  if (configSettings != NULL)
    for (k = 0; configSettings[k]; k++)
      if (configSettings[k] == '=' && configSettings[k - 1] != ' ')
	configSettings[k] = ' ';
  if (!read_configuration_file  (configFileList, logFileName, 
configSettings, 0))
    {
      lbu_logEnd ();
      return JNI_FALSE;
    }
  if (strcmp (inputFileName, "stdin") != 0)
    {
      if (!(inputFile = fopen (inputFileName, "rb")))
	{
	  logMessage (LOU_LOG_ERROR, "Can't open input file '%s'.\n", inputFileName);
	  lbu_logEnd ();
	  return JNI_FALSE;
	}
    }
  else
    inputFile = stdin;
  /*Create somewhat edited temporary file to facilitate use of stdin. */
  strcpy (tempFileName, lbu_getWriteablePath ());
  strcat (tempFileName, "file2brl.temp");
  if (!(tempFile = fopen (tempFileName, "wb")))
    {
      logMessage (LOU_LOG_ERROR, "Can't open temporary file.\n");
      lbu_logEnd ();
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
	      whichProc = 'T';
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
	  strcpy (temp2FileName, lbu_getWriteablePath ());
	  strcat (temp2FileName, "file2brl2.temp");
	  if ((lbu_backTranslateFile
	       (configFileList, tempFileName, temp2FileName, NULL,
		NULL, mode)) != 1)
	    {
	      lbu_logEnd ();
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
      case 'p':
      case 'T':
	lbu_translateTextFile (configFileList, tempFileName, outputFileName,
			       NULL, NULL, mode);
	break;
      case 'x':
      case 't':
	lbu_translateFile (configFileList, tempFileName, outputFileName, NULL,
			   NULL, mode);
	break;
      default:
	logMessage (LOU_LOG_ERROR, "Program bug %c\n", whichProc);
	break;
      }
  if (configSettings != NULL)
    free (configSettings);
  lbu_logEnd ();
  return JNI_TRUE;
}

/* Helper for the louis translate and backtranslate functions */
jboolean JNICALL
  louisForBack
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf,
   jintArray inlen,
   jbyteArray outbuf, jintArray outlen,
   jbyteArray typeform, jintArray outputpos, jintArray inputpos,
   jintArray cursorpos, jstring logFile, jint mode, jint forBack)
{
  const jbyte *tableListX = NULL;
  jbyte *inbufY = NULL;
  jint inlenY = EMPTY;
  jbyte *outbufY = NULL;
  jint outlenY = EMPTY;
  jbyte *typeformY = NULL;
  jint *outputposY = NULL;
  jint *inputposY = NULL;
  jint cursorposY = EMPTY;
  const jbyte *logf = NULL;
  /* The mode parameter can be used directly */
  widechar *inbufX = NULL;
  jint inlenX = 0;
  widechar *outbufX = NULL;
  jint outlenX = 0;
#define typeformX typeformY
  jint *outputposX = NULL;
  jint *inputposX = NULL;
  jint cursorposX = 0;
  jint wcLength;
  jint utf8Length;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufY = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufY == NULL)
    goto release;
  (*env)->GetIntArrayRegion (env, inlen, 0, 1, &inlenY);
  if (inlenY == EMPTY)
    goto release;
  if (outbuf == NULL)
    goto release;
  (*env)->GetIntArrayRegion (env, outlen, 0, 1, &outlenY);
  if (outlenY == EMPTY)
    goto release;
  outbufY = malloc (outlenY + 4);
  outbufX = malloc ((outlenY + 4) * CHARSIZE);
  if (typeform != NULL)
    {
      typeformY = malloc (inlenY);
      (*env)->GetByteArrayRegion (env, typeform, 0, inlenY, typeformY);
      if (typeformY == NULL)
	goto release;
    }
  if (outputpos != NULL)
    {
      outputposX = malloc ((inlenY + 4) * sizeof (int));
    }
  if (inputpos != NULL)
    {
      inputposX = malloc ((outlenY + 4) * sizeof (int));
    }
  if (cursorpos != NULL)
    {
      (*env)->GetIntArrayRegion (env, cursorpos, 0, 1, &cursorposY);
      if (cursorposY == EMPTY)
	goto release;
    }
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
      lou_logFile (logf);
    }
  utf8Length = inlenY;
  wcLength = inlenY;
  utf8_string_to_wc (inbufY, &utf8Length, inbufX, &wcLength);
  if (forBack == 0)
    result = lou_translate (tableListX, inbufX, &wcLength, outbufX,
			    &outlenX, typeformX, NULL,
			    outputposX, inputposX, &cursorposX, mode);
  else
    result = lou_backTranslate (tableListX, inbufX, &wcLength,
				outbufX,
				&outlenX, typeformX, NULL,
				outputposX, inputposX, &cursorposX, mode);
  if (result)
    {
      wcLength = outlenX;
      utf8Length = outlenY;
      wc_string_to_utf8 (outbufX, &wcLength, outbufY, &utf8Length);
      (*env)->SetByteArrayRegion (env, outbuf, 0, utf8Length, outbufY);
      if (typeformX != NULL)
	(*env)->SetByteArrayRegion (env, typeform, 0, inlenY, typeformX);
      if (outputpos != NULL)
	(*env)->SetIntArrayRegion (env, outputpos, 0, inlenY, outputposX);
      if (inputpos != NULL)
	(*env)->SetIntArrayRegion (env, inputpos, 0, outlenX, inputposX);
      if (cursorpos != NULL)
	(*env)->SetIntArrayRegion (env, cursorpos, 0, 1, &cursorposX);
    }
  if (logf != NULL)
    lou_logEnd ();
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufY != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufY, 0);
  if (typeformX != NULL)
    free (typeformX);
  if (outputposX != NULL)
    free (outputposX);
  if (inputposX != NULL)
    free (inputposX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    translateString
 * Signature: (Ljava/lang/String;[B[I[B[I[BLjava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_LibLouis_translateString (JNIEnv * env,
						      jobject obj,
						      jstring tableList,
						      jbyteArray inbuf,
						      jintArray inlen,
						      jbyteArray outbuf,
						      jintArray outlen,
						      jbyteArray typeform,
						      jstring logFile,
						      jint mode)
{
  return louisForBack (env, obj, tableList, inbuf, inlen, outbuf, outlen,
		       typeform, NULL, NULL, NULL, logFile, mode, 0);
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    translate
 * Signature: (Ljava/lang/String;[B[I[B[I[B[I[I[ILjava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouis_translate
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf,
   jintArray inlen,
   jbyteArray outbuf, jintArray outlen,
   jbyteArray typeform, jintArray outputpos, jintArray inputpos,
   jintArray cursorpos, jstring logFile, jint mode)
{
  return louisForBack (env, obj, tableList, inbuf, inlen, outbuf, outlen,
		       typeform,
		       outputpos, inputpos, cursorpos, logFile, mode, 0);
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    hyphenate
 * Signature: (Ljava/lang/String;[BI[BLjava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouis_hyphenate
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf, jint
   inlen, jbyteArray hyphens, jstring logFile, jint mode)
{
  const jbyte *tableListX = NULL;
  widechar *inbufX = NULL;
  jbyte *inbufY = NULL;
  jint inlenX = 0;
  const jbyte *logf = NULL;
  jbyte hyphensX[128];
  jint wcLength = 0;
  jint utf8Length = 0;
  jboolean result = JNI_FALSE;
  tableListX = (*env)->GetStringUTFChars (env, tableList, NULL);
  if (tableListX == NULL)
    goto release;
  inbufY = (*env)->GetByteArrayElements (env, inbuf, NULL);
  if (inbufY == NULL)
    goto release;
  if (hyphens == NULL)
    goto release;
  inlen = (*env)->GetArrayLength (env, inbuf);
  if (logFile != NULL)
    {
      logf = (*env)->GetStringUTFChars (env, logFile, NULL);
      if (logf == NULL)
	goto release;
      lou_logFile (logf);
    }
  inbufX = malloc (inlenX);
  wcLength = inlenX;
  utf8Length = inlenX;
  utf8_string_to_wc (inbufY, &utf8Length, inbufX, &wcLength);
  result = lou_hyphenate (tableListX, inbufX, inlenX, hyphensX, mode);
  if (result)
    (*env)->SetByteArrayRegion (env, hyphens, 0, strlen (hyphensX), hyphensX);
  if (logFile != NULL)
    lou_logEnd ();
release:
  if (tableListX != NULL)
    (*env)->ReleaseStringUTFChars (env, tableList, tableListX);
  if (inbufY != NULL)
    (*env)->ReleaseByteArrayElements (env, inbuf, inbufY, 0);
  if (inbufX != NULL)
    free (inbufX);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  return result;
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    backTranslateString
 * Signature: (Ljava/lang/String;[B[I[B[I[BLjava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
  Java_org_liblouis_LibLouis_backTranslateString
  (JNIEnv * env, jobject obj,
   jstring tableList,
   jbyteArray
   inbuf,
   jintArray
   inlen,
   jbyteArray
   outbuf, jintArray outlen, jbyteArray typeform, jstring logFile, jint mode)
{
  return louisForBack (env, obj, tableList, inbuf, inlen, outbuf, outlen,
		       typeform, NULL, NULL, NULL, logFile, mode, 1);
}

/*
 * Class:     org_liblouis_LibLouis
 * Method:    backTranslate
 * Signature: (Ljava/lang/String;[B[I[B[I[B[I[I[ILjava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_LibLouis_backTranslate
  (JNIEnv * env, jobject obj, jstring tableList, jbyteArray inbuf,
   jintArray inlen,
   jbyteArray outbuf, jintArray outlen,
   jbyteArray typeform, jintArray outputpos, jintArray inputpos,
   jintArray cursorpos, jstring logFile, jint mode)
{
  return louisForBack (env, obj, tableList, inbuf, inlen, outbuf, outlen,
		       typeform,
		       outputpos, inputpos, cursorpos, logFile, mode, 1);
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    setLogFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_setLogFile
  (JNIEnv * env, jobject obj, jstring logFile)
{
  const char *logf = NULL;
  logf = (*env)->GetStringUTFChars (env, logFile, NULL);
  if (logf == NULL)
    return;
  lbu_logFile (logf);
  (*env)->ReleaseStringUTFChars (env, logFile, logf);
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    logEnd
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_logEnd
  (JNIEnv * env, jobject this)
{
  lbu_logEnd ();
}

static jobject lbuLogCBFunc;
static void javaLbuLogCallbackFunc(int level, const char *message)
{
  execJavaLogCallback(lbuLogCBFunc, level, message);
}

/*
 * Class:     org_liblouis_LibLouisUTDML
 * Method:    registerLogCallback
 * Signature: (Lorg/liblouis/LogCallback;)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_registerLogCallback
  (JNIEnv * env, jobject this, jobject cb)
{
  // if not previously set, set the JVM pointer
  if (jvm == NULL)
  {
    jint rs = (*env)->GetJavaVM(env, &jvm);
    if (rs != JNI_OK)
    {
      return;
    }
  }
  // Remove any existing global reference to callbacks
  if (lbuLogCBFunc != NULL)
  {
    (*env)->DeleteGlobalRef(env, lbuLogCBFunc);
    lbuLogCBFunc = NULL;
  }
  if (cb != NULL)
  {
    lbuLogCBFunc = (*env)->NewGlobalRef(env, cb);
  }
  if (lbuLogCBFunc != NULL)
  {
    lbu_registerLogCallback(javaLbuLogCallbackFunc);
  }
  else
  {
    lbu_registerLogCallback(NULL);
  }
}

/*
 * Class:    org_liblouis_LibLouisUTDML
 * Method:   setLogLevel
 *Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_LibLouisUTDML_setLogLevel
  (JNIEnv * env, jobject this, jint level)
{
  lbu_setLogLevel(level);
}
