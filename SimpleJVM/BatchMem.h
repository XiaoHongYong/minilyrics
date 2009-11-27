#pragma once

class CBatchMem
{
public:
	CBatchMem(int nBlockSize = 1024 * 8);
	virtual ~CBatchMem(void);

	LPVOID Alloc(size_t nSize);
	LPSTR DupStr(LPCSTR szString);

	void FreeAll();

protected:
	struct Block {
		size_t		nUsedSize;
		char		mem[sizeof(int)];
	};
	typedef list<char *>	ListBlocks;

	size_t					m_nBlockSize;

	ListBlocks				m_listBlocks;

	Block					*m_pCurBlock;

};
