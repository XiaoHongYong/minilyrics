#pragma once


DECLAR_CPPUNIT_TEST_REG(VersionNumber)


typedef uint32	VersionNumber;

VersionNumber VersionNumMake(int nMajor, int nMinor, int nBuild);

void VersionNumSplit(VersionNumber ver, int &nMajor, int &nMinor, int &nBuild);

VersionNumber VersionNumFrom(LPCTSTR szVersion);

tstring VersionNumToStr(VersionNumber verNum);
