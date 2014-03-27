#include "StdAfx.h"
#include "base.h"
#include "TextFile.h"

CTextFile::CTextFile()
{
	m_offset = 0;
	m_fp = NULL;
	m_fileEncoding = FE_ANSI;
	m_encoding = ED_SYSDEF;
}

CTextFile::~CTextFile()
{
	Close();
}

int CTextFile::Open(LPCTSTR szFile, OpenMode om, CHAR_ENCODING encoding)
{
	Close();

	LPCTSTR			szMode = _T("rb");

	if (om == OM_WRITE)
		szMode = _T("wb");
	else if (om == OM_APPEND)
		szMode = _T("a+b");

	m_fp = _tfopen(szFile, szMode);
	if (!m_fp)
	{
		SetCustomErrorDesc(CStrPrintf(_T("%s: %s"), (LPCTSTR)OSError(), szFile).c_str());
		return ERR_CUSTOM_ERROR;
	}

	m_offset = 0;
	m_encoding =  encoding;

	bool bSaveBom = false;

	if (om == OM_WRITE)
	{
		m_fileEncoding = CharEncodingToFileEncoding(encoding);
		bSaveBom = true;
	}
	else if (om == OM_READ)
	{
		OpenFileToTString(szFile, m_fileData, m_encoding);
	}
	else
	{
		// Open for append
		char	buf[4];
		int len = fread(buf, 1, 3, m_fp);
		int offset = 0;
		if (len >= 2)
		{
			m_fileEncoding = DetectFileEncoding(buf, 3, offset);
		}
		else
		{
			m_fileEncoding = CharEncodingToFileEncoding(encoding);
			bSaveBom = true;
		}
		fseek(m_fp, offset, SEEK_SET);
	}

	if (bSaveBom)
	{
		LPCSTR bom = GetFileEncodingBom(m_fileEncoding);
		fwrite(bom, 1, strlen(bom), m_fp);
	}

	return ERR_OK;
}

void CTextFile::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	m_fileData.clear();
}

bool CTextFile::ReadLine(tstring &strText)
{
	LPCTSTR begin = m_fileData.c_str() + m_offset;
	LPCTSTR end = m_fileData.c_str() + m_fileData.size();
	LPCTSTR p = begin;

	strText.clear();

	while (p < end)
	{
		if (*p == '\r' || *p == '\n')
		{
			// Found new line position
			strText.append(begin, (size_t)(p - begin));
			if (*p == '\r' && p[1] == '\n')
				p++;
			p++;

			m_offset = (size_t)(p - m_fileData.c_str());
			return true;
		}
		p++;
	}

	// To the end of file
	if (m_offset == m_fileData.size())
		return false;

	strText.append(begin);
	m_offset = m_fileData.size();

	return true;
}

bool CTextFile::WriteLine(LPCTSTR szText, size_t nLenText)
{
	if (!Write(szText, nLenText))
		return false;

	// Write new line
	return Write(_T("\r\n"));
}

bool CTextFile::Write(LPCTSTR szText, size_t nLenText)
{
	if (nLenText == -1)
		nLenText = _tcslen(szText);

	int nWritten = 0;
	if (m_fileEncoding == FE_UCS2)
	{
#ifdef UNICODE
		nWritten = fwrite(szText, sizeof(WCHAR), nLenText, m_fp);
		if (nWritten != nLenText)
			return false;
#else
		wstring_t	strUCS2;
		TStringToUCS2(szText, nLenText, strUCS2, m_encoding);

		nWritten = fwrite(strUCS2.c_str(), sizeof(WCHAR), strUCS2.size(), m_fp);
		if (nWritten != strUCS2.size())
			return false;
#endif
	}
	else if (m_fileEncoding == FE_UCS2_BE)
	{
		wstringex	strUCS2;
		TStringToUCS2(szText, nLenText, strUCS2, m_encoding);
		UCS2LEToBE(strUCS2.data(), strUCS2.size());

		nWritten = fwrite(strUCS2.c_str(), sizeof(WCHAR), strUCS2.size(), m_fp);
		if (nWritten != strUCS2.size())
			return false;
	}
	else if (m_fileEncoding == FE_UTF8)
	{
#ifdef UTF8
		nWritten = fwrite(szText, sizeof(char), nLenText, m_fp);
		if (nWritten != nLenText)
			return false;
#else
		stringex	strUtf8;
		TStringToUTF8(szText, nLenText, strUtf8, m_encoding);

		nWritten = fwrite(strUtf8.c_str(), sizeof(char), strUtf8.size(), m_fp);
		if (nWritten != strUtf8.size())
			return false;
#endif
	}
	else
	{
		stringex	strMbcs;
		TStringToMBCS(szText, nLenText, strMbcs, m_encoding);

		nWritten = fwrite(strMbcs.c_str(), sizeof(char), strMbcs.size(), m_fp);
		if (nWritten != strMbcs.size())
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CTextFile)

class CTestCaseCTextFile : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCTextFile);
	CPPUNIT_TEST(TestReadLine);
	CPPUNIT_TEST(TestAll);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestReadLine()
	{
		CTextFile	file;

		file.m_fileData = _T("l1\nl2\rl3\r\nl4");

		tstring line;

		CPPUNIT_ASSERT(file.ReadLine(line));
		CPPUNIT_ASSERT(line == _T("l1"));

		CPPUNIT_ASSERT(file.ReadLine(line));
		CPPUNIT_ASSERT(line == _T("l2"));

		CPPUNIT_ASSERT(file.ReadLine(line));
		CPPUNIT_ASSERT(line == _T("l3"));

		CPPUNIT_ASSERT(file.ReadLine(line));
		CPPUNIT_ASSERT(line == _T("l4"));
	}

	void TestAll()
	{
		CTextFile	file;

		tstring szFile = GetUnitTestFolder() + _T("testTextFile.txt");

		CHAR_ENCODING encodings[] = { ED_UTF8, ED_UNICODE, ED_SYSDEF };

		for (int i = 0; i < CountOf(encodings); i++)
		{
			int nRet = file.Open(szFile.c_str(), CTextFile::OM_WRITE, encodings[i]);
			CPPUNIT_ASSERT(nRet == ERR_OK);

			CPPUNIT_ASSERT(file.WriteLine(_T("l1")));
			CPPUNIT_ASSERT(file.WriteLine(_T("l2")));
			CPPUNIT_ASSERT(file.WriteLine(_T("l3")));

			file.Close();

			nRet = file.Open(szFile.c_str(), CTextFile::OM_READ, ED_SYSDEF);
			CPPUNIT_ASSERT(nRet == ERR_OK);

			tstring line;

			CPPUNIT_ASSERT(file.ReadLine(line));
			CPPUNIT_ASSERT(line == _T("l1"));

			CPPUNIT_ASSERT(file.ReadLine(line));
			CPPUNIT_ASSERT(line == _T("l2"));

			CPPUNIT_ASSERT(file.ReadLine(line));
			CPPUNIT_ASSERT(line == _T("l3"));
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCTextFile);

#endif // _CPPUNIT_TEST
