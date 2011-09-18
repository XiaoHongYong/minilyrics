#include "stdafx.h"
#include "cppunit-utils.h"

#ifdef _CPPUNIT_TEST

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
#include <cppunit/Portability/Stream.h>

using namespace CppUnit;

#ifdef _WIN32

#include <fcntl.h>
#include <io.h>


#define SZ_CLASSNAME _T("UnitTest")
#define SZ_WNDNAME	_T("UnitTest")

LRESULT CALLBACK LogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		DestroyWindow(hWnd);
		return 0;
	}
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ATOM UnitTestRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= LogWndProc;
	wcex.hInstance		= hInstance;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= SZ_CLASSNAME;

	return RegisterClassEx(&wcex);
}

HWND g_hWndLogEdit = NULL;
HWND g_hWndLog =  NULL;

void LogUnitTestResult(LPCSTR szStr)
{
	if (g_hWndLogEdit == NULL)
	{
		printf(szStr);
		return;
	}

	int	len = GetWindowTextLength(g_hWndLogEdit);
	SendMessage(g_hWndLogEdit, EM_SETSEL, len, len);
#ifdef _UNICODE
	wstringex	strW;
	ConvertStr(szStr, -1, strW);
	SendMessage(g_hWndLogEdit, EM_REPLACESEL, FALSE, (WPARAM)strW.c_str());
#else
	SendMessage(g_hWndLogEdit, EM_REPLACESEL, FALSE, (WPARAM)szStr);
#endif
}

void CreateUnitResultWnd(HINSTANCE hInstance)
{
	UnitTestRegisterClass(hInstance);

	g_hWndLog = CreateWindow(SZ_CLASSNAME, SZ_WNDNAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!g_hWndLog)
		return;

	RECT rc;
	::GetClientRect(g_hWndLog, &rc);

	// Create Edit
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL| ES_MULTILINE | ES_WANTRETURN;
	g_hWndLogEdit = CreateWindow(_T("EDIT"), _T(""), dwStyle, 
		0, 0, rc.right, rc.bottom, g_hWndLog, NULL, hInstance, NULL);

	ShowWindow(g_hWndLog, SW_SHOW);
	UpdateWindow(g_hWndLog);
}
/*
void RedirectIOToConsole()
{
#define MAX_CONSOLE_LINES	500

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
}*/
#else
void LogUnitTestResult(LPCSTR str)
{
	printf(str);
}
#endif // _WIN32


class LogStreamBuffer : public StreamBuffer
{
public:
	virtual void write( const char *text, unsigned int length )
	{
		string	str;
		str.append(text, length);
		LogUnitTestResult(str.c_str());
	}

};

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

	LogStreamBuffer buf;
	OStream streamLog(&buf);

	stdCOut().setBuffer(&buf);

	// Add the top suite to the test runner
	CPPUNIT_NS::TestRunner runner;
	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );   
	try
	{
		LogUnitTestResult(("Running " + testPath).c_str());
		// std::cout << "Running "  <<  testPath;
		runner.run(controller, testPath);

		LogUnitTestResult("\r\n");
		// std::cerr << std::endl;

		// Print test in a compiler compatible format.
		CPPUNIT_NS::CompilerOutputter outputter(&result, streamLog);
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
		LogUnitTestResult("\r\nERROR: ");
		LogUnitTestResult(e.what());
		LogUnitTestResult("\r\n");
// 		std::cerr  <<  std::endl  
// 			<<  "ERROR: "  <<  e.what()
// 			<< std::endl;
		return;
	}

	stdCOut().setBuffer(NULL);

	if (result.wasSuccessful())
		SetTimer(g_hWndLog, 0, 1000, NULL);
}

#endif // _CPPUNIT_TEST
