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
	m_nCapacity = 0;
	m_nRemainSize = 0;
	m_lpData = NULL;
	m_lpRemainDataPtr = NULL;

	m_pHead = NULL;
}

CFileSpaceAllocator::~CFileSpaceAllocator()
{
	Close();
}

int CFileSpaceAllocator::Open(LPCTSTR szFile, int nRemainSize)
{
	m_nRemainSize = nRemainSize;

	if (szFile == NULL || IsEmptyString(szFile))
	{
		m_nCapacity = FI_MINI_FREE;
		m_lpRemainDataPtr = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_nCapacity);
	}
	else
	{
		if (!m_memMapFile.Open(szFile, FI_MINI_FREE, false))
			return ERR_OPEN_FILE;
		m_lpRemainDataPtr = (LPBYTE)m_memMapFile.GetPtr();
		m_nCapacity = m_memMapFile.GetSize();
	}

	m_lpData = m_lpRemainDataPtr + m_nRemainSize;
	m_pHead = (Head *)m_lpData;

	m_pHead->m_nLenMax = m_nCapacity - m_nRemainSize;

	return ERR_OK;
}

int CFileSpaceAllocator::PreAlloc(int nSize)
{
	if (m_pHead->m_nLen + nSize >= m_pHead->m_nLenMax)
	{
		//
		// 重新分配
		// DBGLOG0("Should Realloc"));
		// m_nCapacity += nSize;
		// m_nCapacity += sizeof(DWORD) - m_nCapacity % sizeof(DWORD);
		int	nNewSize = (int)(m_nCapacity * FI_FREE_RATE);
		if (nNewSize < m_nCapacity + nSize)
			nNewSize = m_nCapacity + nSize + 4;
		return ReSize(nNewSize);
	}

	return ERR_OK;
}

int CFileSpaceAllocator::ReSize(size_t nSize)
{
	if (!IsValid())
		return ERR_NOT_OPENED;

	if (m_memMapFile.IsValid())
	{
		if (!m_memMapFile.Resize(nSize))
		{
			m_lpRemainDataPtr = m_lpData = NULL;
			return ERR_CREATE_FILE_MAPPING;
		}
		m_nCapacity = m_memMapFile.GetSize();
	}
	else
	{
		nSize = (nSize + sizeof(int)) & ~3;
		m_nCapacity = nSize;
		m_lpRemainDataPtr = (LPBYTE)HeapReAlloc(GetProcessHeap(), 0, m_lpRemainDataPtr, nSize);
	}

	m_lpData = m_lpRemainDataPtr + m_nRemainSize;
	m_pHead = (Head *)m_lpData;

	m_pHead->m_nLenMax = m_nCapacity - m_nRemainSize;

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
		uint32	nNewSize = (uint32)(m_nCapacity * FI_FREE_RATE);
		if (nNewSize < m_nCapacity + nSize)
			nNewSize = m_nCapacity + nSize + 4;
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
	m_pHead->m_nLenMax = m_nCapacity - m_nRemainSize;

	return ERR_OK;
}

void CFileSpaceAllocator::Close()
{
	if (m_memMapFile.IsValid())
		m_memMapFile.Close();
	else
		HeapFree(GetProcessHeap(), 0, m_lpRemainDataPtr);
	m_lpRemainDataPtr = NULL;
	m_nCapacity = 0;
	m_lpData = NULL;
	m_pHead = NULL;
}
