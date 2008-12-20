// StringDB.h: interface for the CStringDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGDB_H__1C397A15_C12F_4B60_9BBC_6BD4AC2B323E__INCLUDED_)
#define AFX_STRINGDB_H__1C397A15_C12F_4B60_9BBC_6BD4AC2B323E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStrDBFunICmpStr
{
public:
	int operator()(LPVOID lpItem1, LPVOID lpItem2)
	{
		return _tcsicmp((LPCTSTR)lpItem1, (LPCTSTR)lpItem2);
	}
};

class CStrICmpDB : public CFileIndex1<CStrDBFunICmpStr>
{

public:
	CStrICmpDB();
	virtual ~CStrICmpDB();

	BOOL Init(LPCTSTR szFile);

	DB_POS AddOrGetPos(LPCTSTR szString);

	LPCTSTR GetString(DB_POS pos)
	{
		return (LPCTSTR)GetItem(pos);
	}

};

typedef CStrICmpDB		CStringDB;

class CStrDBFunCmpStrA
{
public:
	int operator()(LPVOID lpItem1, LPVOID lpItem2)
	{
		return strcmp((LPCSTR)lpItem1, (LPCSTR)lpItem2);
	}
};

class CStrCmpDBA : public CFileIndex1<CStrDBFunCmpStrA>
{
public:
	CStrCmpDBA();
	virtual ~CStrCmpDBA();

	BOOL Init(LPCTSTR szFile);

	DB_POS AddOrGetPos(LPCSTR szString);

	DB_POS GetPos(LPCTSTR szString)
	{
		return SearchItemOfIndex1((LPVOID)szString);
	}

	LPCSTR GetString(DB_POS pos)
	{
		return (LPCSTR)GetItem(pos);
	}

};

#endif // !defined(AFX_STRINGDB_H__1C397A15_C12F_4B60_9BBC_6BD4AC2B323E__INCLUDED_)
