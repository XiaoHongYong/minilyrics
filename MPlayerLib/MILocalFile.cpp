// MILocalFile.cpp: implementation of the CMILocalFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMPlayer.h"
#include "MILocalFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMILocalFile::CMILocalFile()
{
	OBJ_REFERENCE_INIT;

	m_fp = NULL;
}

CMILocalFile::~CMILocalFile()
{
	Close();
}

MLRESULT CMILocalFile::Open(LPCXSTR szSourceMedia)
{
	m_fp = _tfopen(szSourceMedia, _T("rb"));
	if (m_fp == NULL)
	{
		if (!IsFileExist(szSourceMedia))
			return ERR_MI_NOT_FOUND;
		return ERR_MI_OPEN_SRC;
	}

	m_strFile = szSourceMedia;

	return ERR_OK;
}

uint32 CMILocalFile::Read(LPVOID lpBuffer, uint32 dwSize)
{
	return fread(lpBuffer, 1, dwSize, m_fp);
}

MLRESULT CMILocalFile::Seek(uint32 dwOffset, int nOrigin)
{
	if (fseek(m_fp, dwOffset, nOrigin) == 0)
		return ERR_OK;
	else
		return ERR_MI_SEEK;
}

MLRESULT CMILocalFile::GetSize(uint32 &dwSize)
{
	uint32		dwPos;

	dwPos = ftell(m_fp);
	if (fseek(m_fp, 0, SEEK_END) == 0)
	{
		dwSize = ftell(m_fp);
		fseek(m_fp, dwPos, SEEK_SET);
		return ERR_OK;
	}
	else
		return ERR_MI_SEEK;
}

uint32 CMILocalFile::GetPos()
{
	return ftell(m_fp);
}

bool CMILocalFile::IsEOF()
{
	if (feof(m_fp))
		return true;
	else
		return false;
}

bool CMILocalFile::IsError()
{
	return Tobool(ferror(m_fp));
}

void CMILocalFile::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

LPCXSTR CMILocalFile::GetSource()
{
	return m_strFile.c_str();
}
