// StringDB.cpp: implementation of the CStringDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SearchArith.h"
#include "StringDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStrICmpDB::CStrICmpDB()
{

}

CStrICmpDB::~CStrICmpDB()
{

}

BOOL CStrICmpDB::Init(LPCTSTR szFile)
{
	if (!CFileIndex1<CStrDBFunICmpStr>::Init(szFile, 1))
		return FALSE;

	CStrDBFunICmpStr	fun;
	SetCmpFun1(fun);

	return TRUE;
}

DB_POS CStrICmpDB::AddOrGetPos(LPCTSTR szString)
{
	DB_POS			pos;

	pos = SearchItemOfIndex1((LPVOID)szString);
	if (pos == POS_NULL)
	{
		pos = AddItem((LPVOID)szString, (_tcslen(szString) + 1) * sizeof(TCHAR));
	}

	return pos;
}

//////////////////////////////////////////////////////////////////////////
// CStrCmpDBA
CStrCmpDBA::CStrCmpDBA()
{

}

CStrCmpDBA::~CStrCmpDBA()
{

}

BOOL CStrCmpDBA::Init(LPCTSTR szFile)
{
	CFileIndex1<CStrDBFunCmpStrA>::Init(szFile, 1);

	CStrDBFunCmpStrA	fun;
	SetCmpFun1(fun);

	return TRUE;
}

DB_POS CStrCmpDBA::AddOrGetPos(LPCSTR szString)
{
	DB_POS			pos;

	pos = SearchItemOfIndex1((LPVOID)szString);
	if (pos == POS_NULL)
	{
		pos = AddItem((LPVOID)szString, (strlen(szString) + 1) * sizeof(char));
	}

	return pos;
}
