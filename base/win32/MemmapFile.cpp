// MemmapFile.cpp: implementation of the CMemmapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base.h"
#include "fileapi.h"
#include "MemmapFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemmapFile::CMemmapFile()
{
	m_hFile = NULL;
	m_hFilemap = NULL;
	m_nFileSize = 0;
	m_lpFile = NULL;
}

CMemmapFile::~CMemmapFile()
{
	Close();
}

bool CMemmapFile::Open(LPCTSTR szFile, int nSizeMin, bool bMustExist)
{
	if (IsFileExist(szFile))
	{
		m_hFile = CreateFile(szFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		ASSERT(m_hFile != INVALID_HANDLE_VALUE);
		if (m_lpFile == INVALID_HANDLE_VALUE)
			return false;

		m_nFileSize = GetFileSize(m_hFile, NULL);
		if (m_nFileSize <= nSizeMin)
			m_nFileSize = nSizeMin;
	}
	else
	{
		if (bMustExist)
			return false;

		m_nFileSize = nSizeMin;
		m_hFile = CreateFile(szFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		ASSERT(m_hFile != INVALID_HANDLE_VALUE);
		if (m_lpFile == INVALID_HANDLE_VALUE)
			return false;
	}

	m_hFilemap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, m_nFileSize, NULL);
	ASSERT(m_hFilemap);
	if (!m_hFilemap)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return false;
	}

	m_lpFile = (LPBYTE)MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
	ASSERT(m_lpFile);
	if (!m_lpFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		CloseHandle(m_hFilemap);
		m_hFilemap = NULL;
		return false;
	}

	return true;
}

bool CMemmapFile::Resize(size_t nSize)
{
	if (m_lpFile == NULL)
		return false;

	if (nSize < 1024)
		nSize = 1024;

	// m_nFileSize = (DWORD)(m_nFileSize * FI_FREE_RATE);
	m_nFileSize = nSize + sizeof(int);
	m_nFileSize -= m_nFileSize % sizeof(int);

	if (m_lpFile)
	{
		UnmapViewOfFile(m_lpFile);
		m_lpFile = NULL;
	}

	if (m_hFilemap)
	{
		CloseHandle(m_hFilemap);
		m_hFilemap = NULL;
	}

	m_hFilemap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, m_nFileSize, NULL);
	ASSERT(m_hFilemap);
	if (!m_hFilemap)
	{
		CloseHandle(m_hFile);
		m_lpFile = NULL;
		return false;
	}

	m_lpFile = (LPBYTE)MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
	ASSERT(m_lpFile);
	if (!m_lpFile)
	{
		CloseHandle(m_hFile);
		CloseHandle(m_hFilemap);
		m_lpFile = NULL;
		m_hFilemap = NULL;
		return false;
	}

	return true;
}

void CMemmapFile::Close()
{
	if (m_lpFile)
	{
		UnmapViewOfFile(m_lpFile);
		m_lpFile = NULL;
	}

	if (m_hFilemap)
	{
		CloseHandle(m_hFilemap);
		m_hFilemap = NULL;
	}

	if (m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}
