// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "../../base/base.h"
#include "../CharEncoding.h"

#define DBGLOG1 printf

// The order of ED_XXX is the order of __encodingCodepage 
// __encodingCodepage[ED_XXX].encodingID = ED_XXX
ENCODING_CODEPAGE	__encodingCodepage[] = 
{
	{ ED_SYSDEF, "", "", "systemdefault", "ISO-8859-15" },
	{ ED_UNICODE, "unicode", "", "Unicode", "unicode" },
	{ ED_UNICODE_BIG_ENDIAN, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
	{ ED_UTF8, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
	{ ED_ARABIC, "windows-1256", "arabic", "Arabic", "windows-1256" },
	{ ED_BALTIC_WINDOWS, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, "windows-1250", "", "Central European (Windows)", "windows-1250" },
	{ ED_GB2312, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb18030" },
	{ ED_BIG5, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
	{ ED_CYRILLIC_WINDOWS, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
	{ ED_GREEK_WINDOWS, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
	{ ED_HEBREW_WINDOWS, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
	{ ED_JAPANESE_SHIFT_JIS, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
	{ ED_KOREAN, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
	{ ED_LATIN9_ISO, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
	{ ED_THAI, "windows-874", "Thai", "Thai (Windows)", "CP874" },
	{ ED_TURKISH_WINDOWS, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
	{ ED_VIETNAMESE, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
	{ ED_RUSSIAN_WINDOWS, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
};

#define 		__MaxEncodings		CountOf(__encodingCodepage)

int GetCharEncodingCount() { return __MaxEncodings; }


ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	return __encodingCodepage[ED_UTF8];
}

int MbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
	if (encodingID == ED_UTF8)
	{
		if (nLen == -1)
		{
			strcpy_safe(strOut, nOut, str);
			return strlen(strOut);
		}
		else
		{
			strncpysz_safe(strOut, nOut, str, nLen);
			return nLen;
		}
	}

	EmptyStr(strOut);

	if (encodingID > __MaxEncodings || encodingID < 0)
		encodingID = 0;
	assert(encodingID == __encodingCodepage[encodingID].encodingID);

	wstringex strW;
	MbcsToUCS2(str, nLen, strW, encodingID);

	return UCS2ToUtf8(strW.c_str(), strW.size(), strOut, nOut);
}

int Utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
	if (encodingID == ED_UTF8)
	{
		if (nLen == -1)
		{
			strcpy_safe(strOut, nOut, str);
			return strlen(strOut);
		}
		else
		{
			strncpysz_safe(strOut, nOut, str, nLen);
			return nLen;
		}
	}

	EmptyStr(strOut);

	if (encodingID > __MaxEncodings || encodingID < 0)
		encodingID = 0;
	assert(encodingID == __encodingCodepage[encodingID].encodingID);

	wstringex strW;

	Utf8ToUCS2(str, nLen, strW);

	return UCS2ToMbcs(strW.c_str(), strW.size(), strOut, nOut, encodingID);
}

jclass		g_clsString = NULL;
jmethodID	g_methodStringInitWithBufEncoding = NULL;
jmethodID	g_methodStringGetBytesEncoding = NULL;

bool PrepareJString(JNIEnv *env)
{
	if (g_clsString == NULL)
	{
		jclass clsString = env->FindClass("java/lang/String");
		if (clsString == NULL)
			return false;

		g_clsString = reinterpret_cast<jclass>(env->NewGlobalRef(clsString));
	}

	if (g_methodStringInitWithBufEncoding == NULL)
	{
		jmethodID mtdString = env->GetMethodID(g_clsString, "<init>", "([BLjava/lang/String;)V");
		if (mtdString == NULL)
			return false;

		g_methodStringInitWithBufEncoding = mtdString;
	}

	if (g_methodStringGetBytesEncoding == NULL)
	{
		jmethodID mtdString = env->GetMethodID(g_clsString, "getBytes", "(Ljava/lang/String;)[B");
		if (mtdString == NULL)
			return false;

		g_methodStringGetBytesEncoding = mtdString;
	}

	return true;
}

jstring NewJString(JNIEnv *env, const char *str, int nLen, const char *encoding)
{
	if (!PrepareJString(env))
		return env->NewStringUTF("Prepare JString");

	jobject objString = env->AllocObject(g_clsString);
	if (objString == NULL)
		return env->NewStringUTF("Failed to alloc object String");

	jbyteArray buf = env->NewByteArray(100);
	if (buf == NULL)
		return env->NewStringUTF("Failed: NewByteArray");

	env->SetByteArrayRegion(buf, 0, nLen, (const jbyte *)str);

	jobject jstrEncoding = env->NewStringUTF(encoding);

	env->CallVoidMethod(objString, g_methodStringInitWithBufEncoding, buf, jstrEncoding);

	env->DeleteLocalRef(buf);
	env->DeleteLocalRef(jstrEncoding);

	return reinterpret_cast<jstring>(objString);
}

int GetBytesOfJString(JNIEnv *env, jstring jstr, char *strOut, int nOut, const char *encoding)
{
	if (!PrepareJString(env))
		return 0;

	jbyteArray buf = (jbyteArray)env->CallObjectMethod(jstr, g_methodStringGetBytesEncoding, env->NewStringUTF(encoding));
	if (buf == NULL)
		return 0;

	int length = env->GetArrayLength(buf);
	if (nOut > length - 1)
		nOut = length - 1;

	env->GetByteArrayRegion(buf, 0, nOut, (jbyte *)strOut);
	strOut[nOut] = '\0';

	env->DeleteLocalRef(buf);

	return nOut;
}

int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
	EmptyStr(strOut);

	if (encodingID > __MaxEncodings || encodingID < 0)
		encodingID = 0;
	assert(encodingID == __encodingCodepage[encodingID].encodingID);

	JNIEnv *env;

	g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

	jstring jstr = NewJString(env, str, nLen, __encodingCodepage[encodingID].szIConvCode);

	const jchar * ucs2 = env->GetStringChars(jstr, NULL);

	wcscpy_safe(strOut, nOut, ucs2);

	env->ReleaseStringChars(jstr, ucs2);

	env->DeleteLocalRef(jstr);

	return wcslen(strOut);
}

int UCS2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
	EmptyStr(strOut);
	if (encodingID > __MaxEncodings || encodingID < 0)
		encodingID = 0;
	assert(encodingID == __encodingCodepage[encodingID].encodingID);

	JNIEnv *env;
	g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

	jstring jstr = env->NewString(str, nLen);

	return GetBytesOfJString(env, jstr, strOut, nOut, __encodingCodepage[encodingID].szIConvCode);
}

