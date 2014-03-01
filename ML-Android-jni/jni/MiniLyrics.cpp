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
#include "../../MLProtocol/MLProtocol.h"
#include "LyricsLocalSearch.h"
#include "MLProtocolCmd.h"


CMLProtocolCmd		g_mlProtocolCmd;

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

inline void WriteAttributeIfNoNull(CXMLWriter &writer, LPCTSTR name, tstring &val)
{
	if (val.size() > 0) {
		writer.WriteAttribute(name, val.c_str());
	}
}

void ToString(V_LRCSEARCHRESULT &vResult, CXMLWriter &writer)
{
	for (V_LRCSEARCHRESULT::iterator it = vResult.begin(); it != vResult.end(); ++it) {
		LrcSearchResult &item = *it;

		writer.WriteStartElement("item");

		WriteAttributeIfNoNull(writer, "url", item.strUrl);
		WriteAttributeIfNoNull(writer, "save_file_name", item.strSaveFileName);
		WriteAttributeIfNoNull(writer, "artist", item.strArtist);
		WriteAttributeIfNoNull(writer, "title", item.strTitle);
		WriteAttributeIfNoNull(writer, "album", item.strAlbum);
		WriteAttributeIfNoNull(writer, "uploader", item.strUploader);
		if (item.nMatchValue > 0)
			writer.WriteAttribute("match_value", item.nMatchValue);
		if (item.fRate > 0)
			writer.WriteAttribute("rate", item.fRate);
		if (item.nRateCount > 0)
			writer.WriteAttribute("rate_count", item.nRateCount);
		if (item.nDownloads > 0)
			writer.WriteAttribute("downloads", item.nDownloads);

		writer.WriteEndElement();
	}
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
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

#define NativeInit Java_com_crintsoft_music_n_a_b
#define encodeOnlineSearchRequest Java_com_crintsoft_music_n_a_c
#define decodeOnlineSearchReesult Java_com_crintsoft_music_n_a_d
#define searchAllLocalLyrics Java_com_crintsoft_music_n_a_e
#define searchBestMatchLyrics Java_com_crintsoft_music_n_a_f
#define setLocalSearchOptions Java_com_crintsoft_music_n_a_g
#define readLyricsSource Java_com_crintsoft_music_n_a_h

extern "C" JNIEXPORT jstring NativeInit(JNIEnv* env, jobject thiz, jobject ctx, jstring jstrWorkingFolder, jstring jstrAppName, jstring jstrProfile, jstring jstrClientName)
{
	// TODO: Get package name & signature.

	CJStr strWorkingFolder(env, jstrWorkingFolder);
	CJStr strAppName(env, jstrAppName);
	CJStr strProfile(env, jstrProfile);
	CJStr strClientName(env, jstrClientName);

	InitBaseFrameWork(strWorkingFolder.c_str(), strAppName.c_str(), strProfile.c_str(), strAppName.c_str());

	g_mlProtocolCmd.Init(strClientName.c_str());

	return env->NewStringUTF("1init");
}

extern "C" JNIEXPORT jstring searchAllLocalLyrics(JNIEnv* env, jobject thiz, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
{
	CJStr strFile(env, jstrFile);
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	V_LRCSEARCHRESULT result;

	g_lyricsSearch.SearchAllMatchLyrics(strFile.c_str(), strArtist.c_str(), strTitle.c_str(), result);

	CXMLWriter writer;
	writer.WriteStartElement("result");
	ToString(result, writer);
	writer.WriteEndElement();

	return env->NewStringUTF(writer.GetBuffer().c_str());
}

extern "C" JNIEXPORT jstring searchBestMatchLyrics(JNIEnv* env, jobject thiz, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
{
	CJStr strFile(env, jstrFile);
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	tstring lyricsFile;
	if (g_lyricsSearch.GetBestMatchLyrics(strFile.c_str(), strArtist.c_str(), strTitle.c_str(), lyricsFile))
		return env->NewStringUTF(lyricsFile.c_str());

	return NULL;
}

extern "C" JNIEXPORT void setLocalSearchOptions(JNIEnv* env, jobject thiz, jstring jstrDownloadDir, jstring jstrLyricsDir)
{
	CJStr strDownloadDir(env, jstrDownloadDir);
	CJStr strLyricsDir(env, jstrLyricsDir);

	ERRLOG2("Settings: %s, %s", strLyricsDir.c_str(), strDownloadDir.c_str());

	g_lyricsSearch.m_lyricsDirOfDownload = strDownloadDir.c_str();
	g_lyricsSearch.m_lyricsDirOfUser = strLyricsDir.c_str();
}

extern "C" JNIEXPORT jstring readLyricsSource(JNIEnv* env, jobject thiz, jstring mediaFile, jstring lyricsSrc)
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

extern "C" JNIEXPORT jbyteArray encodeOnlineSearchRequest(JNIEnv* env, jobject thiz, jstring jstrArtist, jstring jstrTitle, jboolean onlyMatched)
{
	ERRLOG0("Enter: encodeSearchRequest");

	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	CMLBinXMLWriter xmlStream;
	g_mlProtocolCmd.ToSearchCommand(strArtist.c_str(), strTitle.c_str(), 0, onlyMatched, xmlStream);

	CCharBuffer &buf = xmlStream.GetBuffer();
	jbyteArray jbyteArr = env->NewByteArray(buf.size());
	if (jbyteArr == NULL)
		return NULL;

	env->SetByteArrayRegion(jbyteArr, 0, buf.size(), (const jbyte *)buf.c_str());

	return jbyteArr;
}

extern "C" JNIEXPORT jstring decodeOnlineSearchReesult(JNIEnv* env, jobject thiz, jbyteArray data, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
{
	CCharBuffer buf;
	buf.resize(env->GetArrayLength(data));
	env->GetByteArrayRegion(data, 0, buf.size(), (jbyte *)buf.data());

	CJStr strFile(env, jstrFile);
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	CXMLWriter writer;
	g_mlProtocolCmd.DecodeSearchRetCommand(strFile.c_str(), strArtist.c_str(), strTitle.c_str(), buf, writer);

	return env->NewStringUTF(writer.GetBuffer().c_str());
}
