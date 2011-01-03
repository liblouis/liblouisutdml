#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Jliblouisutdml.h>
#include <liblouisutdml.h>

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1version
  (JNIEnv *, jobject)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_translateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
  Java_org_liblouis_Jliblouisutdml_lbu_1translateString (JNIEnv *, jobject,
							 jstring, jbyteArray,
							 jbyteArray,
							 jintArray, jstring,
							 jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_backTranslateString
 * Signature: (Ljava/lang/String;[B[B[ILjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
  Java_org_liblouis_Jliblouisutdml_lbu_1backTranslateString (JNIEnv *,
							     jobject, jstring,
							     jbyteArray,
							     jbyteArray,
							     jintArray,
							     jstring, jstring,
							     jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_translateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1translateFile
  (JNIEnv *, jobject, jstring, jstring, jstring, jstring, jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_translateTextFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
  Java_org_liblouis_Jliblouisutdml_lbu_1translateTextFile (JNIEnv *, jobject,
							   jstring, jstring,
							   jstring, jstring,
							   jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_backTranslateFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
  Java_org_liblouis_Jliblouisutdml_lbu_1backTranslateFile (JNIEnv *, jobject,
							   jstring, jstring,
							   jstring, jstring,
							   jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_charToDots
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1charToDots
  (JNIEnv *, jobject, jstring, jbyteArray, jbyteArray, jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_dotsToChar
 * Signature: (Ljava/lang/String;[B[BLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1dotsToChar
  (JNIEnv *, jobject, jstring, jbyteArray, jbyteArray, jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_checkTable
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1checkTable
  (JNIEnv *, jobject, jstring, jstring, jint)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_charSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1charSize
  (JNIEnv *, jobject)
{
}

/*
 * Class:     org_liblouis_Jliblouisutdml
 * Method:    lbu_free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_liblouis_Jliblouisutdml_lbu_1free
  (JNIEnv *, jobject)
{
}
