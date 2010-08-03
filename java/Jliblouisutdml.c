#include "Jliblouisutdml.h"

jstring
Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1version (JNIEnv * env,
								jobject obj)
{
}

jboolean
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1translateString
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jbyteArray inbuf,
   jbyteArray outbuf,
   jintArray outlen, jstring logFileName, jstring settingsString, jint mode)
{
}

jboolean
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1backTranslateString
  (JNIEnv * env,
     jobject obj,
     jstring configFileList,
     jbyteArray inbuf,
     jbyteArray outbuf,
     jintArray outlen, jstring logFileName, jstring settingsString, jint mode)
{
}

jboolean
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1translateFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputFileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
{
}

jboolean
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1translateTextFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputFileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
{
}

jboolean
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1backTranslateFile
  (JNIEnv * env,
   jobject obj,
   jstring configFileList,
   jstring inputfileName,
   jstring outputFileName,
   jstring logFileName, jstring settingsString, jint mode)
{
}

void
  Java_org_brailleblaster_JliblouisX_Jliblouisutdml_lbu_1free
  (JNIEnv * env, jobject obj)
{
}
