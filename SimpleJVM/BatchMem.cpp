#include "StdAfx.h"
#include "BatchMem.h"

#define BLOCK_HEAD_SIZE		sizeof(int)
#define ALIGN_SIZE			sizeof(int)

CBatchMem::CBatchMem(int nBlockSize)
{
	m_nBlockSize = nBlockSize - BLOCK_HEAD_SIZE;
	m_pCurBlock = NULL;
}

CBatchMem::~CBatchMem(void)
{
	FreeAll();
}

LPVOID CBatchMem::Alloc(size_t nSize)
{
	if (m_pCurBlock == NULL)
	{
		m_pCurBlock = (Block *)new char[m_nBlockSize + BLOCK_HEAD_SIZE];
		m_pCurBlock->nUsedSize = BLOCK_HEAD_SIZE;
	}

	if (m_pCurBlock->nUsedSize + nSize >= m_nBlockSize)
	{
		if (nSize > m_nBlockSize)
		{
			char	*p = new char[nSize];
			m_listBlocks.push_back(p);
			return p;
		}
		m_listBlocks.push_back((char *)m_pCurBlock);
		m_pCurBlock = (Block*)(new char[m_nBlockSize + BLOCK_HEAD_SIZE]);
		m_pCurBlock->nUsedSize = BLOCK_HEAD_SIZE;
	}

	char	*p = m_pCurBlock->mem + m_pCurBlock->nUsedSize;
	m_pCurBlock->nUsedSize = (m_pCurBlock->nUsedSize + nSize + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);

	return p;
}

LPSTR CBatchMem::DupStr(LPCSTR szString)
{
	size_t	len = strlen(szString) + 1;
	char	*p = (char *)Alloc(len);
	ASSERT(p);
	memcpy(p, szString, len);
	return p;
}

void CBatchMem::FreeAll()
{
	if (m_pCurBlock)
	{
		delete[] (char *)m_pCurBlock;
		m_pCurBlock = NULL;
	}

	ListBlocks::iterator it;
	for (it = m_listBlocks.begin(); it != m_listBlocks.end(); ++it)
	{
		delete [] (*it);
	}
	m_listBlocks.clear();
}
