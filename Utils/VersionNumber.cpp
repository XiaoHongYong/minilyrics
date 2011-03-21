#include "stdafx.h"
#include "VersionNumber.h"

VersionNumber VersionNumMake(int nMajor, int nMinor, int nBuild)
{
	ASSERT(nMajor <= 0xFF && nMinor <= 0xFF && nBuild <= 0xFFFF);

	return MAKELONG(nBuild, MAKEWORD(nMinor, nMajor));
}

void VersionNumSplit(VersionNumber ver, int &nMajor, int &nMinor, int &nBuild)
{
	WORD wHigh = HIWORD(ver);
	nMajor = HIBYTE(wHigh);
	nMinor = LOBYTE(wHigh);
	nBuild = LOWORD(ver);
}

VersionNumber VersionNumFrom(LPCTSTR szVersion)
{
	int nVerMajor, nVerMinor, nVerBuild;
	int nRet = _stscanf(szVersion, _T("%d.%d.%d"), &nVerMajor, &nVerMinor, &nVerBuild);
	if (nRet == 3)
		return VersionNumMake(nVerMajor, nVerMinor, nVerBuild);
	return 0;
}

tstring VersionNumToStr(VersionNumber verNum)
{
	int nVerMajor, nVerMinor, nVerBuild;
	VersionNumSplit(verNum, nVerMajor, nVerMinor, nVerBuild);
	return CStrPrintf(_T("%d.%d.%d"), nVerMajor, nVerMinor, nVerBuild).c_str();
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(VersionNumber)

class CTestCaseVersionNumber : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseVersionNumber);
	CPPUNIT_TEST(TestVersionNumber);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestVersionNumber()
	{
		VersionNumber v1 = VersionNumMake(1, 2, 3);
		VersionNumber v2 = VersionNumMake(1, 1, 4);
		CPPUNIT_ASSERT(v1 > v2);

		v2 = VersionNumMake(2, 2, 3);
		CPPUNIT_ASSERT(v1 < v2);

		tstring	str = VersionNumToStr(v1);
		CPPUNIT_ASSERT(str == _T("1.2.3"));

		v2 = VersionNumFrom(str.c_str());
		CPPUNIT_ASSERT(v1 == v2);

		int nMajor, nMinor, nBuild;
		VersionNumSplit(v1, nMajor, nMinor, nBuild);
		CPPUNIT_ASSERT(nMajor == 1);
		CPPUNIT_ASSERT(nMinor == 2);
		CPPUNIT_ASSERT(nBuild == 3);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseVersionNumber);

#endif // _CPPUNIT_TEST
