// HelperFun.cpp: implementation of the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ID3v2.h"
#include "ID3Helper.h"
/*
void AppendUcs2Str(CCharBuffer &buff, wstringex &strUcs2)
{
	// Unicode
	buff += (char)0xFF;
	buff += (char)0xFE;
	buff.append((const char *)strUcs2.c_str(), (strUcs2.size() + 1) * sizeof(WCHAR));
}
*/

void AppendStrByEncodingAndBom(CCharBuffer &buff, LPCTSTR str, ID3v2EncType encType, CHAR_ENCODING encoding)
{
	if (encType == IET_UCS2LE_BOM)
	{
		buff += SZ_FE_UCS2;
		wstringex	strW;
		TStringToUCS2(str, -1, strW, encoding);
		buff.append((const char *)strW.c_str(), (strW.size() + 1) * sizeof(WCHAR));
	}
	else if (encType == IET_UCS2BE_NO_BOM)
	{
		wstringex	strW;
		TStringToUCS2(str, -1, strW, encoding);
		UCS2LEToBE(strW.data(), strW.size());
		buff.append((const char *)strW.c_str(), (strW.size() + 1) * sizeof(WCHAR));
	}
	else if (encType == IET_UTF8)
	{
		// 		buff += (char)0xEF;
		// 		buff += (char)0xBB;
		// 		buff += (char)0xBF;
		stringex	strUtf8;
		TStringToUTF8(str, -1, strUtf8, encoding);
		buff.append(strUtf8.c_str(), strUtf8.size() + 1);
	}
	else
	{
		// IET_ANSI
		stringex	strAnsi;
		TStringToMBCS(str, -1, strAnsi, encoding);
		buff.append(strAnsi.c_str(), strAnsi.size() + 1);
	}
}

// return the size of bytes used from buffer data
int CopyAnsiStr(string &str, const char *data, int nLen)
{
	str.resize(0);

	int nStrLen = strlen_safe(data, nLen);
	str.append(data, nStrLen);
	return nStrLen + 1;
}

// return the size of bytes used from buffer data
int CopyStrByEncodingAndBom(tstringex &str, ID3v2EncType encType, const char *data, int len, CHAR_ENCODING encoding)
{
	int		lenOrg = len;
	LPBYTE	pbyData = (LPBYTE)data;

	str.resize(0);

	// Determine the encoding type.
	if (pbyData[0] == 0xFF && pbyData[1] == 0xFE)
	{
		ASSERT(encType == IET_UCS2LE_BOM);
		encType = IET_UCS2LE_BOM;
		pbyData += 2;
		len -= 2;
	}
	else if (pbyData[0] == 0xFE && pbyData[1] == 0xFF)
	{
		ASSERT(encType == IET_UCS2BE_NO_BOM);
		encType = IET_UCS2BE_NO_BOM;
		pbyData += 2;
		len -= 2;
	}
	else if (pbyData[0] == 0xEF && pbyData[1] == 0xBB && pbyData[2] == 0xBF)
	{
		encType = IET_UTF8;
		pbyData += 3;
		len -= 3;
	}

	// Copy data
	int		nCopyiedBytes = 0;
	if (encType == IET_UCS2LE_BOM || encType == IET_UCS2BE_NO_BOM)
	{
		int n = wcslen_safe((WCHAR *)pbyData, len / sizeof(WCHAR));

		if (encType == IET_UCS2BE_NO_BOM)
			UCS2BEToLE((WCHAR *)pbyData, n);
		UCS2ToTString((WCHAR *)pbyData, n, str, encoding);

		nCopyiedBytes = (n + 1) * sizeof(WCHAR);
	}
	else if (encType == IET_UTF8)
	{
		int n = strlen_safe((const char *)pbyData, len);

		UTF8ToTString((const char *)pbyData, n, str, encoding);

		nCopyiedBytes = n + 1;
	}
	else
	{
		int n = strlen_safe((const char *)pbyData, len);
		MBCSToTString((const char *)pbyData, n, str, encoding);

		nCopyiedBytes = n + 1;
	}

	return lenOrg - len + nCopyiedBytes;
}

// return the size of bytes used from buffer data
int CopyStrByEncodingAndBom(tstring &str, ID3v2EncType encType, const char *data, int len, CHAR_ENCODING encoding)
{
	tstringex	str2;
	int n = CopyStrByEncodingAndBom(str2, encType, data, len, encoding);
	str.clear();
	str.append(str2.c_str(), str2.size());

	return n;
}

