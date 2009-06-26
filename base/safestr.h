// safestr.h
#ifndef _SAFE_STRING_H_
#define _SAFE_STRING_H_

#include "xstr.h"

// 安全的字符串拷贝，如果超出长度范围，则进行截断
char *strcpy_safe(char *strDestination, int nLenMax, const char *strSource);
int	strncpy_safe(char *strDestination, int nLenMax, const char *strSource, int nToCopy);
int	strncpysz_safe(char *strDestination, int nLenMax, const char *strSource, int nToCopy);
char *strcat_safe(char * dst, int Size, const char * src);

wchar_t *wcscpy_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource);
int	wcsncpy_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource, int nToCopy);
int	wcsncpysz_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource, int nToCopy);
wchar_t * __cdecl wcscat_safe(wchar_t * dst, int Size, const wchar_t * src);

#ifdef _UNICODE
#define _tcscpy_safe		wcscpy_safe
#define _tcsncpy_safe		wcsncpy_safe
#define _tcsncpysz_safe		wcsncpysz_safe
#define _tcscat_safe		wcscat_safe
#else
#define _tcscpy_safe		strcpy_safe
#define _tcsncpy_safe		strncpy_safe
#define _tcsncpysz_safe		strncpysz_safe
#define _tcscat_safe		strcat_safe
#endif

#ifndef _WIN32
#define vsnprintf_s vsnprintf
#define _vsntprintf vsnprintf
#define _vstprintf_s vsnprintf
#define _stprintf_s sprintf_s
#define _stprintf_s sprintf_s
#define _itoa_s	_itot_s

inline int sprintf_s(char *string, size_t sizeInBytes, const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	return vsnprintf(string, sizeInBytes, format, arglist);
}

inline char *_itoa(int value, char *string, size_t stringLen, int radix)
{
	return _itox_s(value, string, stringLen, radix);
}

inline int _itot_s(int value, TCHAR *buffer, size_t sizeInCharacters, int radix)
{
	_itox_s(value, buffer, sizeInCharacters, radix);
	return 0;
}


#endif

#endif // _SAFE_STRING_H_
