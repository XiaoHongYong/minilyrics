/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	CProfile.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_PROFILE_H__2102F360_D5B3_45ED_921E_6A155BD28862__INCLUDED_)
#define AFX_PROFILE_H__2102F360_D5B3_45ED_921E_6A155BD28862__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <string>
#include <map>
using namespace std;

//
// 从*.ini文件中读入设置之类的
// 为了提高效率，使用缓冲，将访问过的key都保存在m_mapAppName2Keys中。
// 在保存的时候，没有使用缓冲，即只要修改，就会更新；因为修改不多，所以直接保存
//
class CProfile  
{
public:
	CProfile();
	virtual ~CProfile();

protected:
	typedef	map<string, string>		STRING_MAP;
	typedef map<string, STRING_MAP>	APPNAME_MAP;

	TCHAR	m_szProfile[MAX_PATH];
	APPNAME_MAP		m_mapAppName2Keys;
	BOOL			m_bModified;

protected:
	STRING_MAP::iterator GetKey(LPCTSTR szAppName, LPCTSTR szKeyName);

public:
	void AddKeys(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);
	void Init(LPCTSTR szProfile, HINSTANCE hInst);
	// 读和写配置文件 MiniLyrics.ini
	DWORD GetString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPSTR szReturnedString, DWORD nSize);
	BOOL WriteString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);

	UINT GetInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault);
	BOOL WriteInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue);

	static void GetFile(char szProfile[]);
};

extern CProfile	g_Profile;

////////////////////////////
// for APPNAME : "DebugTracer"
inline DWORD ProfileGetString(LPCTSTR szKeyName, LPCTSTR szDefault, LPSTR szReturnedString, DWORD nSize)
{
	return g_Profile.GetString("DebugTracer", szKeyName, szDefault, szReturnedString, nSize);
}

inline DWORD ProfileWriteString(LPCTSTR szKeyName, LPCTSTR szValue)
{
	return g_Profile.WriteString("DebugTracer", szKeyName, szValue);
}

inline BOOL ProfileWriteInt(LPCTSTR szKeyName, int nValue)
{
	return g_Profile.WriteInt("DebugTracer", szKeyName, nValue);
}

inline UINT ProfileGetInt(LPCTSTR szKeyName, int nDefault)
{
	return g_Profile.GetInt("DebugTracer", szKeyName, nDefault);
}

#endif // !defined(AFX_PROFILE_H__2102F360_D5B3_45ED_921E_6A155BD28862__INCLUDED_)
