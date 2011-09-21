#include "stdafx.h"
#include "OnlyAllocator.h"
#include "base.h"

COnlyAllocator::COnlyAllocator(size_t nBlockSize)
{
	m_pCurBlock = NULL;
	m_nBlockSize = nBlockSize;
	m_nCurBlockUsed = 0;
}

COnlyAllocator::~COnlyAllocator()
{
	for (uint i = 0; i < m_vBlocks.size(); i++)
		free(m_vBlocks[i]);
	if (m_pCurBlock)
		free(m_pCurBlock);
}

LPVOID COnlyAllocator::allocate(size_t _Count)
{
	// Align start ptr.
	if (_Count >= 2)
	{
		if (m_nCurBlockUsed % 4 != 0)
			m_nCurBlockUsed += 4 - m_nCurBlockUsed % 4;
	}

	if (m_pCurBlock == NULL || m_nCurBlockUsed + _Count >= m_nBlockSize)
	{
		if (_Count >= (m_nBlockSize >> 2))
		{
			LPSTR p = (LPSTR)malloc(_Count);
			m_vBlocks.push_back(p);
			return p;
		}
		if (m_pCurBlock)
			m_vBlocks.push_back(m_pCurBlock);
		m_pCurBlock = (LPSTR)malloc(m_nBlockSize);
		m_nCurBlockUsed = 0;
	}

	LPSTR p = m_pCurBlock + m_nCurBlockUsed;
	m_nCurBlockUsed += _Count;
	ASSERT(m_nCurBlockUsed <= m_nBlockSize);
	ASSERT(p + _Count <= m_pCurBlock + m_nCurBlockUsed);

	return p;
}

LPSTR COnlyAllocator::allocate(LPCSTR str)
{
	size_t n = strlen(str) + 1;
	LPSTR p = (LPSTR)allocate(n);
	memcpy(p, str, n);
	return p;
}

LPWSTR COnlyAllocator::allocate(LPCWSTR str)
{
	size_t n = sizeof(WCHAR) * (wcslen(str) + 1);
	LPWSTR p = (LPWSTR)allocate(n);
	memcpy(p, str, n);
	return p;
}


//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(COnlyAllocator)

class CTestCaseCOnlyAllocator : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCOnlyAllocator);
	CPPUNIT_TEST(TestOnlyAllocator);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestOnlyAllocator()
	{
		{
			COnlyAllocator		allocator;
		}
		{
			COnlyAllocator		allocator;
			allocator.allocate("11232232");
			allocator.allocate("11232232");
			allocator.allocate("11232232");
			allocator.allocate("11232232");
			allocator.allocate("11232232");
		}

		COnlyAllocator		allocator;
		LPCSTR	str[] = { ";alkge", "sldjglea", "" };
		LPCWSTR	strw[] = { L";alkge", L"sldjglea", L"" };

		LPCSTR	astr[CountOf(str)];
		LPCWSTR	astrw[CountOf(str)];

		allocator.allocate(1024);

		ASSERT(CountOf(str) == CountOf(strw));
		for (int i = 0; i < CountOf(str); i++)
		{
			astr[i] = allocator.allocate(str[i]);
			astrw[i] = allocator.allocate(strw[i]);
		}

		allocator.allocate(512);
		allocator.allocate(64);

		for (int i = 0; i < CountOf(str); i++)
		{
			CPPUNIT_ASSERT(strcmp(astr[i], str[i]) == 0);
			CPPUNIT_ASSERT(wcscmp(astrw[i], strw[i]) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCOnlyAllocator);

#endif // _CPPUNIT_TEST
