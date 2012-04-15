#pragma once

DECLAR_CPPUNIT_TEST_REG(CFileEx)

class CFileEx
{
public:
	CFileEx();
	virtual ~CFileEx();

	int Open(LPCTSTR szFile, LPCTSTR szMode);
	void Close();

	int Seek(int offset, int origin = SEEK_CUR);
	long GetPos();

	bool IsEOF();

	size_t Read(LPVOID buf, size_t count) { return fread(buf, 1, count, m_fp); }
	size_t Write(LPCVOID buf, size_t count) { return fwrite(buf, 1, count, m_fp); }

	int ReadLine(string &line);
	int ReadTill(string &buf, char chTill);

	int ReadCount(LPVOID buf, size_t count);
	int WriteCount(LPVOID buf, size_t count);

	int WriteByte(byte v);
	int WriteInt16BE(int16 v);
	int WriteUInt16BE(uint16 v);
	int WriteInt32BE(int32 v);
	int WriteUInt32BE(uint32 v);

	int ReadByte(byte &v);
	int ReadInt16BE(int16 &v);
	int ReadUInt16BE(uint16 &v);
	int ReadInt32BE(int32 &v);
	int ReadUInt32BE(uint32 &v);

protected:
	FILE			*m_fp;

};
