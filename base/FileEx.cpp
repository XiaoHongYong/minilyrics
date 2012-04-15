#include "StdAfx.h"
#include "base.h"
#include "FileEx.h"

CFileEx::CFileEx()
{
	m_fp = NULL;
}

CFileEx::~CFileEx()
{
	Close();
}

int CFileEx::Open(LPCTSTR szFile, LPCTSTR szMode)
{
	Close();

	m_fp = _tfopen(szFile, szMode);
	if (!m_fp)
	{
		SetCustomErrorDesc(CStrPrintf(_T("%s: %s"), (LPCTSTR)OSError(), szFile).c_str());
		return ERR_CUSTOM_ERROR;
	}

	return ERR_OK;
}

void CFileEx::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

int CFileEx::Seek(int offset, int origin)
{
	if (fseek(m_fp, offset, origin) == 0)
		return ERR_OK;
	return ERR_SEEK_FILE;
}

long CFileEx::GetPos()
{
	return ftell(m_fp);
}

bool CFileEx::IsEOF()
{
	return feof(m_fp) != 0;
}

int CFileEx::ReadLine(string &line)
{
	return ReadTill(line, '\n');
}

int CFileEx::ReadTill(string &buf, char chTill)
{
	int c = fgetc(m_fp);
	if (c == EOF)
		return ERR_EOF;

	do
	{
		if (c == chTill)
			break;
		buf.append(1, (char)c);
		c = fgetc(m_fp);
	}
	while (c != EOF);

	return ERR_OK;
}

int CFileEx::ReadCount(LPVOID buf, size_t count)
{
	size_t n = fread(buf, 1, count, m_fp);
	if (n != count)
		return ERR_READ_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteCount(LPVOID buf, size_t count)
{
	size_t n = fwrite(buf, 1, count, m_fp);
	if (n != count)
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteByte(byte v)
{
	size_t n = fwrite(&v, 1, 1, m_fp);
	if (n != 1)
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteInt16BE(int16  v)
{
	byte	buf[2];
	BigEndianFromUInt16(v, buf);

	size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteUInt16BE(uint16  v)
{
	byte	buf[2];
	BigEndianFromUInt16(v, buf);

	size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteInt32BE(int32 v)
{
	byte	buf[4];
	BigEndianFromUInt32(v, buf);

	size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::WriteUInt32BE(uint32 v)
{
	byte	buf[4];
	BigEndianFromUInt32(v, buf);

	size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_WRITE_FILE;
	else
		return ERR_OK;
}

int CFileEx::ReadByte(byte &v)
{
	size_t n = fread(&v, 1, 1, m_fp);
	if (n != 1)
		return ERR_READ_FILE;
	else
		return ERR_OK;
}

int CFileEx::ReadInt16BE(int16 &v)
{
	byte buf[2];

	size_t n = fread(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_READ_FILE;

	v = (int)BigEndianToUInt16(buf);
	return ERR_OK;
}

int CFileEx::ReadUInt16BE(uint16 &v)
{
	byte buf[2];

	size_t n = fread(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_READ_FILE;

	v = (int)BigEndianToUInt16(buf);
	return ERR_OK;
}

int CFileEx::ReadInt32BE(int32 &v)
{
	byte buf[4];

	size_t n = fread(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_READ_FILE;

	v = (int)BigEndianToUInt32(buf);
	return ERR_OK;
}

int CFileEx::ReadUInt32BE(uint32 &v)
{
	byte buf[4];

	size_t n = fread(&buf, 1, CountOf(buf), m_fp);
	if (n != CountOf(buf))
		return ERR_READ_FILE;

	v = (int)BigEndianToUInt32(buf);
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CFileEx)

class CTestCaseCFileEx : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCFileEx);
	CPPUNIT_TEST(TestReadWrite);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestReadWrite()
	{
		CFileEx	f;
		tstring file = DirStringJoint(GetUnitTestFolder().c_str(), _T("test_FileEx.bin"));

		// Write
		int n = f.Open(file.c_str(), _T("wb"));
		CPPUNIT_ASSERT(n == ERR_OK);

		size_t count = f.Write("a", 1);
		CPPUNIT_ASSERT(count == 1);

		n = f.WriteByte(1);
		CPPUNIT_ASSERT(n == ERR_OK);

		n = f.WriteInt16BE(258);
		CPPUNIT_ASSERT(n == ERR_OK);

		n = f.WriteInt16BE((uint16)65535);
		CPPUNIT_ASSERT(n == ERR_OK);

		n = f.WriteInt32BE(3688823);
		CPPUNIT_ASSERT(n == ERR_OK);

		n = f.WriteUInt32BE(0xFFFFFFFF);
		CPPUNIT_ASSERT(n == ERR_OK);

		n = f.WriteCount("ab", 2);
		CPPUNIT_ASSERT(n == ERR_OK);

		const char *s = "dde\r\n";
		n = f.Write(s, strlen(s));

		f.Close();


		// Read
		n = f.Open(file.c_str(), _T("rb"));
		CPPUNIT_ASSERT(n == ERR_OK);

		byte buf[64];
		count = f.Read(buf, 1);
		CPPUNIT_ASSERT(count == 1);
		CPPUNIT_ASSERT(buf[0] == 'a');

		byte vb;
		n = f.ReadByte(vb);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(vb == 1);

		short vs;
		n = f.ReadInt16BE(vs);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(vs == 258);
		n = f.ReadInt16BE(vs);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT((uint16)vs == 65535);

		int vn;
		n = f.ReadInt32BE(vn);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(vn == 3688823);

		uint32 vu;
		n = f.ReadUInt32BE(vu);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(vu == 0xFFFFFFFF);

		n = f.ReadCount(buf, 2);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(buf[0] == 'a' && buf[1] == 'b');

		string str;
		n = f.ReadLine(str);
		CPPUNIT_ASSERT(n == ERR_OK);
		CPPUNIT_ASSERT(str == "dde\r");

		f.Close();
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCFileEx);

#endif // _CPPUNIT_TEST
