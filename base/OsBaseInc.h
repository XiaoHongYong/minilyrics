#pragma once

#include "config.h"

#if defined(_LINUX_GTK2)
#include <gtk/gtk.h>
#define MLDebug		g_print
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT	0x0500

// Windows Header Files:
#include <windows.h>
#include <winuser.h>
#include <Shlobj.h>
#include <shellapi.h>
#include <commdlg.h>

#include <tchar.h>
#include <malloc.h>
#endif

#ifdef _WINCE_
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winuser.h>
#include <Shlobj.h>
#include <shellapi.h>
#include <commdlg.h>

#include <tchar.h>
#include <malloc.h>
#endif

#ifdef _LINUX
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#endif

#ifdef _MAC
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>


// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using namespace std;

#ifndef ASSERT
#define ASSERT	assert
#endif

#include "mltypes.h"
#include "common.h"
#include "wintypes.h"

typedef vector<tstring>		V_Str;

typedef list<tstring>		ListString;

class SetStrLessICmp
{
public:
	bool operator()(const tstring &str1, const tstring &str2) const
	{
		return _tcsicmp(str1.c_str(), str2.c_str()) < 0;
	}
};

typedef set<tstring, SetStrLessICmp>		SetTStrNCase;

//
//#define ML_USES_CONVERSION		USES_CONVERSION
//#define ML_T2A					T2A
//#define ML_T2CA					T2CA
//#define ML_T2W					T2W
//#define ML_T2CW					T2CW
//#define ML_A2CT					A2CT
//#define ML_W2CA					W2CA

// CPPUnit test headers
#include "cppunit-utils.h"
