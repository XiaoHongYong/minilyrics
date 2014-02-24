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
#include "../../MLLib/MLLib.h"

/************************************************************************

Native library of MiniLyrics provide features:
 * BinaryXML handle
 * Search lyrics locally
 * Associate lyrics
 * Open lyrics[File & embedded]


************************************************************************/

class CJStr
{
public:
	CJStr(JNIEnv *env, jstring jstr) {
		this->jstr = jstr;
		this->env = env;
		this->utf8Str = env->GetStringUTFChars(jstr, NULL);
	}

	~CJStr() {
		env->ReleaseStringUTFChars(jstr, utf8Str);
	}

	LPCSTR c_str() {
		return utf8Str;
	}

protected:
	jstring		jstr;
	LPCSTR		utf8Str;
	JNIEnv		*env;

};

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	jvm_init(env);

	// Get jclass with env->FindClass.
	// Register methods with env->RegisterNatives.

	return JNI_VERSION_1_6;
}

extern "C" jstring Java_com_example_hellojni_HelloJni_init(JNIEnv* env, jobject thiz, jstring jstrWorkingFolder, jstring jstrAppName, jstring jstrProfile)
{
	CJStr strWorkingFolder(env, jstrWorkingFolder);
	CJStr strAppName(env, jstrAppName);
	CJStr strProfile(env, jstrProfile);

	InitBaseFrameWork(strWorkingFolder.c_str(), strAppName.c_str(), strProfile.c_str(), strAppName.c_str());

	return env->NewStringUTF("1init");
}

LPCTSTR _TL(LPCTSTR szString)
{
	return szString;
}

// Remove '&'
tstring RemovePrefixOfAcckey(LPCTSTR szStr)
{
	LPCTSTR		p;

	p = _tcschr(szStr, '&');
	if (!p)
		return szStr;

	tstring		str;

	// Remove "(&A)"
	if (p > szStr && *(p - 1) == '(' && p[1] != '\0' && p[2] == ')' && p[3] == '\0')
	{
		str.append(szStr, (int)(p - szStr) - 1);
	}
	else
	{
		str.append(szStr, (int)(p - szStr));
		str.append(p + 1);
	}

	return str;
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
}

extern "C" jstring Java_com_crintsoft_music_lyrics_LyricsData_readLyricsSource(JNIEnv* env,
                                                  jobject thiz, jstring mediaFile, jstring lyricsSrc)
{
	CMLData		lyricsData;

	LPCSTR szMediaFile = env->GetStringUTFChars(mediaFile, NULL);
	LPCSTR szLyricsSrc = env->GetStringUTFChars(lyricsSrc, NULL);

	int n = lyricsData.OpenLyrics(szMediaFile, 0, szLyricsSrc);
	if (n != ERR_OK) {
		return env->NewStringUTF(Error2Str(n).c_str());;
	}

	tstring str;
	lyricsData.ToString(str, FT_LYRICS_LRC, true);

	env->ReleaseStringUTFChars(mediaFile, szMediaFile);
	env->ReleaseStringUTFChars(lyricsSrc, szLyricsSrc);

	return env->NewStringUTF(str.c_str());
}
