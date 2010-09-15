#ifndef __CPPUNIT_UTILS__
#define __CPPUNIT_UTILS__

#pragma once

#ifdef _CPPUNIT_TEST

//
// DECLAR_CPPUNIT_TEST_REG and IMPLEMENT_CPPUNIT_TEST_REG is used to be sure
// that every testFixture in the static library will be registered and called.
//
// Every .cpp file with CPPUnit TestCase should have IMPLEMENT_CPPUNIT_TEST_FILE
//

#define DECLAR_CPPUNIT_TEST_REG(UniName)		int CPPUnitTest##UniName();	\
	namespace NS_CPPUnitTest##UniName { const int __ntempCPPTest = CPPUnitTest##UniName(); }

#define IMPLEMENT_CPPUNIT_TEST_REG(UniName)		int CPPUnitTest##UniName() { return 0; }

#else	// _CPPUNIT_TEST

#define DECLAR_CPPUNIT_TEST_REG(UniName)		
#define IMPLEMENT_CPPUNIT_TEST_REG(UniName)		

#endif	// _CPPUNIT_TEST


#ifdef _CPPUNIT_TEST

// CPPUnit test headers
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/config/SourcePrefix.h>

#include "StrPrintf.h"
#include "stringex_t.h"
#include "CharEncoding.h"


BOOL GetModulePath(TCHAR szPath[], HINSTANCE hInstance);


/** Fails with the specified message.
* \ingroup Assertions
* \param message Message reported in diagnostic.
*/
#define CPPUNIT_FAIL_T( message )                                         \
{ stringex	strAnsi; ConvertStr(message, -1, strAnsi); CppUnit::Asserter::fail( CPPUNIT_NS::Message( "forced failure",  \
	strAnsi.c_str() ),         \
	CPPUNIT_SOURCELINE() ); }

inline tstring GetUnitTestFolder()
{
	TCHAR		szPath[MAX_PATH];

	GetModulePath(szPath, NULL);

	return szPath;
}

#ifdef _WIN32
void RedirectIOToConsole();
#else
void RedirectIOToConsole() { }
#endif

void RunUnitTest();

#endif // _CPPUNIT_TEST

#endif // __CPPUNIT_UTILS__
