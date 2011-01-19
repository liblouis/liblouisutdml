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
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_liblouis_Jliblouisutdml_version
  (JNIEnv * env, jobject this)
{
  return (*env)->NewStringUTF (env, lbu_version ());
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    translateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_translateString (JNIEnv * env,
						  jobject this,
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
  jbyte *inbufx = NULL;
  jint inlen = 0;
  jbyte *outbufx = NULL;
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
  outbufx = (*env)->GetByteArrayElements (env, outbuf, NULL);
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
    (*env)->ReleaseByteArrayElements (env, outbufx, outbuf, 0);
  if (outlenx != NULL)
    (*env)->ReleaseIntArrayElements (env, outlenx, outlen, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    backTranslateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_backTranslateString (JNIEnv * env,
						      jobject this,
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
  jbyte *inbufx = NULL;
  jint inlen = 0;
  jbyte *outbufx = NULL;
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
  outbufx = (*env)->GetByteArrayElements (env, outbuf, NULL);
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
    (*env)->ReleaseByteArrayElements (env, outbufx, outbuf, 0);
  if (outlenx != NULL)
    (*env)->ReleaseIntArrayElements (env, outlenx, outlen, 0);
  if (logf != NULL)
    (*env)->ReleaseStringUTFChars (env, logFile, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    translateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_translateFile
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
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    translateTextFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_translateTextFile (JNIEnv * env,
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
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    backTranslateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_backTranslateFile (JNIEnv * env,
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
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    charToDots
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_charToDots
  (JNIEnv * env, jobject this, jstring trantab, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    dotsToChar
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_dotsToChar
  (JNIEnv * env, jobject this, jstring trantab, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    checkTable
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_checkTable
  (JNIEnv * env, jobject this, jstring trantab, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    charSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_liblouis_Jliblouisutdml_charSize
  (JNIEnv * env, jobject this)
{
  return lou_charSize ();
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_free
  (JNIEnv * env, jobject this)
{
  lbu_free ();
  return;
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    file2brl
 * Signature: ([Ljava/lang/String;)Z
 */

/* Helper function for this method */
JNIEXPORT char *JNICALL
getArg (JNIEnv * env, jobject this, jobjectArray args, jint index)
{
//    curArg = (*env)->GetObjectArrayElement(env, args, k);

}

JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_file2brl
  (JNIEnv * env, jobject this, jobjectArray args)
{
  jint numArgs = (*env)->GetArrayLength (env, args);
  int mode = dontInit;
  char *configFileName = "default.cfg";
  char *inputFileName = "stdin";
  char *outputFileName = "stdout";
  char tempFileName[MAXNAMELEN];
  char logFileName = "file2brl.log";
  char whichProc = 0;
  char *configSettings = NULL;
  FILE *inputFile = NULL;
  FILE *tempFile;
  int ch = 0;
  int pch = 0;
  int nch = 0;
  int charsRead = 0;
  int k;
  char *curArg = NULL;
  UserData *ud;
  k = 0;
  while (k < numArgs)
    {
      curarg = getArg (env, this, args, k);
      if (curArg[0] == '-')
	switch (curArg[1])
	  {
	  case 'l':
	    strcpy (logFileName, getArg (env, this, args k = 1));
	    k += 2;
	    break;
	  case 't':
	    mode |= htmlDoc;
	    break;
	  case 'f':
	    strcpy (configFileName, getArg (env, this, args, k + 1));
	    k += 2;
	    break;
	  case 'b':
	  case 'p':
	  case 'r':
	  case 'x':
	    whichProc = curArg[1];
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
	  default:
	    return JNI_FALSE;
	    break;
	  }
      if (k < numArgs)
	{
	  if (k == numArgs - 1)
	    {
	      strcpy (inputFileName, getArg (env, this, args, k));
	    }
	  else if (k == numArgs - 2)
	    {
	      if (strcmp (curArg, "-") != 0)
		strcpy (inputFileName, curArg);
	      strcpy (outputFileName, getArg (env, this, args, k + 1));
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

  if (whichProc == 0)
    whichProc = 'x';
  if (configSettings != NULL)
    for (k = 0; configSettings[k]; k++)
      if (configSettings[k] == '=' && configSettings[k - 1] != ' ')
	configSettings[k] = ' ';
  if ((ud =
       lbu_initialize (configFileName, logFileName, configSettings)) == NULL)
    return JNI - fALSE;
  if (strcmp (inputFileName, "stdin") != 0)
    {
      if (!(inputFile = fopen (inputFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", inputFileName);
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
	lbu_backTranslateFile (configFileName, tempFileName, outputFileName,
			       NULL, NULL, mode);
	break;
      case 'r':
	{
	  char temp2FileName[MAXNAMELEN];
	  strcpy (temp2FileName, ud->writeable_path);
	  strcat (temp2FileName, "file2brl2.temp");
	  if ((lbu_backTranslateFile
	       (configFileName, tempFileName, temp2FileName, NULL,
		NULL, mode)) != 1)
	    return JNI_FALSE;
	  if (ud->back_text == html)
	    lbu_translateFile (configFileName,
			       temp2FileName,
			       outputFileName, NULL, NULL, mode);
	  else
	    lbu_translateTextFile (configFileName, temp2FileName,
				   outputFileName, NULL, NULL, mode);
	}
	break;
      case 't':
      case 'p':
	lbu_translateTextFile (configFileName, tempFileName, outputFileName,
			       NULL, NULL, mode);
	break;
      case 'x':
	lbu_translateFile (configFileName, tempFileName, outputFileName, NULL,
			   NULL, mode);
	break;
      default:
	lou_logPrint ("Program bug %c\n", whichProc);
	break;
      }
  lbu_free ();
  if (configSettings != NULL)
    free (configSettings);
  return JNI_TRUE;
}
