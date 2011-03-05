
#pragma once

DECLAR_CPPUNIT_TEST_REG(COnlyAllocator)

//
// Only provide allocator, and free memory one time on quit.
//
class COnlyAllocator
{
public:
	COnlyAllocator(size_t nBlockSize = 1024);
	~COnlyAllocator();

	LPVOID allocate(size_t _Count);

	LPSTR allocate(LPCSTR str);
	LPWSTR allocate(LPCWSTR str);

protected:
	size_t			m_nBlockSize;
	LPSTR			m_pCurBlock;
	size_t			m_nCurBlockUsed;
	vector<LPSTR>	m_vBlocks;

};
