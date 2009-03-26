// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iconv.h>
#include "SimpStr_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include "xstr.h"

namespace CharEncoding
{

#ifndef WIN32

#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET 129
#endif

#define CP_ACP                    0           // default to ANSI code page
#define CP_UTF8                   65001       // UTF-8 translation

	
#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#endif

#define  DBGLOG1		printf

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
	{ ED_SYSDEF, CP_ACP, DEFAULT_CHARSET, "", "", "systemdefault", "ISO-8859-15" },
	{ ED_UNICODE, 0, DEFAULT_CHARSET, "unicode", "", "Unicode", "unicode" },
	{ ED_UTF8, CP_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
	{ ED_UNICODE_BIG_ENDIAN, 0, DEFAULT_CHARSET, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
	{ ED_ARABIC, 1256, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic", "windows-1256" },
	//	{ ED_BALTIC_ISO, 28594, DEFAULT_CHARSET, "iso-8859-4", "", "Baltic (ISO)" },
	{ ED_BALTIC_WINDOWS, 1257, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
	//	{ ED_CENTRAL_EUROPEAN_ISO, 28592, DEFAULT_CHARSET, "iso-8859-2", "", "Central European (ISO)" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, 1250, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)", "windows-1250" },
	{ ED_GB2312, 936, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb2312" },
	{ ED_BIG5, 950, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
	//	{ ED_CYRILLIC, 28595, DEFAULT_CHARSET, "iso-8859-5", "", "Cyrillic (ISO)" },
	{ ED_CYRILLIC_WINDOWS, 1251, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
	//	{ ED_GREEK_ISO, 28597, DEFAULT_CHARSET, "iso-8859-7", "", "Greek (ISO)" },
	{ ED_GREEK_WINDOWS, 1253, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
	{ ED_HEBREW_WINDOWS, 1255, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
	{ ED_JAPANESE_SHIFT_JIS, 932, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
	{ ED_KOREAN, 949, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
	{ ED_LATIN9_ISO, 1252, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
	{ ED_THAI, 874, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)", "CP874" },
	//	{ ED_TURKISH_ISO, 28599, DEFAULT_CHARSET, "iso-8859-9", 0, "Turkish (ISO)" },
	{ ED_TURKISH_WINDOWS, 1254, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
	{ ED_VIETNAMESE, 1258, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
	//	{ ED_WESTERN_EUROPEAN_ISO, 28591, DEFAULT_CHARSET, "iso-8859-1", "", "Western European (ISO)" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, 1252, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, 1250, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
	{ ED_RUSSIAN_WINDOWS, 1251, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
};

size_t		__MaxEncodings = CountOf(__encodingCodepage);

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

bool IsDefaultEncodingName(LPCTSTR szEncoding)
{
	return (GetCharEncodingID(szEncoding) == ED_SYSDEF);
}

ENCODING_CODEPAGE &MapDefaultEncodingToDetail()
{
/*
	UINT		nCodePage;

	nCodePage = GetACP();
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (nCodePage == __encodingCodepage[i].nCodePage)
			return __encodingCodepage[i];
	}
*/

	return __encodingCodepage[0];
}

LPCSTR GetDefaultEncodingName()
{
	return __encodingCodepage[0].szDesc;
}

int GetEncodingID(const char *szEncoding)
{
	assert(ED_END + 1 == CountOf(__encodingCodepage));
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (stricmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return i;
	}

	return 0;
}

int GetEncodingID(const WCHAR *szEncoding)
{
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return i;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return i;
	}

	return 0;
}

bool IsUnicodeEncodingID(int nEncodingID)
{
	return GetEncodingByIdx(nEncodingID).nEncodingId == ED_UNICODE;
}

bool IsUtf8EncodingID(int nEncodingID)
{
	return GetEncodingByIdx(nEncodingID).nEncodingId == ED_UTF8;
}

ENCODING_CODEPAGE &GetEncodingByIdx(int nIndex)
{
	if (nIndex > __MaxEncodings || nIndex < 0)
		nIndex = 0;

	return __encodingCodepage[nIndex];
}

ENCODING_CODEPAGE &GetEncodingByID(CHAR_ENCODING encoding)
{
	if (encoding >= __MaxEncodings || encoding < 0)
		encoding = ED_SYSDEF;

	ASSERT(__encodingCodepage[encoding].nEncodingId == encoding);
	return __encodingCodepage[encoding];

//	for (int i = 0; i < __MaxEncodings; i++)
//	{
//		if (encoding == __encodingCodepage[i].nEncodingId)
//			return __encodingCodepage[i];
//	}
//
//	return __encodingCodepage[0];
}

ENCODING_CODEPAGE &GetEncodingByName(LPCTSTR szEncoding)
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

int ConvertMultiByteToWideChar(int nEncodingID, const char *lpByteStr, int nSizeMultibyte, WCHAR *lpWideStr, int nSizeWideChar)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UNICODE, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		lpWideStr[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeWideChar;
	n = iconv(code, (char **)&lpByteStr, (size_t*)&nSizeMultibyte, (char **)(&lpWideStr), (size_t*)&outBytesLeft);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to unicode: %s"), lpByteStr);
		n = 0;
	}
	else
		n = nSizeMultibyte - outBytesLeft / sizeof(WCHAR);

	lpWideStr[n] = '\0';
	iconv_close(code);

	return n;
}

int ConvertWideCharToMultiByte(int nEncodingID, const WCHAR *lpWideStr, int nSizeWideChar, char *lpByteStr, int nSizeMultibyte)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UNICODE, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		lpByteStr[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeMultibyte;
	n = iconv(code, (char **)&lpWideStr, (size_t*)&nSizeWideChar, (char **)(&lpByteStr), (size_t*)&outBytesLeft);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert unicode str to mbcs: %s"), lpWideStr);
		n = 0;
	}
	else
		n = nSizeMultibyte - outBytesLeft;

	lpByteStr[n] = '\0';
	iconv_close(code);
	return n;
}


void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.reserve(nLen * 3);

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, strOut.data(), strOut.capacity());

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int nEncodingID)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstringex &strOut, int nEncodingID)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen);

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, strOut.data(), strOut.capacity());

	strOut.resize(n);
}

//////////////////////////////////////////////////////////////////////////
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.resize(nLen * 3);

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, (char *)strOut.c_str(), strOut.size());

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstring &strOut, int nEncodingID)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, string &strOut, int nEncodingID)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstring &strOut, int nEncodingID)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.resize(nLen);

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, (wchar_t *)strOut.c_str(), strOut.size());

