// FileSpaceAllocator.h: interface for the CFileSpaceAllocator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILESPACEALLOCATOR_H__257B3499_7A6E_4DD0_8067_DB30E42AA407__INCLUDED_)
#define AFX_FILESPACEALLOCATOR_H__257B3499_7A6E_4DD0_8067_DB30E42AA407__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "IAllocator.h"
#include "avltree.h"
#include "assert.h"

#include <sys/mman.h>

class CFileSpaceAllocator// : public IAllocator  
{
public:
	void Close();
	int ReSize(size_t nSize);
	int Open(LPCTSTR szFile, int nRemainSize);
	int Initialize();

	void PreAlloc(int nSize);

	LPVOID Alloc(size_t nSize);
	int Free(DB_POS pos);

	inline BOOL IsValid() { return m_lpFile != NULL; }

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
		return (LPBYTE)m_lpFile;
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

	tstring		m_strFile;			// 文件名

	int			m_file;
	DWORD		m_nFileSize;		// 文件大小
	int			m_nRemainSize;
	LPVOID		m_lpData;			// 数据段的内容，不包括文件头
	LPVOID		m_lpFile;			// 文件映射之后的文件内容

	Head		*m_pHead;

};

#endif // !defined(AFX_FILESPACEALLOCATOR_H__257B3499_7A6E_4DD0_8067_DB30E42AA407__INCLUDED_)
