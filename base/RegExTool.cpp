#include "stdafx.h"
#include "Error.h"
#include "xstr.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include <regex>
#include "RegExTool.h"


tstring RegExSub(LPCSTR szRegEx, LPCTSTR szInput, LPCTSTR szWithWhat)
{
#ifdef UTF8
#ifdef _MAC
	regex re(szRegEx, regex_constants::icase);

	return regex_replace(string(szInput), re, string(szWithWhat));
#else
	std::regex re(szRegEx, std::regex_constants::icase);
	
	return std::regex_replace(string(szInput), re, string(szWithWhat));
#endif
#else
	string input, withWhat;

	TStringToUTF8(szInput, -1, input);
	TStringToUTF8(szWithWhat, -1, withWhat);

	tr1::regex re(szRegEx, std::tr1::regex_constants::icase);

	string result = tr1::regex_replace(input, re, withWhat);

	tstring strOut;
	UTF8ToTString(result.c_str(), result.size(), strOut);

	return strOut;
#endif
}
