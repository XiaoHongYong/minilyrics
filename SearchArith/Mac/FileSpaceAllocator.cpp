// FileSpaceAllocator.cpp: implementation of the CFileSpaceAllocator class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fileapi.h"
#include "../Error.h"
#include "FileSpaceAllocator.h"

#define FI_FREE_RATE		1.1
#define FI_MINI_FREE		(1024 * 100)

#define FSA_BEGIN_POS		sizeof(Head)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileSpaceAllocator::CFileSpaceAllocator()
{
	m_file = -1;
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
		struct stat		filestat;

		m_file = open(m_strFile.c_str(), O_RDWR);
		if (m_file < 0)
			return false;

		memset(&filestat, 0, sizeof(filestat));
		if (fstat(m_file, &filestat) < 0)
		{
			Close();
			return false;
		}

		m_nFileSize = filestat.st_size;
		if (m_nFileSize < dwMiniSize)
		{
			m_nFileSize = dwMiniSize;
			if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1)
			{
				Close();
				return false;
			}

			if (write(m_file, "", 1) != 1)
			{
				Close();
				return false;
			}
		}
	}
	else
	{
		m_file = open(m_strFile.c_str(), O_RDWR | O_CREAT | O_TRUNC);
		if (m_file < 0)
			return false;

		m_nFileSize = dwMiniSize;
		if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1)
		{
			Close();
			return false;
		}

		if (write(m_file, "", 1) != 1)
		{
			Close();
			return false;
		}
	}

	m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, m_file, 0);
	if (m_lpFile == (LPVOID)-1)
	{
		m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, m_file, 0);
		if (m_lpFile == (LPVOID)-1)
		{
		printf("error: %d, %s", errno, strerror(errno));
		m_lpFile = NULL;
		Close();
		return false;
		}
	}

	m_pHead = (Head *)(((char *)m_lpFile) + nRemainSize);
	m_lpData = ((char *)m_lpFile) + nRemainSize;

	m_pHead->m_nLenMax = m_nFileSize - m_nRemainSize;

	return TRUE;
}

void CFileSpaceAllocator::PreAlloc(int nSize)
{
	if (m_pHead->m_nLen + nSize >= m_pHead->m_nLenMax)
	{
		//
		// 重新分配
		// DBGLOG0("Should Realloc"));
		m_nFileSize += nSize;
		m_nFileSize += sizeof(DWORD) - m_nFileSize % sizeof(DWORD);
		ReSize(m_nFileSize);
	}
}

int CFileSpaceAllocator::ReSize(size_t nSize)
{
	if (!IsValid())
		return FALSE;

	DWORD		nNewSize;

	nNewSize = nSize + sizeof(int);
	nNewSize -= nNewSize % sizeof(int);
	if (nNewSize < FI_MINI_FREE)
		nNewSize = FI_MINI_FREE;

	if (nNewSize > m_nFileSize)
	{
		m_nFileSize = nNewSize;
		if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1)
		{
			Close();
			return false;
		}

		if (write(m_file, "", 1) != 1)
		{
			Close();
			return false;
		}
	}
	else
		return true;

	if (m_lpFile)
		munmap(m_lpFile, m_nFileSize);

	m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, m_file, 0);
	if (m_lpFile == (LPVOID)-1)
	{
		m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, m_file, 0);
		if (m_lpFile == (LPVOID)-1)
		{
		printf("error: %d, %s", errno, strerror(errno));
		m_lpFile = NULL;
		Close();
		return false;
		}
	}

	m_pHead = (Head *)(((char *)m_lpFile) + m_nRemainSize);
	m_lpData = ((char *)m_lpFile) + m_nRemainSize;

	m_pHead->m_nLenMax = m_nFileSize - m_nRemainSize;

	return TRUE;
}

LPVOID CFileSpaceAllocator::Alloc(size_t nSize)
{
	DB_POS		pos;

	static		int nCount = 0;
	nCount++;

	if (m_pHead->m_nLen + nSize >= m_pHead->m_nLenMax)
	{
		//
		// 重新分配
		// DBGLOG1("Realloc: %d"), nCount);
		ReSize(m_nFileSize + (uint32)(m_nFileSize * FI_FREE_RATE));
	}

	pos = m_pHead->m_nLen;
	m_pHead->m_nLen += nSize;

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

	return TRUE;
}

void CFileSpaceAllocator::Close()
{
	if (m_lpFile)
	{
		munmap(m_lpFile, m_nFileSize);
		m_lpFile = NULL;
	}

	if (m_file >= 0)
	{
		close(m_file);
		m_file = -1;
	}
}
