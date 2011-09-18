#include "stdafx.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include "xstr.h"

CHAR_ENCODING FileEcodingToCharEncoding(FILE_ENCODING fileEncoding)
{
	switch (fileEncoding)
	{
	case FE_UCS2:
		return ED_UNICODE;
	case FE_UCS2_BE:
		return ED_UNICODE_BIG_ENDIAN;
	case FE_UTF8:
		return ED_UTF8;
	case FE_ANSI:
		return ED_SYSDEF;
	default:
		ASSERT(0);
		return ED_SYSDEF;
	}
}

FILE_ENCODING CharEncodingToFileEncoding(CHAR_ENCODING encoding)
{
	switch (encoding)
	{
	case ED_UNICODE:
		return FE_UCS2;
	case ED_UNICODE_BIG_ENDIAN:
		return FE_UCS2_BE;
	case ED_UTF8:
		return FE_UTF8;
	default:
		return FE_ANSI;
	}
}

CHAR_ENCODING GetCharEncodingID(const char *szEncoding)
{
	assert(ED_END + 1 == GetCharEncodingCount());
	for (int i = GetCharEncodingCount() - 1; i >= 0; i--)
	{
		if (_stricmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].encodingID;
		if (_stricmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].encodingID;
	}

	return ED_SYSDEF;
}

CHAR_ENCODING GetCharEncodingID(const WCHAR *szEncoding)
{
	assert(ED_END + 1 == GetCharEncodingCount());
	for (int i = GetCharEncodingCount() - 1; i >= 0; i--)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].encodingID;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].encodingID;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
			return __encodingCodepage[i].encodingID;
	}

	return ED_SYSDEF;
}

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding)
{
	if (encoding >= GetCharEncodingCount() || encoding < 0)
		encoding = ED_SYSDEF;

	ASSERT(__encodingCodepage[encoding].encodingID == encoding);
	return __encodingCodepage[encoding];
}

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

int UCS2ToUtf8(const WCHAR *str, int nLen, char *strOut, int nOut)
{
	EnsureInputStrLen(str, nLen);

	LPCWSTR		wchBegin = str, wEnd = str + nLen;
	int			nOutPos = 0;

	while (wchBegin < wEnd)
	{
		if (*wchBegin < 0x80)
		{
			if (nOutPos + 1 >= nOut)
				break;

			strOut[nOutPos] = (BYTE)*wchBegin;
			nOutPos++;
			wchBegin++;
		}
		else if (*wchBegin < 0x0800)
		{
			if (nOutPos + 2 >= nOut)
				break;

			strOut[nOutPos] = (BYTE)((*wchBegin >> 6) | 0xC0);
			nOutPos++;
			strOut[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
		}
		else
		{
			if (nOutPos + 3 >= nOut)
				break;

			strOut[nOutPos] = (BYTE)((*wchBegin >> 12) | 0xE0);
			nOutPos++;
			strOut[nOutPos] = (BYTE)((*wchBegin >> 6 & 0x3F) | 0x80);
			nOutPos++;
			strOut[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
			wchBegin ++;
		}
	}
	strOut[nOutPos] = '\0';

	return nOutPos;
}

#define MXS(c,x,s)			(((c) - (x)) <<  (s))
#define M80S(c,s)			MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)	((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int Utf8ToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut)
{
	EnsureInputStrLen(str, nLen);

	unsigned char* p = (unsigned char *)str;
	unsigned char* last = (unsigned char *)str + nLen;
	int		ul;
	for (ul = 0; (p < last) && (ul < nOut); )
	{
		if ((*p) < 0x80)
		{	
			strOut[ul++] = UTF8_1_to_UCS2(p);
			p += 1;
		}
		else if ((*p) < 0xc0)
		{	
			assert((*p));	// Invalid UTF8 First Byte
			strOut[ul++] = '?';
			p += 1;
		}
		else if ((*p) < 0xe0)
		{
			strOut[ul++] = UTF8_2_to_UCS2(p);
			p += 2;
		}
		else if ((*p) < 0xf0)
		{
			strOut[ul++] = UTF8_3_to_UCS2(p);
			p += 3;
		}
		else if ((*p) < 0xf8)
		{
			strOut[ul++] = '?';
			p += 4;
		}
		else if ((*p) < 0xfc)
		{
			strOut[ul++] = '?';
			p += 5;
		}
		else if ((*p) < 0xfe)
		{
			strOut[ul++] = '?';
			p += 6;
		}
		else
		{
			assert((*p));	// Invalid UTF8 First Byte
			strOut[ul++] = '?';
			p += 1;
		}
	}
	return ul;
}

int AnsiToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut)
{
	EnsureInputStrLen(str, nLen);

	if (nOut <= nLen)
		nLen = nOut - 1;

	_xcsncpy(strOut, str, nLen);
	strOut[nLen] = 0;

	return nLen;
}

int UCS2ToAnsi(const WCHAR *str, int nLen, char *strOut, int nOut)
{
	EnsureInputStrLen(str, nLen);

	if (nOut <= nLen)
		nLen = nOut - 1;

	_xcsncpy(strOut, str, nLen);
	strOut[nLen] = 0;

	return nLen;
}

//////////////////////////////////////////////////////////////////////////

void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 3 + 3);
	int n = UCS2ToUtf8(str, nLen, (char *)strOut.c_str(), strOut.capacity());
	strOut.resize(n);
}

