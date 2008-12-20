// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include "xstr.h"

#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET 129
#endif

/*

using namespace System;
using namespace System::Text;
int main()
{
   
   // Print the header.
   Console::Write( "CodePage identifier and name     " );
   Console::WriteLine( "WindowsCodePage" );
   
   // For every encoding, get the Windows code page for it.
   System::Collections::IEnumerator^ myEnum = Encoding::GetEncodings()->GetEnumerator();
   while ( myEnum->MoveNext() )
   {
      EncodingInfo ^ ei = safe_cast<EncodingInfo ^>(myEnum->Current);
      Encoding^ e = ei->GetEncoding();
      Console::Write( "{0,-6} {1,-25} ", ei->CodePage, ei->Name );
      Console::Write( "{0,-6} ", e->WindowsCodePage );
      
      // Mark the ones that are different.
      if ( ei->CodePage != e->WindowsCodePage )
            Console::Write( "*" );
      Console::WriteLine();
   }
}
*/

/* 
This code produces the following output.

CodePage identifier and name     WindowsCodePage
37     IBM037                    1252   *
437    IBM437                    1252   *
500    IBM500                    1252   *
708    ASMO-708                  1256   *
720    DOS-720                   1256   *
737    ibm737                    1253   *
775    ibm775                    1257   *
850    ibm850                    1252   *
852    ibm852                    1250   *
855    IBM855                    1252   *
857    ibm857                    1254   *
858    IBM00858                  1252   *
860    IBM860                    1252   *
861    ibm861                    1252   *
862    DOS-862                   1255   *
863    IBM863                    1252   *
864    IBM864                    1256   *
865    IBM865                    1252   *
866    cp866                     1251   *
869    ibm869                    1253   *
870    IBM870                    1250   *
874    windows-874               874
875    cp875                     1253   *
932    shift_jis                 932
936    gb2312                    936
949    ks_c_5601-1987            949
950    big5                      950
1026   IBM1026                   1254   *
1047   IBM01047                  1252   *
1140   IBM01140                  1252   *
1141   IBM01141                  1252   *
1142   IBM01142                  1252   *
1143   IBM01143                  1252   *
1144   IBM01144                  1252   *
1145   IBM01145                  1252   *
1146   IBM01146                  1252   *
1147   IBM01147                  1252   *
1148   IBM01148                  1252   *
1149   IBM01149                  1252   *
1200   utf-16                    1200
1201   unicodeFFFE               1200   *
1250   windows-1250              1250
1251   windows-1251              1251
1252   Windows-1252              1252
1253   windows-1253              1253
1254   windows-1254              1254
1255   windows-1255              1255
1256   windows-1256              1256
1257   windows-1257              1257
1258   windows-1258              1258
1361   Johab                     949    *
10000  macintosh                 1252   *
10001  x-mac-japanese            932    *
10002  x-mac-chinesetrad         950    *
10003  x-mac-korean              949    *
10004  x-mac-arabic              1256   *
10005  x-mac-hebrew              1255   *
10006  x-mac-greek               1253   *
10007  x-mac-cyrillic            1251   *
10008  x-mac-chinesesimp         936    *
10010  x-mac-romanian            1250   *
10017  x-mac-ukrainian           1251   *
10021  x-mac-thai                874    *
10029  x-mac-ce                  1250   *
10079  x-mac-icelandic           1252   *
10081  x-mac-turkish             1254   *
10082  x-mac-croatian            1250   *
20000  x-Chinese-CNS             950    *
20001  x-cp20001                 950    *
20002  x-Chinese-Eten            950    *
20003  x-cp20003                 950    *
20004  x-cp20004                 950    *
20005  x-cp20005                 950    *
20105  x-IA5                     1252   *
20106  x-IA5-German              1252   *
20107  x-IA5-Swedish             1252   *
20108  x-IA5-Norwegian           1252   *
20127  us-ascii                  1252   *
20261  x-cp20261                 1252   *
20269  x-cp20269                 1252   *
20273  IBM273                    1252   *
20277  IBM277                    1252   *
20278  IBM278                    1252   *
20280  IBM280                    1252   *
20284  IBM284                    1252   *
20285  IBM285                    1252   *
20290  IBM290                    932    *
20297  IBM297                    1252   *
20420  IBM420                    1256   *
20423  IBM423                    1253   *
20424  IBM424                    1255   *
20833  x-EBCDIC-KoreanExtended   949    *
20838  IBM-Thai                  874    *
20866  koi8-r                    1251   *
20871  IBM871                    1252   *
20880  IBM880                    1251   *
20905  IBM905                    1254   *
20924  IBM00924                  1252   *
20932  EUC-JP                    932    *
20936  x-cp20936                 936    *
20949  x-cp20949                 949    *
21025  cp1025                    1251   *
21866  koi8-u                    1251   *
28591  iso-8859-1                1252   *
28592  iso-8859-2                1250   *
28593  iso-8859-3                1254   *
28594  iso-8859-4                1257   *
28595  iso-8859-5                1251   *
28596  iso-8859-6                1256   *
28597  iso-8859-7                1253   *
28598  iso-8859-8                1255   *
28599  iso-8859-9                1254   *
28603  iso-8859-13               1257   *
28605  iso-8859-15               1252   *
29001  x-Europa                  1252   *
38598  iso-8859-8-i              1255   *
50220  iso-2022-jp               932    *
50221  csISO2022JP               932    *
50222  iso-2022-jp               932    *
50225  iso-2022-kr               949    *
50227  x-cp50227                 936    *
51932  euc-jp                    932    *
51936  EUC-CN                    936    *
51949  euc-kr                    949    *
52936  hz-gb-2312                936    *
54936  GB18030                   936    *
57002  x-iscii-de                57002
57003  x-iscii-be                57003
57004  x-iscii-ta                57004
57005  x-iscii-te                57005
57006  x-iscii-as                57006
57007  x-iscii-or                57007
57008  x-iscii-ka                57008
57009  x-iscii-ma                57009
57010  x-iscii-gu                57010
57011  x-iscii-pa                57011
65000  utf-7                     1200   *
65001  utf-8                     1200   *
65005  utf-32                    1200   *
65006  utf-32BE                  1200   *

*/

