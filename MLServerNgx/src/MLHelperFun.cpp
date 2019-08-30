// HelperFun.cpp: implementation of the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#include "Stdafx.h"
#include "MLServer.h"
#include "MLHelperFun.h"


bool IsCorrectUserNameChars(const char *szName)
{
	while (*szName)
	{
		if ((*szName >= 'a' && *szName <= 'z') ||
			(*szName >= 'A' && *szName <= 'Z') ||
			(*szName >= '0' && *szName <= '9') ||
			(*szName == '_') || (*szName == '-'))
			szName++;
		else
			return false;
	}
	return true;
}

bool IsCorrectEmailChars(const char *szName)
{
	while (*szName)
	{
		if ((*szName >= 'a' && *szName <= 'z') ||
			(*szName >= 'A' && *szName <= 'Z') ||
			(*szName >= '0' && *szName <= '9') ||
			(*szName == '_') || (*szName == '-') ||
			(*szName == '@') || (*szName == '.'))
			szName++;
		else
			return false;
	}
	return true;
}

void GetRelatedDir(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedDir)
{
	int		nBaseLen = strlen(szBaseDir);
	if (strnicmp(szBaseDir, szDir, nBaseLen) == 0)
	{
		if (szDir[nBaseLen] == DIR_SLASH)
			nBaseLen++;
		strcpy(szRelatedDir, szDir + nBaseLen);
	}
	else
	{
		assert(0 && "GetRelatedPath()");
		strcpy(szRelatedDir, szDir);
	}
}
