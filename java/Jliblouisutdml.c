#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Jliblouisutdml.h"
#include <louisutdml.h>

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_liblouis_Jliblouisutdml_version
  (JNIEnv * env, jobject obj)
{
  return (*env)->NewStringUTF (env, lbu_version ());
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_translateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_translateString (JNIEnv * env,
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
 * Method:    lbu_backTranslateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_backTranslateString (JNIEnv * env,
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
 * Method:    lbu_translateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_translateFile
  (JNIEnv * env, jobject obj, jstring configFileList, jstring inFile,
   jstring outFile, jstring logFile, jstring settingsString, jint mode)
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
 * Method:    lbu_translateTextFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_translateTextFile (JNIEnv * env,
							 jobject obj,
							 jstring
							 configFileList,
							 jstring inFile,
							 jstring
							 outFile,
							 jstring
							 logFile,
							 jstring
							 settingsString,
							 jint mode)
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
 * Method:    lbu_backTranslateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_liblouis_Jliblouisutdml_backTranslateFile (JNIEnv * env,
							 jobject obj,
							 jstring
							 configFileList,
							 jstring inFile,
							 jstring
							 outFile,
							 jstring
							 logFile,
							 jstring
							 SettingsString,
							 jint mode)
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
 * Method:    lbu_charToDots
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_charToDots
  (JNIEnv * env, jobject obj, jstring trantab, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_dotsToChar
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_dotsToChar
  (JNIEnv * env, jobject obj, jstring trantab, jbyteArray inbuf,
   jbyteArray outbuf, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_checkTable
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_checkTable
  (JNIEnv * env, jobject obj, jstring trantab, jstring logFile, jint mode)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_charSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_liblouis_Jliblouisutdml_charSize
  (JNIEnv * env, jobject)
{
  return lou_charsize ();
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_free
  (JNIEnv * env, jobject)
{
  lbu_free ();
  return;
}
