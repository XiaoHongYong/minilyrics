/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <jni.h>
#include "../../base/android/android.h"
#include "../../base/base.h"
#include "../../Utils/Utils.h"

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	jvm_init(env);

	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	// Get jclass with env->FindClass.
	// Register methods with env->RegisterNatives.

	return JNI_VERSION_1_6;
}

extern "C" jstring Java_com_example_hellojni_HelloJni_init(JNIEnv* env, jobject thiz)
{

	return env->NewStringUTF("1init");
}

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
extern "C" jstring Java_com_example_hellojni_HelloJni_stringFromJNI(JNIEnv* env,
                                                  jobject thiz, jstring inStr)
{
	const char *utf8 = env->GetStringUTFChars(inStr, NULL);

	CHAR_ENCODING encoding = ED_GB2312;
	//CHAR_ENCODING encoding = ED_UTF8;

	tstring strOut;
	UTF8ToTString(utf8, -1, strOut, encoding);

	DBGLOG0("Log test");
	ERRLOG0("Log test");
	ERRLOG0(strOut.c_str());

	string strOutUtf8;
	TStringToUTF8(strOut.c_str(), strOut.size(), strOutUtf8, encoding);
	ERRLOG0(strOutUtf8.c_str());

	return env->NewStringUTF((const char *)strOutUtf8.c_str());
/*	g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

	jclass clsString = env->FindClass("java/lang/String");
	if (clsString == NULL)
		return env->NewStringUTF("p1");

	jobject objString = env->AllocObject(clsString);
	if (objString == NULL)
		return env->NewStringUTF("p2");

	jmethodID mtdString = env->GetMethodID(clsString, "<init>", "([BLjava/lang/String;)V");
	if (mtdString == NULL)
		return env->NewStringUTF("p3");

	const char *str = "This is a test";
//	strcpy(str, "This is a test");

	jbyteArray buf = env->NewByteArray(100);
	if (buf == NULL)
		return env->NewStringUTF("p4");

	env->SetByteArrayRegion(buf, 0, strlen(str), (const jbyte *)str);

	jobject encoding = env->NewStringUTF("utf-8");
	if (encoding == NULL)
		return env->NewStringUTF("p5");

	env->CallVoidMethod(objString, mtdString, buf, encoding);

	env->DeleteLocalRef(buf);

	return (jstring)objString;;

#if defined(__arm__)
  #if defined(__ARM_ARCH_7A__)
    #if defined(__ARM_NEON__)
      #define ABI "armeabi-v7a/NEON"
    #else
      #define ABI "armeabi-v7a"
    #endif
  #else
   #define ABI "armeabi"
  #endif
#elif defined(__i386__)
   #define ABI "x86"
#elif defined(__mips__)
   #define ABI "mips"
#else
   #define ABI "unknown"
#endif

    return env->NewStringUTF("Hello from JNI !  Compiled with ABI " ABI ".");*/
}