uint32 SynchDataToUInt(BYTE *byData, int nLen)
{
	uint32	sum = 0;
	int		last;
	if (nLen > 4)
		last = 3;
	else
		last = nLen - 1;

	for(int i = 0; i <= last; i++)
		sum |= (byData[i] & 0x7f) << ((last - i) * 7);

	return sum;
}

void SynchDataFromUInt(uint32 nValue, BYTE *byData, int nLen)
{
	int		last;
	last = nLen - 1;
	for(int i = 0; i < nLen; i++)
		byData[i] = BYTE((nValue >> ((last - i) * 7)) & 0x7f);

}

uint32 ByteDataToUInt(BYTE *byData, int nLen)
{
	uint32	sum = 0;
	int		last;
	if (nLen > 4)
		last = 3;
	else
		last = nLen - 1;

	for(int i = 0; i <= last; i++)
		sum |= byData[i] << ((last - i) * 8);

	return sum;
}

void ByteDataFromUInt(uint32 nValue, BYTE *byData, int nLen)
{
	int		last;
	last = nLen - 1;
	for(int i = 0; i < nLen; i++)
		byData[i] = BYTE((nValue >> ((last - i) * 8)) & 0xff);
}

int FileDataReset(FILE *m_fp, int nOffset, int nSize, int nMask)
{
	char	buff[4096];

	memset(buff, nMask, sizeof(buff));

	if (fseek(m_fp, nOffset, SEEK_SET) != 0)
		return ERR_SEEK_FILE;

	int		nLen;

	for (int i = nSize; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fwrite(buff, 1, nLen, m_fp) != nLen)
		{
			return ERR_WRITE_FILE;
		}
	}

	return ERR_OK;
}

int FileDataCmp(FILE *m_fp, int nOffset, LPCVOID lpData, int nSize, int &nResult)
{
	char	buff[4096];
	char	*pData = (char *)lpData;

	if (fseek(m_fp, nOffset, SEEK_SET) != 0)
		return ERR_SEEK_FILE;

	int		nLen;

	nResult = 0;
	for (int i = nSize; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fread(buff, 1, nLen, m_fp) != nLen)
		{
			return ERR_READ_FILE;
		}
		nResult = memcmp(buff, pData, nLen);
		if (nResult != 0)
			return ERR_OK;
		pData += nLen;
	}

	return ERR_OK;
}

/*
//	Test function for FileDataCmp(), FileDataReset()
void TestFileDataCmp()
{
	// FileMoveEndData
	FILE		*fp;
	int			nRet;

	fp = fopen("c:\\1.txt", "r+b");
	if (!fp)
		return;

	nRet = FileDataReset(fp, 4, 1000, 0x4d);
	if (nRet != ERR_OK)
		return;

	CCharBuffer		buff;
	int				nResult;

	// buff.append("\0\0", 2);
	for (int i = 0; i < 400; i++)
		buff += 0x4d;

	nRet = FileDataCmp(fp, 100, buff.c_str(), buff.size(), nResult);
	if (nRet != ERR_OK)
		return;

	if (nResult != 0)
		AfxMessageBox("Not equal");

	fclose(fp);
}
*/

int FileAppendData(FILE *m_fp, int nAppend, int nMask)
{
	char	buff[4096];
	int		nOldLen, nLen;

	memset(buff, nMask, sizeof(buff));

	if (fseek(m_fp, 0, SEEK_END) != 0)
		return ERR_SEEK_FILE;

	nOldLen = ftell(m_fp);

	for (int i = nAppend; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fwrite(buff, 1, nLen, m_fp) != nLen)
		{
			// write error!
			filetruncate(m_fp, ftell(m_fp));

			return ERR_WRITE_FILE;
		}
	}

	return ERR_OK;
}