//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE	__encodingCodepage[] = 
{
	{ ED_SYSDEF, CP_ACP, DEFAULT_CHARSET, "", "", "systemdefault" },
	{ ED_ARABIC, 1256, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic" },
	//	{ ED_BALTIC_ISO, 28594, DEFAULT_CHARSET, "iso-8859-4", "", "Baltic (ISO)" },
	{ ED_BALTIC_WINDOWS, 1257, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)" },
	//	{ ED_CENTRAL_EUROPEAN_ISO, 28592, DEFAULT_CHARSET, "iso-8859-2", "", "Central European (ISO)" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, 1250, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)" },
	{ ED_GB2312, 936, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)" },
	{ ED_BIG5, 950, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)" },
	//	{ ED_CYRILLIC, 28595, DEFAULT_CHARSET, "iso-8859-5", "", "Cyrillic (ISO)" },
	{ ED_CYRILLIC_WINDOWS, 1251, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)" },
	//	{ ED_GREEK_ISO, 28597, DEFAULT_CHARSET, "iso-8859-7", "", "Greek (ISO)" },
	{ ED_GREEK_WINDOWS, 1253, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)" },
	{ ED_HEBREW_WINDOWS, 1255, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)" },
	{ ED_JAPANESE_SHIFT_JIS, 932, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)" },
	{ ED_KOREAN, 949, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean" },
	{ ED_LATIN9_ISO, 1252, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)" },
	{ ED_THAI, 874, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)" },
	//	{ ED_TURKISH_ISO, 28599, DEFAULT_CHARSET, "iso-8859-9", 0, "Turkish (ISO)" },
	{ ED_TURKISH_WINDOWS, 1254, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)" },
	{ ED_UNICODE, 0, DEFAULT_CHARSET, "unicode", "", "Unicode" },
	{ ED_UTF8, CP_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)" },
	{ ED_VIETNAMESE, 1258, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)" },
	//	{ ED_WESTERN_EUROPEAN_ISO, 28591, DEFAULT_CHARSET, "iso-8859-1", "", "Western European (ISO)" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, 1252, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, 1250, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)" },
	{ ED_RUSSIAN_WINDOWS, 1251, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)" },
};

#define 		__MaxEncodings		CountOf(__encodingCodepage)


CHAR_ENCODING GetCharEncodingID(const char *szEncoding)
{
	assert(ED_END + 1 == CountOf(__encodingCodepage));
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_stricmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].nEncodingId;
		if (_stricmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].nEncodingId;
	}

	return ED_SYSDEF;
}

