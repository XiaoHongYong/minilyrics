// LocalizeTool.h: interface for the CLocalizeTool class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

LPCTSTR _TL(LPCTSTR szString);
#define _TLT _TL
#define _TL3 _TL

#define _TLS(szString)
#define _TLM(szString)		_T(szString)

//#define ERROR2STR_LOCAL(nError)	(g_LangTool.ToLocalString(Error2Str(nError)))
tstring RemovePrefixOfAcckey(LPCTSTR szStr);