	strOut.resize(n);
}

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID)
{
	int		n;

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, strOut, nOut);

	strOut[n] = '\0';
}

void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int nEncodingID)
{
	strcpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID)
{
	wcscpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int nEncodingID)
{
	int			n;

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, strOut, nOut);

	strOut[n] = '\0';
}

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingID("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingID("utf-8"));
}

void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	int		n;
	iconv_t	code;
	char	*szOut;
	size_t	nLenOut;

	// utf-8 = mbcs 3/2 size
	strOut.reserve(nLen + nLen / 2 + 1);

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UTF8, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		strOut.resize(0);
		return;
	}

	szOut = strOut.data();
	nLenOut = strOut.capacity();
	n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&szOut), (size_t*)&nLenOut);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to utf-8: %s"), str);
		n = 0;
	}
	strOut.resize(strOut.capacity() - nLenOut);

	iconv_close(code);
}

void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	int		n;
	iconv_t	code;
	char	*szOut;
	size_t	nLenOut;

	// utf-8 = mbcs 3/2 size
	strOut.reserve(nLen + nLen / 2 + 1);

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(__encodingCodepage[nEncodingID].szIConvCode, SZ_UTF8);
	if (code == (iconv_t)-1)
	{
		strOut.resize(0);
		return;
	}

	szOut = strOut.data();
	nLenOut = strOut.capacity();
	n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&szOut), (size_t*)&nLenOut);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to utf-8: %s"), str);
		n = 0;
	}
	strOut.resize(strOut.capacity() - nLenOut);

	iconv_close(code);
}

//////////////////////////////////////////////////////////////////////////
void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingID("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstring &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingID("utf-8"));
}

void MbcsToUtf8(const char *str, int nLen, string &strOut, int nEncodingID)
{
	wstringex		strW;

	ConvertStr(str, nLen, strW, nEncodingID);

	UCS2ToUtf8(strW.c_str(), strW.size(), strOut);
}

void Utf8ToMbcs(const char *str, int nLen, string &strOut, int nEncodingID)
{
	wstringex		strW;

	Utf8ToUCS2(str, nLen, strW);

	ConvertStr(strW.c_str(), strW.size(), strOut, nEncodingID);
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

}
