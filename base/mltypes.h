#ifndef _ML_TYPES_H_
#define _ML_TYPES_H_


typedef unsigned int uint;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;

#ifdef _LINUX
typedef int64_t int64;
typedef u_int64_t uint64;
#endif

#ifdef WIN32
#ifdef _WINCE_
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef LONGLONG int64;
typedef int64 uint64;
#endif
#endif

#if defined(_MAC)
typedef int64_t int64;
typedef u_int64_t uint64;
#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif
#endif

#if !defined(WIN32) && !defined(_WIN32_WCE)

#if !defined( _OBJC_OBJC_H_)
#if defined(_MAC)
typedef signed char		BOOL; 
#else
typedef long BOOL;
#endif
#endif // !defined( _OBJC_OBJC_H_)

#define  __cdecl		
#define WINAPI __stdcall

typedef unsigned char byte;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned int UINT;
typedef unsigned int uint;

typedef DWORD   COLORREF;

typedef /* [string] */ const char *LPCSTR;
typedef /* [string] */ const wchar_t *LPCWSTR;
typedef /* [string] */ char *LPSTR;
typedef /* [string] */ wchar_t *LPWSTR;
typedef /* [string] */ wchar_t WCHAR;

#ifdef UNICODE
typedef LPCWSTR		LPCTSTR;
typedef LPWSTR		LPTSTR;
typedef wchar_t		TCHAR;
#else
typedef LPCSTR		LPCTSTR;
typedef LPSTR		LPTSTR;
typedef char		TCHAR;
#endif

typedef const void* LPCVOID;

typedef void* LPVOID;

typedef BYTE		*LPBYTE;

// #define TRUE		1
// #define FALSE		0

#endif

#ifdef _LINUX
#include <gdk/gdktypes.h>
#endif


#endif // _ML_TYPES_H_
