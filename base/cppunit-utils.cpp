#include "stdafx.h"
#include "cppunit-utils.h"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <stdexcept>
#include <fstream>

#ifdef _CPPUNIT_TEST


#ifdef _WIN32

#include <fcntl.h>
#include <io.h>

#define MAX_CONSOLE_LINES	500

void RedirectIOToConsole()
{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	ios::sync_with_stdio();
}
#endif // _WIN32

void RunUnitTest()
{
	// Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;
	string			testPath;

	// Add a listener that colllects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener(&result);        

	// Add a listener that print dots as test run.
#ifdef WIN32
	CPPUNIT_NS::TextTestProgressListener progress;
#else
	CPPUNIT_NS::BriefTestProgressListener progress;
#endif
	controller.addListener( &progress );      

	// Add the top suite to the test runner
	CPPUNIT_NS::TestRunner runner;
	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );   
	try
	{
		std::cout << "Running "  <<  testPath;
		runner.run(controller, testPath);

		std::cerr << std::endl;

		// Print test in a compiler compatible format.
		CPPUNIT_NS::CompilerOutputter outputter(&result, std::cerr);
		outputter.write(); 

		// Uncomment this for XML output
		//    std::ofstream file( "tests.xml" );
		//    CPPUNIT_NS::XmlOutputter xml( &result, file );
		//    xml.setStyleSheet( "report.xsl" );
		//    xml.write();
		//    file.close();
	}
	catch (std::invalid_argument &e)  // Test path not resolved
	{
		std::cerr  <<  std::endl  
			<<  "ERROR: "  <<  e.what()
			<< std::endl;
		return;
	}

	result.wasSuccessful() ? 0 : 1;
}

#endif // _CPPUNIT_TEST