CHAR_ENCODING GetCharEncodingID(const wchar_t *szEncoding)
{
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].nEncodingId;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].nEncodingId;
	}

	return ED_SYSDEF;
}

void EnumAllEncodingsName(vector<tstring> &vEncodingNames)
{
	TCHAR		szEncoding[256];

	for (int i = 0; i < __MaxEncodings; i++)
	{
		_xcscpy(szEncoding, __encodingCodepage[i].szDesc);
		vEncodingNames.push_back(szEncoding);
	}
}

void EnumMBCSEncodingsName(vector<tstring> &vEncodingNames)
{
	TCHAR		szEncoding[256];

	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (__encodingCodepage[i].nEncodingId != ED_UTF8 && 
			__encodingCodepage[i].nEncodingId != ED_UNICODE)
		{
			_xcscpy(szEncoding, __encodingCodepage[i].szDesc);
			vEncodingNames.push_back(szEncoding);
		}
	}
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	UINT		nCodePage;

	nCodePage = GetACP();
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (nCodePage == __encodingCodepage[i].nCodePage)
			return __encodingCodepage[i];
	}

	return __encodingCodepage[0];
}

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding)
{
	if (encoding >= __MaxEncodings || encoding < 0)
		encoding = ED_SYSDEF;

	ASSERT(__encodingCodepage[encoding].nEncodingId == encoding);
	return __encodingCodepage[encoding];
}

ENCODING_CODEPAGE &GetCharEncodingByName(LPCTSTR szEncoding)
{
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
			return __encodingCodepage[i];
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i];
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i];
	}

	return __encodingCodepage[0];
}

void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int nEncodingIdx)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.reserve(nLen * 3);

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, strOut.data(), strOut.capacity(), NULL, NULL);

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int nEncodingIdx)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, stringex &strOut, int nEncodingIdx)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstringex &strOut, int nEncodingIdx)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen);

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, strOut.data(), strOut.capacity());
	if (n == 0 && nLen > 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut.data(), strOut.capacity());
	}

	strOut.resize(n);
}

//////////////////////////////////////////////////////////////////////////
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int nEncodingIdx)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.resize(nLen * 3);

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, (LPSTR)strOut.c_str(), strOut.size(), NULL, NULL);

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstring &strOut, int nEncodingIdx)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, string &strOut, int nEncodingIdx)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstring &strOut, int nEncodingIdx)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.resize(nLen);

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, (LPWSTR)strOut.c_str(), strOut.size());
	if (n == 0 && nLen > 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, (LPWSTR)strOut.c_str(), strOut.size());
	}

	strOut.resize(n);
}

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int nEncodingIdx)
{
	int		n;

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, strOut, nOut);
	if (n == 0 && nLen > 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut, nOut);
	}

	strOut[n] = '\0';
}

void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int nEncodingIdx)
{
	strcpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int nEncodingIdx)
{
	wcscpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int nEncodingIdx)
{
	int			n;

	if (nEncodingIdx >= __MaxEncodings || nEncodingIdx < 0)
		nEncodingIdx = 0;
	assert(nEncodingIdx == __encodingCodepage[nEncodingIdx].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingIdx].nCodePage, 0,
		str, nLen, strOut, nOut, NULL, NULL);

	strOut[n] = '\0';
}

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingIdx("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingIdx("utf-8"));
}

void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int nEncodingIdx)
{
	wstringex		strW;

	ConvertStr(str, nLen, strW, nEncodingIdx);

	UCS2ToUtf8(strW.c_str(), strW.size(), strOut);
}

