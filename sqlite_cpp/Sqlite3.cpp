// Sqlite3.cpp: implementation of the CSqlite3 class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../SDK/sqlite/sqlite3.h"

#include "Sqlite3.h"


int sqlite_query_int_value(sqlite3 *db, LPCSTR szSql)
{
	sqlite3_stmt*	sqlquery;
	int				n, nRet;

	nRet = sqlite3_prepare(db, szSql, -1, &sqlquery, NULL);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(db);
		return 0;
	}
	nRet = sqlite3_step(sqlquery);
	if (nRet != SQLITE_ROW)
	{
	    LogSqlite3Error(db);
		return 0;
	}
	n = sqlite3_column_int(sqlquery, 0);
	sqlite3_finalize(sqlquery);
	return n;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSqlite3::CSqlite3()
{
	m_db = NULL;
}

CSqlite3::~CSqlite3()
{

}

int CSqlite3::Open(LPCTSTR szFile)
{
	int		nRet;
	stringex	strFile;

	ConvertStr(szFile, -1, strFile);

	nRet = sqlite3_open(strFile.c_str(), &m_db);
	if (nRet != SQLITE_OK)
	{
		ERRLOG1(_T("Open db FAILED: %s"), szFile);
		return ERR_SL_OPEN_DB;
	}

	return ERR_OK;
}

void CSqlite3::Close()
{
	if (m_db)
	{
		sqlite3_close(m_db);
		m_db = NULL;
	}
}

int CSqlite3::Exec(LPCSTR szSql)
{
	int		nRet;

	nRet = sqlite3_exec(m_db, szSql, NULL, NULL, NULL);
	if (nRet != SQLITE_OK)
	{
#ifdef _UNICODE
		ERRLOG2(_T("CSqlite3::Exec %S, %S"), sqlite3_errmsg(m_db), szSql);
#else
		ERRLOG2(_T("CSqlite3::Exec %s, %s"), sqlite3_errmsg(m_db), szSql);
#endif
		return ERR_SL_EXE_SQL;
	}

	return ERR_OK;
}

LPCSTR CSqlite3::ErrorMsg()
{
	return sqlite3_errmsg(m_db);
}

//////////////////////////////////////////////////////////////////////////

CSqlite3Stmt::CSqlite3Stmt()
{
	m_db = NULL;
	m_stmt = NULL;
}

CSqlite3Stmt::~CSqlite3Stmt()
{
	Finalize();
}

int CSqlite3Stmt::Prepare(CSqlite3 *db, LPCSTR szSql)
{
	int			nRet;

	m_db = db->m_db;

	nRet = sqlite3_prepare(m_db, szSql, -1, &m_stmt, NULL);
	if (nRet != SQLITE_OK)
	{
#ifdef _UNICODE
		ERRLOG2(_T("Sqlite3Stmt::Prepare %S, %S"), sqlite3_errmsg(m_db), szSql);
#else
		ERRLOG2(_T("Sqlite3Stmt::Prepare %s, %s"), sqlite3_errmsg(m_db), szSql);
#endif
		return ERR_FALSE;
	}

	return ERR_OK;
}

int CSqlite3Stmt::Step()
{
	int			nRet;

	nRet = sqlite3_step(m_stmt);
	if (nRet == SQLITE_ROW)
		return ERR_SL_OK_ROW;
	else if (nRet == SQLITE_DONE)
		return ERR_OK;
	else
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_DB_ERR;
	}
}

void CSqlite3Stmt::Reset()
{
	sqlite3_reset(m_stmt);
}

void CSqlite3Stmt::Finalize()
{
	if (m_stmt)
	{
		sqlite3_finalize(m_stmt);
		m_stmt = NULL;
		m_db = NULL;
	}
}

int CSqlite3Stmt::BindText(int nField, LPCSTR szText, int nLen)
{
	int		nRet;

	nRet = sqlite3_bind_text(m_stmt, nField, szText, nLen, SQLITE_TRANSIENT);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::BindStaticText(int nField, LPCSTR szText, int nLen)
{
	int		nRet;

	nRet = sqlite3_bind_text(m_stmt, nField, szText, nLen, SQLITE_STATIC);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::BindBlob(int nField, LPVOID lpData, int nLen)
{
	int		nRet;

	nRet = sqlite3_bind_blob(m_stmt, nField, lpData, nLen, SQLITE_TRANSIENT);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::BindStaticBlob(int nField, LPCVOID lpData, int nLen)
{
	int		nRet;

	nRet = sqlite3_bind_blob(m_stmt, nField, lpData, nLen, SQLITE_STATIC);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::BindUCS2Text(int nField, LPCWSTR szText, int nLen)
{
	int				nRet;
	stringex	str;

	UCS2ToUtf8(szText, nLen, str);

	nRet = sqlite3_bind_text(m_stmt, nField, str.c_str(), str.size(), SQLITE_TRANSIENT);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::BindInt(int nField, int nData)
{
	int		nRet;

	nRet = sqlite3_bind_int(m_stmt, nField, nData);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
		return ERR_SL_BIND_TXT;
	}

	return ERR_OK;
}

int CSqlite3Stmt::GetColumnCount()
{
	return sqlite3_column_count(m_stmt);
}

LPCSTR CSqlite3Stmt::GetColumnName(int nCol)
{
	return sqlite3_column_name(m_stmt, nCol);
}

int CSqlite3Stmt::ColumnInt(int nCol)
{
	return sqlite3_column_int(m_stmt, nCol);
}

void CSqlite3Stmt::ColumnText(int nCol, string &strText)
{
	LPCSTR	szText = (LPCSTR)sqlite3_column_text(m_stmt, nCol);

	if (szText)
		strText = szText;
	else
		strText.resize(0);
}

LPCSTR CSqlite3Stmt::ColumnText(int nCol)
{
	return (LPCSTR)sqlite3_column_text(m_stmt, nCol);
}

void CSqlite3Stmt::ColumnBlob(int nCol, string &strData)
{
	char *szData = (char *)sqlite3_column_blob(m_stmt, nCol);
	int		len = sqlite3_column_bytes(m_stmt, nCol);

	strData.resize(0);
	if (szData)
		strData.append(szData, len);
}
