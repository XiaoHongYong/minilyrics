// LocalizeTool.h: interface for the CLocalizeTool class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

LPCTSTR TOLOCAL(LPCTSTR szString);

#define ERROR2STR_LOCAL(nError)	(TOLOCAL(Error2Str(nError)))
