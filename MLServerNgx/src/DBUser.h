#pragma once

#include "Sqlite3Utils.h"

#define		TIME_OFF_SET_FACTOR			100

class CDBUser
{
public:
	CDBUser();
	virtual ~CDBUser();

	int Init(const char *fileName);
	void Quit();

	int LoginUser(LPCSTR szLoginName, LPCSTR szUserPwdMask, long &nUserID);

	int IncUserLrcUploadCount(LPCTSTR szLoginName, int nCount = 1);
	int IncUserLrcUploadCount(long nID, int nCount = 1);

protected:
	sqlite3*		m_db;

	sqlite3_stmt	*m_sqlLoginWithMLPassword;

};

extern CDBUser        g_UserDB;
