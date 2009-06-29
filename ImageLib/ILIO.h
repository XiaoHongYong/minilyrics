// ILIO.h: interface for the CILIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILIO_H__35C6A017_5F2B_45CB_927E_E6537F270A30__INCLUDED_)
#define AFX_ILIO_H__35C6A017_5F2B_45CB_927E_E6537F270A30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IILIO  
{
public:
	IILIO();
	virtual ~IILIO();

	virtual uint32 Read(void *buf, uint32 nSize) = 0;
	virtual uint32 Write(const void *buf, uint32 nSize) { return nSize; }
	// nOrigin: SEEK_SET, SEEK_CUR, SEEK_END
	virtual bool Seek(int nOffset, int nOrigin) = 0;
	virtual bool GetSize(uint32 &nSize) = 0;
	virtual uint32 GetPos() = 0;

	virtual bool IsEOF() = 0;

	virtual void Close() = 0;

};

class CFileILIO : public IILIO
{
public:
	CFileILIO();
	~CFileILIO();

	bool Open(LPCTSTR szFile);

public:
	virtual uint32 Read(void *buf, uint32 nSize);
	virtual bool Seek(int nOffset, int nOrigin);
	virtual bool GetSize(uint32 &nSize);
	virtual uint32 GetPos();

	virtual bool IsEOF();

	virtual void Close();

protected:
	FILE		*m_fp;

};

class CBuffILIO : public IILIO
{
public:
	CBuffILIO();
	~CBuffILIO();

	bool Open(const void *buff, uint32 nLength);

public:
	virtual uint32 Read(void *buf, uint32 nSize);
	virtual bool Seek(int nOffset, int nOrigin);
	virtual bool GetSize(uint32 &nSize);
	virtual uint32 GetPos();

	virtual bool IsEOF();

	virtual void Close();

protected:
	uint8		*m_buf;
	uint32		m_nLength;
	size_t			m_nPos;

};

#endif // !defined(AFX_ILIO_H__35C6A017_5F2B_45CB_927E_E6537F270A30__INCLUDED_)
