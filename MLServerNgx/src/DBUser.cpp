// DBUser.cpp: implementation of the CDBUser class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLServer.h"
#include "DBUser.h"

/**
 CREATE TABLE IF NOT EXISTS users (
 id integer primary key AUTOINCREMENT, MLPasswordHash, PasswordHash text,
 Email text, EmailLower text, CreateDate date, LastLoginDate date,
 LastPasswordChangedDate date, UserName text, UserNameLower text

 */

#define SQL_LOGIN_WITH_MLPASSWORD	"SELECT id FROM users WHERE UserNameLower=? and MLPasswordHash=?"

CDBUser        g_UserDB;

CDBUser::CDBUser()
{
	m_db = NULL;
}

CDBUser::~CDBUser()
{

}

int CDBUser::Init(const char *fileName)
{
	int ret = sqlite3_open(fileName, &m_db);
	if (ret != SQLITE_OK)
	{
		ERRLOG1(_T("Open user db FAILED: %s"), fileName);
		return ERR_FALSE;
	}

	SQLIT3_STMT_PREPARE(m_db, SQL_LOGIN_WITH_MLPASSWORD, m_sqlLoginWithMLPassword);

	return ERR_OK;
}

void CDBUser::Quit()
{
	sqlite3_finalize(m_sqlLoginWithMLPassword);
}

int CDBUser::LoginUser(LPCSTR szLoginName, LPCSTR szUserPwdMask, long &nUserID)
{
	int	ret = ERR_OK, n = 1;

    string name = ToLower(szLoginName);

    SQLITE3_BIND_TEXT(m_db, m_sqlLoginWithMLPassword, name.c_str());
    SQLITE3_BIND_TEXT(m_db, m_sqlLoginWithMLPassword, szUserPwdMask);

	// Check for password
	ret = sqlite3_step(m_sqlLoginWithMLPassword);
	if (ret == SQLITE_ERROR) {
		LogSqlite3Error(m_db);
		ret = ERR_FALSE;
	} else if (ret == SQLITE_ROW) {
		nUserID = sqlite3_column_int(m_sqlLoginWithMLPassword, 0);
		ret = ERR_OK;
    } else {
		ret = ERR_BAD_USERPWD;
    }

	sqlite3_reset(m_sqlLoginWithMLPassword);

	return ret;
}
