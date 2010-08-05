#include "Jliblouisutdml.h"

jstring
Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1version (JNIEnv * env,
								jobject obj)
{
return (*env)->NewStringUTF (env, lbu_getVersion ());
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
     const jbyte *cfl;
const jbyte *logf;
const jbyte *settings;
     cfl = (*env)->GetStringUTFChars(env, configFileList, NULL);
     if (cfl == NULL)
         return NULL;
     logf = (*env)->GetStringUTFChars(env, logFileName, NULL);
     if (logf == NULL)
         return NULL;
     settings = (*env)->GetStringUTFChars(env, settingsString, NULL);
     if (settings == NULL)
         return NULL;

     (*env)->ReleaseStringUTFChars(env, configFileList, cfl);
     (*env)->ReleaseStringUTFChars(env, logFileName, logf);
     (*env)->ReleaseStringUTFChars(env, settingsString, settings);
return;
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
     const jbyte *cfl;
const jbyte *logf;
const jbyte *settings;
     cfl = (*env)->GetStringUTFChars(env, configFileList, NULL);
     if (cfl == NULL)
         return NULL;
     logf = (*env)->GetStringUTFChars(env, logFileName, NULL);
     if (logf == NULL)
         return NULL;
     settings = (*env)->GetStringUTFChars(env, settingsString, NULL);
     if (settings == NULL)
         return NULL;

     (*env)->ReleaseStringUTFChars(env, configFileList, cfl);
     (*env)->ReleaseStringUTFChars(env, logFileName, logf);
     (*env)->ReleaseStringUTFChars(env, settingsString, settings);
return;
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
     const jbyte *cfl;
const jbyte *logf;
const jbyte *settings;
     cfl = (*env)->GetStringUTFChars(env, configFileList, NULL);
     if (cfl == NULL)
         return NULL;
     logf = (*env)->GetStringUTFChars(env, logFileName, NULL);
     if (logf == NULL)
         return NULL;
     settings = (*env)->GetStringUTFChars(env, settingsString, NULL);
     if (settings == NULL)
         return NULL;

     (*env)->ReleaseStringUTFChars(env, configFileList, cfl);
     (*env)->ReleaseStringUTFChars(env, logFileName, logf);
     (*env)->ReleaseStringUTFChars(env, settingsString, settings);
returnj;
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
     const jbyte *cfl;
const jbyte *logf;
const jbyte *settings;
     cfl = (*env)->GetStringUTFChars(env, configFileList, NULL);
     if (cfl == NULL)
         return NULL;
     logf = (*env)->GetStringUTFChars(env, logFileName, NULL);
     if (logf == NULL)
         return NULL;
     settings = (*env)->GetStringUTFChars(env, settingsString, NULL);
     if (settings == NULL)
         return NULL;

     (*env)->ReleaseStringUTFChars(env, configFileList, cfl);
     (*env)->ReleaseStringUTFChars(env, logFileName, logf);
     (*env)->ReleaseStringUTFChars(env, settingsString, settings);
returnj;
}

jboolean
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1backTranslateFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputfileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
{
     const jbyte *cfl;
const jbyte *logf;
const jbyte *settings;
     cfl = (*env)->GetStringUTFChars(env, configFileList, NULL);
     if (cfl == NULL)
         return NULL;
     logf = (*env)->GetStringUTFChars(env, logFileName, NULL);
     if (logf == NULL)
         return NULL;
     settings = (*env)->GetStringUTFChars(env, settingsString, NULL);
     if (settings == NULL)
         return NULL;

     (*env)->ReleaseStringUTFChars(env, configFileList, cfl);
     (*env)->ReleaseStringUTFChars(env, logFileName, logf);
     (*env)->ReleaseStringUTFChars(env, settingsString, settings);
return;
}

void
  Java_org_brailleblaster_jlouislibs_Jliblouisutdml_lbu_1free
  (JNIEnv * env, jobject obj)
{
lbu_free ();
return;
}
