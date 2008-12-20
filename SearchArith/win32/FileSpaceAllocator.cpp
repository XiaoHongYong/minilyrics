// FileSpaceAllocator.cpp: implementation of the CFileSpaceAllocator class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "../SearchArith.h"
#include "FileSpaceAllocator.h"

#define FI_FREE_RATE		 1.2
#ifdef _WIN32_WCE
#define FI_MINI_FREE		(1024 * 5)
#else
#define FI_MINI_FREE		(1024 * 10)
#endif

#define FSA_BEGIN_POS		sizeof(Head)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileSpaceAllocator::CFileSpaceAllocator()
{
	m_hFile = NULL;
	m_hFilemap = NULL;
	m_nFileSize = 0;
	m_nRemainSize = 0;
	m_lpData = NULL;
	m_lpFile = NULL;

	m_pHead = NULL;
}

CFileSpaceAllocator::~CFileSpaceAllocator()
{
	Close();
}

int CFileSpaceAllocator::Open(LPCTSTR szFile, int nRemainSize)
{
	DWORD		dwMiniSize;

	dwMiniSize = FI_MINI_FREE;

	m_nRemainSize = nRemainSize;
	m_strFile = szFile;

	if (IsFileExist(m_strFile.c_str()))
	{
#ifdef _WINCE_
		m_hFile = CreateFileForMapping(m_strFile.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		m_hFile = CreateFile(m_strFile.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
		ASSERT(m_hFile != INVALID_HANDLE_VALUE);
		if (m_lpFile == INVALID_HANDLE_VALUE)
			return ERR_OPEN_FILE;

		m_nFileSize = GetFileSize(m_hFile, NULL);
		if (m_nFileSize <= dwMiniSize)
			m_nFileSize = dwMiniSize;
	}
	else
	{
		m_nFileSize = dwMiniSize;
#ifdef _WINCE_
		m_hFile = CreateFileForMapping(m_strFile.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		m_hFile = CreateFile(m_strFile.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
		ASSERT(m_hFile != INVALID_HANDLE_VALUE);
		if (m_lpFile == INVALID_HANDLE_VALUE)
			return ERR_OPEN_FILE;
	}

	m_hFilemap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, m_nFileSize, NULL);
	ASSERT(m_hFilemap);
	if (!m_hFilemap)
	{
		CloseHandle(m_hFile);
		return ERR_CREATE_FILE_MAPPING;
	}

	m_lpFile = (LPBYTE)MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
	ASSERT(m_lpFile);
	if (!m_lpFile)
	{
		CloseHandle(m_hFile);
		CloseHandle(m_hFilemap);
		return ERR_MAP_VIEWOFFILE;
	}

	m_pHead = (Head *)(m_lpFile + nRemainSize);
	m_lpData = m_lpFile + nRemainSize;

	m_pHead->m_nLenMax = m_nFileSize - m_nRemainSize;

	return ERR_OK;
}

int CFileSpaceAllocator::PreAlloc(int nSize)
{
	if (m_pHead->m_nLen + nSize >= m_pHead->m_nLenMax)
	{
		//
		// 重新分配
		// DBGLOG0("Should Realloc"));
		// m_nFileSize += nSize;
		// m_nFileSize += sizeof(DWORD) - m_nFileSize % sizeof(DWORD);
		uint32	nNewSize = (uint32)(m_nFileSize * FI_FREE_RATE);
		if (nNewSize < m_nFileSize + nSize)
			nNewSize = m_nFileSize + nSize + 4;
		return ReSize(nNewSize);
	}

	return ERR_OK;
}

int CFileSpaceAllocator::ReSize(size_t nSize)
{
	if (!IsValid())
		return ERR_NOT_OPENED;

	// m_nFileSize = (DWORD)(m_nFileSize * FI_FREE_RATE);
	m_nFileSize = nSize + sizeof(int);
	m_nFileSize -= m_nFileSize % sizeof(int);
	if (m_nFileSize < FI_MINI_FREE)
		m_nFileSize = FI_MINI_FREE;

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
		return ERR_CREATE_FILE_MAPPING;
	}

	m_lpFile = (LPBYTE)MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
	ASSERT(m_lpFile);
	if (!m_lpFile)
	{
		CloseHandle(m_hFile);
		CloseHandle(m_hFilemap);
		m_lpFile = NULL;
		m_hFilemap = NULL;
		return ERR_MAP_VIEWOFFILE;
	}

	m_pHead = (Head *)(m_lpFile + m_nRemainSize);
	m_lpData = m_lpFile + m_nRemainSize;

	m_pHead->m_nLenMax = m_nFileSize - m_nRemainSize;

	return ERR_OK;
}

LPVOID CFileSpaceAllocator::Alloc(size_t nSize)
{
	DB_POS		pos;

	if (!IsValid())
		return NULL;

	if (m_pHead->m_nLen + nSize >= m_pHead->m_nLenMax)
	{
		//
		// 重新分配
		// static		int nCount = 0;
		// nCount++;
		// DBGLOG1("Realloc: %d"), nCount);
		uint32	nNewSize = (uint32)(m_nFileSize * FI_FREE_RATE);
		if (nNewSize < m_nFileSize + nSize)
			nNewSize = m_nFileSize + nSize + 4;
		if (ReSize(nNewSize) != ERR_OK)
			return NULL;
	}

	pos = m_pHead->m_nLen;
	m_pHead->m_nLen += nSize;

#ifdef _WINCE_
	// align mem with int32 按照int32对齐
	m_pHead->m_nLen = (m_pHead->m_nLen + sizeof(int32) - 1) & ~(0x3);
#endif

	return PosToAddr(pos);
}

int CFileSpaceAllocator::Free(DB_POS pos)
{
	return 0;
}

int CFileSpaceAllocator::Initialize()
{
	m_pHead->m_nLen = FSA_BEGIN_POS;
	m_pHead->m_posFree = POS_NULL;
	m_pHead->m_nLenMax = m_nFileSize - m_nRemainSize;

	return ERR_OK;
}

void CFileSpaceAllocator::Close()
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