int FileMoveEndData(FILE *m_fp, int nOffset, int nNewOffset)
{
	CCharBuffer		buff;

	buff.reserve(1024 * 128);
	if (!buff.data())
		buff.reserve(1024);
	if (!buff.data())
		return ERR_NO_MEM;

	int			nLen, nPos, nFileLen, nNewAppend;
	int			nRet;

	if (fseek(m_fp, 0, SEEK_END) != 0)
		return ERR_SEEK_FILE;

	nFileLen = ftell(m_fp);

	if (nOffset >= nFileLen)
	{
		assert(nOffset < nFileLen);
		return ERR_FALSE;
	}

	if (nNewOffset == nOffset)
		return ERR_OK;
	else if (nNewOffset > nOffset)
	{
		nRet = FileAppendData(m_fp, nNewOffset - nOffset);
		if (nRet != ERR_OK)
			return nRet;

		int		nLastEndPos;

		nNewAppend = nNewOffset - nOffset;
		nLastEndPos = nFileLen;

		// from end to move data
		//     ---oooo
		//     ----oooo
		while (nLastEndPos > nOffset)
		{
			nPos = nLastEndPos - buff.capacity();
			if (nPos < nOffset)
				nPos = nOffset;
			nLen = nLastEndPos - nPos;

			if (fseek(m_fp, nPos, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fread(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_READ_FILE;
			if (fseek(m_fp, nPos + nNewAppend, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fwrite(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_WRITE_FILE;

			nLastEndPos = nPos;
		}
	}
	else
	{
		// from begin to move data
		//     ---oooo
		//     --oooo
		int		nNextPos;

		nNewAppend = nOffset - nNewOffset;
		nPos = nOffset;
		while (nPos < nFileLen)
		{
			nNextPos = nPos + buff.capacity();
			if (nNextPos > nFileLen)
				nNextPos = nFileLen;
			nLen = nNextPos - nPos;

			if (fseek(m_fp, nPos, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fread(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_READ_FILE;
			if (fseek(m_fp, nPos - nNewAppend, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fwrite(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_WRITE_FILE;

			nPos = nNextPos;
		}
		// clip the file.
		filetruncate(m_fp, nFileLen - nNewAppend);
	}

	return ERR_OK;
}

/*
	Test function for FileMoveEndData()
void TestFileMoveEndData()
{
	// FileMoveEndData
	FILE		*fp;
	fp = fopen("c:\\1.txt", "wb");
	if (!fp)
		return;

	for (int i = 0; i < 100; i++)
	{
		fprintf(fp, "%d", i % 10);
	}
	fclose(fp);

	fp = fopen("c:\\1.txt", "r+b");
	if (!fp)
		return;
	FileMoveEndData(fp, 90, 89);
	fclose(fp);
}
*/

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(ID3_HELPER)

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseLrcTag : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLrcTag);
	CPPUNIT_TEST(TestCopyAnsiStr);
	CPPUNIT_TEST(TestCopyStrByEncodingAndBom);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestCopyAnsiStr()
	{
#define		SZ_TEXT_TEST	"abc\0def\0"
		LPCSTR		text = SZ_TEXT_TEST;
		int			lenText = CountOf(SZ_TEXT_TEST);
		int			n;

		{
			string		str;
			n = CopyAnsiStr(str, text, lenText);
			CPPUNIT_ASSERT(n == 4);
			CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

			n = CopyAnsiStr(str, text, 2);
			CPPUNIT_ASSERT(n == 2);
			CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);
		}
	}

	void TestCopyStrByEncodingAndBom()
	{
#define		SZ_TEXT_TEST	"abc\0def\0"
		LPCSTR		text = SZ_TEXT_TEST;
		int			lenText = CountOf(SZ_TEXT_TEST);
		int			n;
		wstringex	wStr;
		ConvertStr(text, lenText, wStr);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(text, lenText);

		LPCSTR		vData[] = { (LPCSTR)text, (LPCSTR)wLyrics.c_str(),		(LPCSTR)wLyricsBe.c_str(),			(LPCSTR)utf8Lyrics.c_str(), };
		int			vLen[] = { lenText,	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };
		ID3v2EncType	vEncoding[] = { IET_ANSI,	IET_UCS2LE_BOM,						IET_UCS2BE_NO_BOM,				IET_UTF8, };
		int			vCopiedOffset[] = {0, 2, 2, 3 };

		for (int i = 0; i < CountOf(vData); i++)
		{
			{
				tstring		str;
				n = CopyStrByEncodingAndBom(str, vEncoding[i], vData[i], vLen[i]);
				CPPUNIT_ASSERT(n == 4 + vCopiedOffset[i]);
				CPPUNIT_ASSERT(_tcscmp(str.c_str(), _T("abc")) == 0);

				n = CopyStrByEncodingAndBom(str, vEncoding[i], vData[i], 2);
				CPPUNIT_ASSERT(n == 2 + vCopiedOffset[i]);
				CPPUNIT_ASSERT(_tcscmp(str.c_str(), _T("ab")) == 0);
			}
			{
				tstringex	str;
				n = CopyStrByEncodingAndBom(str, vEncoding[i], vData[i], vLen[i]);
				CPPUNIT_ASSERT(n == 4 + vCopiedOffset[i]);
				CPPUNIT_ASSERT(_tcscmp(str.c_str(), _T("abc")) == 0);

				n = CopyStrByEncodingAndBom(str, vEncoding[i], vData[i], 2);
				CPPUNIT_ASSERT(n == 2 + vCopiedOffset[i]);
				CPPUNIT_ASSERT(_tcscmp(str.c_str(), _T("ab")) == 0);
			}
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcTag);

#endif // _CPPUNIT_TEST
