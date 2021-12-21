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

BOOL WritePrivateProfileInt(
							LPCTSTR lpAppName,  // section name
							LPCTSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCTSTR lpFileName  // initialization file
							);

#if !defined(WIN32) || defined(_WIN32_WCE)
DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName);
UINT GetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nDefault, LPCTSTR lpFileName);
#endif

#ifdef _WIN32
DWORD RegGetProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize);
BOOL RegWriteProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szValue);

BOOL RegWriteProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nValue);
int RegGetProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nDefault);
#endif

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
	typedef map<tstring, tstring>	MAP_STRING;

	struct APP_SECT
	{
		tstring		strAppName;
		MAP_STRING	mapKeys;		// all keyname will be converted to lowercase.
	};
	typedef list<APP_SECT*>		LIST_APPSECT;

	tstring			m_strProfile;
	LIST_APPSECT	m_listAppSect;
	tstring			m_strDefAppName;

#ifdef _WIN32
	//
	// Use Windows registry as target, pass szProfile with HKEY_XXX with use this.
	//
	enum
	{
		RET_STR_MAX		= 3,
		RET_STR_LEN		= 1024
	};
	HKEY			m_hKeyRoot;
	LPTSTR			m_szRetString[RET_STR_MAX];
	uint32			m_nCurRetStr;
#endif

protected:
	void AddKeys(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);
	bool GetKey(LPCTSTR szAppName, LPCTSTR szKeyName, MAP_STRING::iterator &it);

	void SetKeyDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);

public:
	void Init(LPCTSTR szProfile, LPCTSTR szDefKey = NULL);
	void Close();

	void DoCache();

	void SetDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault);
	void SetDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault);

	// 读和写配置文件 MiniLyrics.ini
	DWORD GetString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize);
	LPCTSTR GetString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault);
	BOOL WriteString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);

	UINT GetInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault);
	BOOL WriteInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue);

	bool GetBool(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
		{ return GetInt(szAppName, szKeyName, nDefault) != 0; }
	bool GetBool(LPCTSTR szKeyName, int nDefault)
		{ return GetInt(m_strDefAppName.c_str(), szKeyName, nDefault) != 0; }

	// 无需输入 szAppName 的函数
	DWORD GetString(LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
		{ return GetString(m_strDefAppName.c_str(), szKeyName, szDefault, szReturnedString, nSize); }
	LPCTSTR GetString(LPCTSTR szKeyName, LPCTSTR szDefault)
		{ return GetString(m_strDefAppName.c_str(), szKeyName, szDefault); }
	BOOL WriteString(LPCTSTR szKeyName, LPCTSTR szValue)
		{ return WriteString(m_strDefAppName.c_str(), szKeyName, szValue); }

	UINT GetInt(LPCTSTR szKeyName, int nDefault)
		{ return GetInt(m_strDefAppName.c_str(), szKeyName, nDefault); }
	BOOL WriteInt(LPCTSTR szKeyName, int nValue)
		{ return WriteInt(m_strDefAppName.c_str(), szKeyName, nValue); }

	LPCTSTR GetFile();
	LPCTSTR GetDefAppName();

	//
	// support encrypt keyname and value
	//	only keyname is encrypted in cache
	//  vlaue does not encrypted in cache, and value only encrypted in file.
	//
	tstring EncryptGetString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault);
	BOOL EncryptWriteString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue);

	// 无需输入 szAppName 的函数
	tstring EncryptGetString(LPCTSTR szKeyName, LPCTSTR szDefault)
		{ return EncryptGetString(m_strDefAppName.c_str(), szKeyName, szDefault); }
	BOOL EncryptWriteString(LPCTSTR szKeyName, LPCTSTR szValue)
		{ return EncryptWriteString(m_strDefAppName.c_str(), szKeyName, szValue); }
	int EncryptGetInt(LPCTSTR szKeyName, int nValue);
	void EncryptWriteInt(LPCTSTR szKeyName, int nValue);

};


#endif // !defined(AFX_PROFILE_H__2102F360_D5B3_45ED_921E_6A155BD28862__INCLUDED_)
