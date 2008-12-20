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

#endif // _ML_COMMON_INC_
