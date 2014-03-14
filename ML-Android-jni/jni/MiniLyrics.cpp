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
bool				g_isSignatureCorrect = false;;


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

		MLFileType fileType = MLGetFileType(item.strUrl.c_str());
		LRC_SOURCE_TYPE lrcSrcType = LyrSrcTypeFromName(item.strUrl.c_str());
		if (lrcSrcType == LST_ID3V2_SYLT || lrcSrcType == LST_ID3V2_LYRICS || lrcSrcType == LST_LYRICS3V2 || lrcSrcType == LST_KAR_LYRICS)
			fileType = FT_LYRICS_LRC;
		else if (lrcSrcType == LST_ID3V2_USLT || lrcSrcType == LST_M4A_LYRICS)
			fileType = FT_LYRICS_TXT;
		writer.WriteAttribute("file_type", (int)fileType);

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
#define setDefaultCharEncoding Java_com_crintsoft_music_n_a_i



jbyteArray GetSignature(JNIEnv* env, jobject ctx)
{
	jclass Context = env->FindClass("android/content/Context");

	// Get Package name
	jmethodID midGetPackageName = env->GetMethodID(Context, "getPackageName", "()Ljava/lang/String;");
	jstring pkgName = reinterpret_cast<jstring>(env->CallObjectMethod(ctx, midGetPackageName));

	// Get Package Manager
	jmethodID midGetPackageManager = env->GetMethodID(Context, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jobject pkgMgr = env->CallObjectMethod(ctx, midGetPackageManager);

	// Get Package info
	jclass PackageManager = env->GetObjectClass(pkgMgr);
	jmethodID midGetPackageInfo = env->GetMethodID(PackageManager, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jobject pkgInfo = env->CallObjectMethod(pkgMgr, midGetPackageInfo, pkgName, 64);
	if (pkgInfo == NULL)
		return NULL;

	// Get Signature
	jclass PackageInfo = env->GetObjectClass(pkgInfo);
	jfieldID fidSignatures = env->GetFieldID(PackageInfo, "signatures", "[Landroid/content/pm/Signature;");
	jobjectArray signatures =  reinterpret_cast<jobjectArray>(env->GetObjectField(pkgInfo, fidSignatures));
	if (signatures == NULL)
		return NULL;

	// Get Signature byte array.
	jobject signature = env->GetObjectArrayElement(signatures, 0);
	jclass Signature = env->GetObjectClass(signature);
	jmethodID midToByteArray = env->GetMethodID(Signature, "toByteArray", "()[B");
	jobject sigData = env->CallObjectMethod(signature, midToByteArray);

	env->DeleteLocalRef(pkgName);
	env->DeleteLocalRef(pkgMgr);
	env->DeleteLocalRef(pkgInfo);
	env->DeleteLocalRef(signatures);
	env->DeleteLocalRef(signature);

	return reinterpret_cast<jbyteArray>(sigData);
}

void CheckSignature(JNIEnv* env, jobject ctx)
{
	g_isSignatureCorrect = false;

	jbyteArray jsigData = GetSignature(env, ctx);
	if (jsigData == NULL)
		return;

	int length = env->GetArrayLength(jsigData);
	jbyte *data = env->GetByteArrayElements(jsigData, NULL);

	TCHAR digest[48];
	MD5ToString(data, length, digest);
	env->ReleaseByteArrayElements(jsigData, data, JNI_ABORT);
	env->DeleteLocalRef(jsigData);

	DBGLOG1("CheckSignature: %s", digest);

	// Debug 12d7100f0c5bfbe232fdbfd2e227e947
	LPCTSTR DIG_DEBUG = "12d7100f0c5bfbe232fdbfd2e227e947";
	LPCTSTR DIG_RELEASE = "39f092cb05976e1ab3d526e92d2eea81";

	if (_tcsicmp(digest, DIG_DEBUG) == 0 || _tcsicmp(digest, DIG_RELEASE) == 0)
	{
		g_isSignatureCorrect = true;
	}
	else
	{
		DBGLOG0("CheckSignature: Failed");
		// Crash jni, if signature is not correct.
		env->FindClass("com/crintsoft/music/LyricsOnlineSearch");
	}
}

jclass g_clsNative = NULL;
jmethodID	g_methodDecodeString = NULL;

jstring NewJString(JNIEnv *env, const char *str, int nLen, const char *encoding)
{
	jbyteArray buf = env->NewByteArray(nLen);
	if (buf == NULL)
		return NULL;

	jobject jstrEncoding = env->NewStringUTF(encoding);
	env->SetByteArrayRegion(buf, 0, nLen, (const jbyte *)str);

	jstring output = reinterpret_cast<jstring>(env->CallStaticObjectMethod(g_clsNative, g_methodDecodeString, buf, jstrEncoding));

	env->DeleteLocalRef(buf);
	env->DeleteLocalRef(jstrEncoding);

	return output;
}

jstring NewJUtf8String(JNIEnv *env, const char *str, int nLen)
{
	return NewJString(env, str, nLen, "utf-8");
}

extern "C" JNIEXPORT void NativeInit(JNIEnv* env, jclass clsNative, jobject ctx, jstring jstrWorkingFolder, jstring jstrAppName, jstring jstrProfile, jstring jstrClientName)
{
	g_clsNative = clsNative;
	g_methodDecodeString = env->GetStaticMethodID(g_clsNative, "decodeString", "([BLjava/lang/String;)Ljava/lang/String;");

	CheckSignature(env, ctx);

	CJStr strWorkingFolder(env, jstrWorkingFolder);
	CJStr strAppName(env, jstrAppName);
	CJStr strProfile(env, jstrProfile);
	CJStr strClientName(env, jstrClientName);

	InitBaseFrameWork(strWorkingFolder.c_str(), strAppName.c_str(), strProfile.c_str(), strAppName.c_str());

	g_mlProtocolCmd.Init(strClientName.c_str());
}

extern "C" JNIEXPORT jstring searchAllLocalLyrics(JNIEnv* env, jclass clsNative, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
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

	return NewJUtf8String(env, writer.GetBuffer().c_str(), writer.GetBuffer().size());
}

extern "C" JNIEXPORT jstring searchBestMatchLyrics(JNIEnv* env, jclass clsNative, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
{
	CJStr strFile(env, jstrFile);
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	tstring lyricsFile;
	if (g_lyricsSearch.GetBestMatchLyrics(strFile.c_str(), strArtist.c_str(), strTitle.c_str(), lyricsFile))
		return NewJUtf8String(env, lyricsFile.c_str(), lyricsFile.size());

	return NULL;
}

extern "C" JNIEXPORT void setLocalSearchOptions(JNIEnv* env, jclass clsNative, jstring jstrDownloadDir, jstring jstrLyricsDir)
{
	CJStr strDownloadDir(env, jstrDownloadDir);
	CJStr strLyricsDir(env, jstrLyricsDir);

	g_lyricsSearch.SetLyricsDir(strDownloadDir.c_str(), strLyricsDir.c_str());
}

extern "C" JNIEXPORT jstring readLyricsSource(JNIEnv* env, jclass clsNative, jstring mediaFile, jstring lyricsSrc)
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

	return NewJUtf8String(env, str.c_str(), str.size());
}

extern "C" JNIEXPORT jbyteArray encodeOnlineSearchRequest(JNIEnv* env, jclass clsNative, jstring jstrArtist, jstring jstrTitle, jboolean onlyMatched)
{
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	if (!g_isSignatureCorrect)
	{
		// Crash jni, if signature is not correct.
		env->FindClass("com/crintsoft/music/LyricsOnlineSearch");
	}

	CMLBinXMLWriter xmlStream;
	g_mlProtocolCmd.ToSearchCommand(strArtist.c_str(), strTitle.c_str(), 0, onlyMatched, xmlStream);

	CCharBuffer &buf = xmlStream.GetBuffer();
	jbyteArray jbyteArr = env->NewByteArray(buf.size());
	if (jbyteArr == NULL)
		return NULL;

	env->SetByteArrayRegion(jbyteArr, 0, buf.size(), (const jbyte *)buf.c_str());

	return jbyteArr;
}

extern "C" JNIEXPORT jstring decodeOnlineSearchReesult(JNIEnv* env, jclass clsNative, jbyteArray data, jstring jstrFile, jstring jstrArtist, jstring jstrTitle)
{
	CCharBuffer buf;
	buf.resize(env->GetArrayLength(data));
	env->GetByteArrayRegion(data, 0, buf.size(), (jbyte *)buf.data());

	CJStr strFile(env, jstrFile);
	CJStr strArtist(env, jstrArtist);
	CJStr strTitle(env, jstrTitle);

	CXMLWriter writer;
	g_mlProtocolCmd.DecodeSearchRetCommand(strFile.c_str(), strArtist.c_str(), strTitle.c_str(), buf, writer);

	return NewJUtf8String(env, writer.GetBuffer().c_str(), writer.GetBuffer().size());
}

void SetSysDefaultCharEncoding(LPCTSTR szEncoding);

extern "C" JNIEXPORT void setDefaultCharEncoding(JNIEnv* env, jclass clsNative, jstring encoding) {
	CJStr strEncoding(env, encoding);

	SetSysDefaultCharEncoding(strEncoding.c_str());
}
