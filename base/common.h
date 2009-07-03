// MLError.h: interface for the CMLError class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ML_COMMON_INC_
#define _ML_COMMON_INC_

enum FILE_ENCODING
{
	FE_ANSI,
	FE_UCS2,
	FE_UTF8
};

#define CountOf(arr)		(sizeof(arr) / sizeof(arr[0]))

#define IsEmptyString(str)	((str)[0] == '\0')

#define EmptyStr(str)	((str)[0] = '\0')

#define IsFlagValid(value, flag)	(((value) & (flag)) == (flg))
#define IsFlagSet(value, flag)	(((value) & (flag)) == (flag))

#define Tobool(bValue)		((bValue) != 0)

template<class int_t1, class int_t2>
int_t1 inline max_fun(int_t1 a, int_t2 b)
{
	return (((a) > (b)) ? (a) : (b));
}

template<class int_t1, class int_t2>
int_t1 inline min_fun(int_t1 a, int_t2 b)
{
	return (((a) < (b)) ? (a) : (b));
}

#ifndef min
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define DIR_SLASH		'\\'
#define SZ_DIR_SLASH	"\\"
#else
#define DIR_SLASH		'/'
#define SZ_DIR_SLASH	"/"
#endif

#define CHAR_SPACE		_T(' ')
#define CHAR_TAB		_T('\t')
#define CHAR_NEWLINE	_T('\n')
#define CHAR_RETURN		_T('\r')
#define CHAR_NULL		_T('\0')
#define CHAR_COMMA		_T(',')
#define CHAR_SEMICOLON	_T(';')

#ifndef _WIN32
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((a) & 0xff)) | ((WORD)((BYTE)((b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((a) & 0xffff)) | ((DWORD)((WORD)((b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((l) & 0xffff))
#define HIWORD(l)           ((WORD)((l) >> 16))
#define LOBYTE(w)           ((BYTE)((w) & 0xff))
#define HIBYTE(w)           ((BYTE)((w) >> 8))
#endif

#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#endif

#endif // _ML_COMMON_INC_