void UCS2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 2 + 2);
	int n = UCS2ToMbcs(str, nLen, (char *)strOut.c_str(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void Utf8ToUCS2(const char *str, int nLen, wstring_t &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = Utf8ToUCS2(str, nLen, (WCHAR *)strOut.c_str(), strOut.capacity());
	strOut.resize(n);
}

void Utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = Utf8ToMbcs(str, nLen, (char *)strOut.c_str(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void MbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 3 / 2 + 3);
	int n = MbcsToUtf8(str, nLen, (char *)strOut.c_str(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void MbcsToUCS2(const char *str, int nLen, wstring_t &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = MbcsToUCS2(str, nLen, (WCHAR *)strOut.c_str(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void AnsiToUCS2(const char *str, int nLen, wstring_t &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen);
	_xcsncpy((WCHAR *)strOut.c_str(), str, nLen);
}

void UCS2ToAnsi(const WCHAR *str, int nLen, string &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen);
	_xcsncpy((char *)strOut.c_str(), str, nLen);
}

//////////////////////////////////////////////////////////////////////////

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 3 + 3);
	int n = UCS2ToUtf8(str, nLen, strOut.data(), strOut.capacity());
	strOut.resize(n);
}

void UCS2ToMbcs(const WCHAR *str, int nLen, stringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 2 + 2);
	int n = UCS2ToMbcs(str, nLen, strOut.data(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = Utf8ToUCS2(str, nLen, strOut.data(), strOut.capacity());
	strOut.resize(n);
}

void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = Utf8ToMbcs(str, nLen, strOut.data(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen * 3 / 2 + 3);
	int n = MbcsToUtf8(str, nLen, strOut.data(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void MbcsToUCS2(const char *str, int nLen, wstringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen + 1);
	int n = MbcsToUCS2(str, nLen, strOut.data(), strOut.capacity(), encodingID);
	strOut.resize(n);
}

void AnsiToUCS2(const char *str, int nLen, wstringex &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen);
	_xcsncpy((WCHAR *)strOut.c_str(), str, nLen);
}

void UCS2ToAnsi(const WCHAR *str, int nLen, stringex &strOut)
{
	EnsureInputStrLen(str, nLen);

	strOut.resize(nLen);
	_xcsncpy((char *)strOut.c_str(), str, nLen);
}

//////////////////////////////////////////////////////////////////////////

void ConvertStr(const char *str, int nLen, wstringex &strOut, int encodingID)
{
	MbcsToUCS2(str, nLen, strOut, encodingID);
}

void ConvertStr(const char *str, int nLen, stringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.clear();
	strOut.append(str, nLen);
}

void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);
	strOut.clear();
	strOut.append(str, nLen);
}

void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int encodingID)
{
	UCS2ToMbcs(str, nLen, strOut, encodingID);
}

//////////////////////////////////////////////////////////////////////////

void ConvertStr(const char *str, int nLen, wstring_t &strOut, int encodingID)
{
	MbcsToUCS2(str, nLen, strOut, encodingID);
}

void ConvertStr(const char *str, int nLen, string &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);

	strOut.clear();
	strOut.append(str, nLen);
}

void ConvertStr(const WCHAR *str, int nLen, wstring_t &strOut, int encodingID)
{
	EnsureInputStrLen(str, nLen);
	strOut.clear();
	strOut.append(str, nLen);
}

void ConvertStr(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
	UCS2ToMbcs(str, nLen, strOut, encodingID);
}

//////////////////////////////////////////////////////////////////////////

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
	MbcsToUCS2(str, nLen, strOut, nOut, encodingID);
}

void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
	if (nLen == -1)
		strcpy_safe(strOut, nOut, str);
	else
	{
		nLen = strlen(str);
		strncpy_safe(strOut, nOut, str, nLen);
	}
}

