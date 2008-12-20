// idstring.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "idstring.h"
#include "base.h"

uint32 StringToID(ID_TO_STRING_A Ids[], LPCSTR szID, uint32 dwDefault)
{
	for (ID_TO_STRING_A *ptr = Ids; ptr->szId != NULL; ptr++)
	{
		if (_stricmp(szID, ptr->szId) == 0)
			return ptr->dwId;
	}

	return dwDefault;
}

LPCSTR IDToString(ID_TO_STRING_A Ids[], uint32 nID, LPCSTR szDefaultID)
{
	for (ID_TO_STRING_A *ptr = Ids; ptr->szId != NULL; ptr++)
	{
		if (nID == ptr->dwId)
			return ptr->szId;
	}

	return szDefaultID;
}


uint32 StringToID(ID_TO_STRING Ids[], LPCTSTR szID, uint32 dwDefault)
{
	for (ID_TO_STRING *ptr = Ids; ptr->szId != NULL; ptr++)
	{
		if (_tcsicmp(szID, ptr->szId) == 0)
			return ptr->dwId;
	}

	return dwDefault;
}

LPCTSTR IDToString(ID_TO_STRING Ids[], uint32 nID, LPCTSTR szDefaultID)
{
	for (ID_TO_STRING *ptr = Ids; ptr->szId != NULL; ptr++)
	{
		if (nID == ptr->dwId)
			return ptr->szId;
	}

	return szDefaultID;
}

uint32 GetCombinationValue(ID_TO_STRING Ids[], LPCTSTR szValue)
{
	uint32			dwFlags;
	vector<tstring>	vStr;

	StrBreak(szValue, '|', vStr);
	dwFlags = 0;

	for (int i = 0; i < (int)vStr.size(); i++)
	{
		TrimStr(vStr[i]);
		dwFlags |= StringToID(Ids, vStr[i].c_str(), 0);
	}

	return dwFlags;
}

void GetCombinationStrValue(ID_TO_STRING Ids[], uint32 nValue, tstring &strValue)
{
	for (int i = 0; Ids[i].szId != NULL; i++)
	{
		if ((Ids[i].dwId & nValue) == Ids[i].dwId)
		{
			if (!strValue.empty())
				strValue += _T("|");
			strValue += Ids[i].szId;
		}
	}
}
