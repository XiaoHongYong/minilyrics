#pragma once

#include "../base/win32/MemmapFile.h"

class CFileSpaceAllocator// : public IAllocator  
{
public:
	int Open(LPCTSTR szFile, int nRemainSize);
	void Close();

	int ReSize(size_t nSize);
	int Initialize();

	int PreAlloc(int nSize);

	LPVOID Alloc(size_t nSize);
	int Free(DB_POS pos);

	inline BOOL IsValid() { return m_lpRemainDataPtr != NULL; }

	inline LPVOID PosToAddr(DB_POS pos)
	{
		assert(pos != POS_NULL);
		assert(pos < m_pHead->m_nLenMax);
		return (LPVOID)((LPBYTE)m_lpData + pos);
	}

	inline DB_POS AddrToPos(LPVOID addr)
	{
		assert((DB_POS)((LPBYTE)addr - (LPBYTE)m_lpData) < m_pHead->m_nLenMax);
		return (DB_POS)((LPBYTE)addr - (LPBYTE)m_lpData);
	}

	inline LPBYTE GetDataPtr()
	{
		return (LPBYTE)m_lpData;
	}

	inline LPBYTE GetRemainDataPtr()
	{
		return (LPBYTE)m_lpRemainDataPtr;
	}

	CFileSpaceAllocator();
	virtual ~CFileSpaceAllocator();

protected:

protected:
	struct Head
	{
		DB_POS			m_posFree;		// 空闲节点
		unsigned int	m_nLenMax;		// 最大空间
		unsigned int	m_nLen;			// 已使用的空间
	};

	CMemmapFile	m_memMapFile;

	LPBYTE		m_lpData;
	LPBYTE		m_lpRemainDataPtr;
	int			m_nRemainSize;

	int			m_nCapacity;

	Head		*m_pHead;

};