void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int nEncodingIdx)
{
	wstringex		strW;

	Utf8ToUCS2(str, nLen, strW);

	ConvertStr(strW.c_str(), strW.size(), strOut, nEncodingIdx);
}

//////////////////////////////////////////////////////////////////////////
void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingIdx("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstring &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingIdx("utf-8"));
}

void MbcsToUtf8(const char *str, int nLen, string &strOut, int nEncodingIdx)
{
	wstringex		strW;

	ConvertStr(str, nLen, strW, nEncodingIdx);

	UCS2ToUtf8(strW.c_str(), strW.size(), strOut);
}

void Utf8ToMbcs(const char *str, int nLen, string &strOut, int nEncodingIdx)
{
	wstringex		strW;

	Utf8ToUCS2(str, nLen, strW);

	ConvertStr(strW.c_str(), strW.size(), strOut, nEncodingIdx);
}

//
// 判断字符串中是否包含非ANSI编码的字符
//
bool IsAnsiStr(const WCHAR *szStr)
{
	while (*szStr)
	{
		if (*szStr >= 128)
			return false;
		szStr++;
	}

	return true;
}

bool IsAnsiStr(const char *szStr)
{
	while (*szStr)
	{
		if ((unsigned char)*szStr >= 128)
			return false;
		szStr++;
	}

	return true;
}


/*


//
// 将MultibyeChar转换未UTF-8码
int MultibyteToUTF8(LPCSTR szInput, LPSTR szOutput, int nOutputSize)
{
	WCHAR		*wchBegin;
	LPSTR		szOutStr;
	int			nOutPos = 0;
	WCHAR		wUnicode[1024];

	if (MultiByteToWideChar(CP_ACP, 0, szInput, -1,
		wUnicode, 1024) == 0)
	{
		assert(GetLastError() != ERROR_INSUFFICIENT_BUFFER);
		assert(0);
		return 0;
	}
	
	wchBegin = (WCHAR *)wUnicode;
	szOutStr = szOutput;
	while (*wchBegin && nOutPos < nOutputSize - 3)
	{
		if (*wchBegin < 0x80)
		{
			szOutStr[nOutPos] = (BYTE)*wchBegin;
			nOutPos++;
			wchBegin++;
		}
		else if (*wchBegin < 0x0800)
		{
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 6) | 0xC0);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
		}
		else
		{
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 12) | 0xE0);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 6 & 0x3F) | 0x80);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
			wchBegin ++;
  		}
	}
	szOutStr[nOutPos] = '\0';

	return nOutPos;
}

#define MXS(c,x,s)			(((c) - (x)) <<  (s))
#define M80S(c,s)			MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)	((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int UTF8ToUCS2(unsigned char* text, int length, WCHAR *ucs2, int ubuflen)
{
	unsigned char* p = text;
	unsigned char* last = text + length;
	int		ul;
	for(p = text, ul = 0; (p < last) && (ul < ubuflen) ; )
	{
		if( (*p) < 0x80)
		{	
			ucs2[ul++] = UTF8_1_to_UCS2(p);
			p += 1;
		}
		else if( (*p) < 0xc0 )
		{	
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
		else if( (*p) < 0xe0)
		{
			ucs2[ul++] = UTF8_2_to_UCS2(p);
			p += 2;
		}
		else if( (*p) < 0xf0)
		{
			ucs2[ul++] = UTF8_3_to_UCS2(p);
			p += 3;
		}
		else if( (*p) < 0xf8)
		{
			ucs2[ul++] = '?';
			p += 4;
		}
		else if( (*p) < 0xfc)
		{
			ucs2[ul++] = '?';
			p += 5;
		}
		else if( (*p) < 0xfe)
		{
			ucs2[ul++] = '?';
			p += 6;
		}
		else
		{
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
	}
	return ul;
}

int UTF8ToMultibyte(unsigned char* text, char *szOut, int nOutBuffLen)
{
	WCHAR		wchWideChar[1024];

	UTF8ToUCS2(text, strlen((char *)text) + 1, wchWideChar, 1024);

	return WideCharToMultiByte(CP_ACP, 0, wchWideChar, -1, szOut, nOutBuffLen, NULL, NULL);
}
*/