void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
	if (nLen == -1)
		wcscpy_safe(strOut, nOut, str);
	else
	{
		nLen = wcslen(str);
		wcsncpy_safe(strOut, nOut, str, nLen);
	}
}

void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
	UCS2ToMbcs(str, nLen, strOut, nOut, encodingID);
}

void UCS2BEToLE(WCHAR *str, int nLen)
{
	// Swap the char of every WCHAR.
	if (nLen == -1)
		nLen = wcslen(str);

	byte	*p = (byte *)str;
	int		lenBytes = nLen * sizeof(WCHAR);
	for (int i = 0; i < lenBytes; i += 2)
	{
		byte t = p[i];
		p[i] = p[i + 1];
		p[i + 1] = t;
	}
}

void UCS2LEToBE(WCHAR *str, int nLen)
{
	UCS2BEToLE(str, nLen);
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CharEncodingBase)

class CTestCaseCharEncodingBase : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCharEncodingBase);
	CPPUNIT_TEST(TestUcs2ToUtf8);
	CPPUNIT_TEST(TestAnsiToUcs2);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestUcs2ToUtf8()
	{
		string out;
		stringex outx;
		char szOut[256];

		LPCWSTR		input = L"Test String abc";

		// Test with full length: -1
		UCS2ToUtf8(input, -1, out);
		UCS2ToUtf8(input, -1, outx);
		int n = UCS2ToUtf8(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == wcslen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		UCS2ToUtf8(input, nLen, out);
		UCS2ToUtf8(input, nLen, outx);
		n = UCS2ToUtf8(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = UCS2ToUtf8(input, -1, szOut, nLen + 1);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(strlen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

		// if (IsChsOS())
		{
			input = L"中文测试中文测试中文测试中文测试中文测试中文测试中文测试";
			UCS2ToUtf8(input, -1, out);

			// Use MbcsToUCS2 with ED_UTF8 encoding to convert.
			wstring_t strInputNew;
			MbcsToUCS2(out.c_str(), out.size(), strInputNew, ED_UTF8);
			CPPUNIT_ASSERT(strInputNew == input);
		}
	}

	void TestUcs2ToMbcs()
	{
		string out;
		stringex outx;
		char szOut[256];

		LPCWSTR		input = L"Test String abc";

		// Test with full length: -1
		UCS2ToMbcs(input, -1, out);
		UCS2ToMbcs(input, -1, outx);
		int n = UCS2ToMbcs(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == wcslen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		UCS2ToMbcs(input, nLen, out);
		UCS2ToMbcs(input, nLen, outx);
		n = UCS2ToMbcs(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = UCS2ToMbcs(input, -1, szOut, nLen);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(strlen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

		// if (IsChsOS())
		{
			input = L"中文测试中文测试中文测试中文测试中文测试中文测试中文测试";
			UCS2ToMbcs(input, -1, out);
			CPPUNIT_ASSERT(out.size() == wcslen(input));

			wstring_t strInputNew;
			MbcsToUCS2(out.c_str(), out.size(), strInputNew);
			CPPUNIT_ASSERT(strInputNew == input);
		}
	}

	void TestMbcsToUcs2()
	{
		wstring_t out;
		wstringex outx;
		WCHAR szOut[256];

		LPCSTR		input = "Test String abc";

		// Test with full length: -1
		MbcsToUCS2(input, -1, out);
		MbcsToUCS2(input, -1, outx);
		int n = MbcsToUCS2(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == strlen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		MbcsToUCS2(input, nLen, out);
		MbcsToUCS2(input, nLen, outx);
		n = MbcsToUCS2(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = MbcsToUCS2(input, -1, szOut, nLen);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(wcslen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
	}

	void TestMbcsToUtf8()
	{
		string out;
		stringex outx;
		char szOut[256];

		LPCSTR		input = "Test String abc";

		// Test with full length: -1
		MbcsToUtf8(input, -1, out);
		MbcsToUtf8(input, -1, outx);
		int n = MbcsToUtf8(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == strlen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		MbcsToUtf8(input, nLen, out);
		MbcsToUtf8(input, nLen, outx);
		n = MbcsToUtf8(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = MbcsToUtf8(input, -1, szOut, nLen);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(strlen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

		// if (IsChsOS())
		{
			input = "中文测试中文测试中文测试中文测试中文测试中文测试中文测试";
			MbcsToUtf8(input, -1, out);

			string strInputNew;
			Utf8ToMbcs(out.c_str(), out.size(), strInputNew);
			CPPUNIT_ASSERT(strInputNew == input);
		}
	}

	void TestUtf8ToMbcs()
	{
		string out;
		stringex outx;
		char szOut[256];

		LPCSTR		input = "Test String abc";

		// Test with full length: -1
		Utf8ToMbcs(input, -1, out);
		Utf8ToMbcs(input, -1, outx);
		int n = Utf8ToMbcs(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == strlen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		Utf8ToMbcs(input, nLen, out);
		Utf8ToMbcs(input, nLen, outx);
		n = Utf8ToMbcs(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = Utf8ToMbcs(input, -1, szOut, nLen);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(strlen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
	}

	void TestUtf8ToUcs2()
	{
		wstring_t out;
		wstringex outx;
		WCHAR szOut[256];

		LPCSTR		input = "Test String abc";

		// Test with full length: -1
		Utf8ToUCS2(input, -1, out);
		Utf8ToUCS2(input, -1, outx);
		int n = Utf8ToUCS2(input, -1, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == strlen(input));
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

		// Test with shorter length
		int			nLen = 5;
		Utf8ToUCS2(input, nLen, out);
		Utf8ToUCS2(input, nLen, outx);
		n = Utf8ToUCS2(input, nLen, szOut, CountOf(szOut));
		CPPUNIT_ASSERT(out.size() == n);
		CPPUNIT_ASSERT(outx.size() == n);
		CPPUNIT_ASSERT(out.size() == nLen);
		CPPUNIT_ASSERT(out == outx.c_str());
		CPPUNIT_ASSERT(out == szOut);
		CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

		// Test with insufficient length.
		nLen = 5;
		n = Utf8ToUCS2(input, -1, szOut, nLen);
		CPPUNIT_ASSERT(n == nLen);
		CPPUNIT_ASSERT(wcslen(szOut) == nLen);
		CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
	}

	void TestAnsiToUcs2()
	{
		const int bufLen = 256;
		byte	buf[bufLen];

		wstring_t	strUcs2;
		AnsiToUCS2((const char *)buf, bufLen, strUcs2);
		CPPUNIT_ASSERT(strUcs2.size() == bufLen);

		string		strAnsi;
		UCS2ToAnsi(strUcs2.c_str(), strUcs2.size(), strAnsi);
		CPPUNIT_ASSERT(strAnsi.size() == bufLen);
		CPPUNIT_ASSERT(memcmp(strAnsi.c_str(), buf, bufLen) == 0);

		LPCSTR	strTest = "abcdefalsgeoie1123";
		AnsiToUCS2(strTest, -1, strUcs2);
		CPPUNIT_ASSERT(strUcs2.size() == strlen(strTest));

		UCS2ToAnsi(strUcs2.c_str(), -1, strAnsi);
		CPPUNIT_ASSERT(strAnsi.size() == strlen(strTest));
		CPPUNIT_ASSERT(memcmp(strAnsi.c_str(), strTest, strlen(strTest) + 1) == 0);
	}

	bool IsChsOS()
	{
		return GetACP() == 936;
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCharEncodingBase);


#endif // _CPPUNIT_TEST
