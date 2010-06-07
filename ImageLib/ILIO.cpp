// ILIO.cpp: implementation of the CILIO class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ILIO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IILIO::IILIO()
{

}

IILIO::~IILIO()
{

}

//////////////////////////////////////////////////////////////////////////
CFileILIO::CFileILIO()
{
	m_fp = NULL;
}

CFileILIO::~CFileILIO()
{
	Close();
}

bool CFileILIO::Open(LPCTSTR szFile)
{
	Close();

	m_fp = _tfopen(szFile, _T("rb"));
	if (!m_fp)
		return false;

	return true;
}

uint32 CFileILIO::Read(void *buf, uint32 nSize)
{
	return fread(buf, 1, nSize, m_fp);
}

bool CFileILIO::Seek(int nOffset, int nOrigin)
{
	return fseek(m_fp, nOffset, nOrigin) == 0;
}

bool CFileILIO::GetSize(uint32 &nSize)
{
	uint32	pos = ftell(m_fp);
	fseek(m_fp, 0, SEEK_END);
	nSize = ftell(m_fp);
	fseek(m_fp, pos, SEEK_SET);
	return true;
}

uint32 CFileILIO::GetPos()
{
	return ftell(m_fp);
}

bool CFileILIO::IsEOF()
{
	return feof(m_fp) != 0;
}

void CFileILIO::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

CBuffILIO::CBuffILIO()
{
	m_buf = NULL;
}

CBuffILIO::~CBuffILIO()
{
}

bool CBuffILIO::Open(const void *buff, uint32 nLength)
{
	m_buf = (uint8*)buff;
	m_nLength = nLength;
	m_nPos = 0;

	return true;
}

uint32 CBuffILIO::Read(void *buf, uint32 nSize)
{
	if (m_nPos >= m_nLength)
		return 0;

	if (nSize + m_nPos > m_nLength)
		nSize = m_nLength - m_nPos;

	memcpy(buf, m_buf + m_nPos, nSize);
	m_nPos += nSize;

	return nSize;
}

bool CBuffILIO::Seek(int nOffset, int nOrigin)
{
	if (nOrigin == SEEK_SET)
		m_nPos = nOffset;
	else if (nOrigin == SEEK_CUR)
		m_nPos += nOffset;
	else if (nOrigin == SEEK_END)
		m_nPos = m_nLength + nOffset;

	if (m_nPos < 0)
		m_nPos = 0;
	else if (m_nPos > m_nLength)
		m_nPos = m_nLength;

	return true;
}

bool CBuffILIO::GetSize(uint32 &nSize)
{
	nSize = m_nLength;
	return true;
}

uint32 CBuffILIO::GetPos()
{
	return m_nPos;
}

bool CBuffILIO::IsEOF()
{
	return m_nPos >= m_nLength;
}

void CBuffILIO::Close()
{
	m_buf = NULL;
}
