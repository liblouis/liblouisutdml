#include "Jliblouisutdml.h"
#include <liblouisutdml.h>
#include <stdlib.h>

jstring
Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1version (JNIEnv * env,
								jobject obj)
{
  return (*env)->NewStringUTF (env, lbu_version ());
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1translateString
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jbyteArray inbuf,
   jbyteArray outbuf,
   jintArray outlen, jstring logFileName, jstring settingsString, jint mode)
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
  logf = (*env)->GetStringUTFChars (env, logFileName, NULL);
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
    (*env)->ReleaseStringUTFChars (env, logFileName, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1backTranslateString
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jbyteArray inbuf,
   jbyteArray outbuf,
   jintArray outlen, jstring logFileName, jstring settingsString, jint mode)
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
  logf = (*env)->GetStringUTFChars (env, logFileName, NULL);
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
    (*env)->ReleaseStringUTFChars (env, logFileName, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1translateFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputFileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
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
  logf = (*env)->GetStringUTFChars (env, logFileName, NULL);
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
    (*env)->ReleaseStringUTFChars (env, logFileName, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1translateTextFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputFileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
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
  logf = (*env)->GetStringUTFChars (env, logFileName, NULL);
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
    (*env)->ReleaseStringUTFChars (env, logFileName, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1backTranslateFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputFileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
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
  logf = (*env)->GetStringUTFChars (env, logFileName, NULL);
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
    (*env)->ReleaseStringUTFChars (env, logFileName, logf);
  if (settings != NULL)
    (*env)->ReleaseStringUTFChars (env, settingsString, settings);
  return result;
}

void
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1free
  (JNIEnv * env, jobject obj)
{
  lbu_free ();
  return;
}
