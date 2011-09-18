// UnitTestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../base/base.h"


#ifdef _CPPUNIT_TEST
void MLRunUnitTest()
{
	// CreateUnitResultWnd();

	::RunUnitTest();
}
#endif // _CPPUNIT_TEST

int _tmain(int argc, _TCHAR* argv[])
{
	MLRunUnitTest();

	return 0;
}

