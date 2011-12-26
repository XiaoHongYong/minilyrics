#pragma once

DECLAR_CPPUNIT_TEST_REG(CTextFile)

class CTextFile
{
public:
	CTextFile();
	virtual ~CTextFile();

	enum OpenMode
	{
		OM_READ,
		OM_WRITE,
		OM_APPEND,
	};

	// This will open all the file data to memory at one time.
	int Open(LPCTSTR szFile, OpenMode om, CHAR_ENCODING encoding);
	void Close();

	bool ReadLine(tstring &strText);
	bool WriteLine(LPCTSTR szText, size_t nLenText = -1);
	bool Write(LPCTSTR szText, size_t nLenText = -1);

protected:
	FILE_ENCODING		m_fileEncoding;
	CHAR_ENCODING		m_encoding;
	FILE				*m_fp;

	tstringex			m_fileData;
	size_t				m_offset;

	friend class CTestCaseCTextFile;

};
