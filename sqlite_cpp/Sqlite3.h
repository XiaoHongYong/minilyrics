// Sqlite3.h: interface for the CSqlite3 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SQLITE3_H__6B98E6CE_8F3B_4230_A6A2_FB8A1CA6BB88__INCLUDED_)
#define AFX_SQLITE3_H__6B98E6CE_8F3B_4230_A6A2_FB8A1CA6BB88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../SDK/sqlite/sqlite3.h"
#include "../MPlayerLib/IMPlayer.h"

#define LogSqlite3Error(db)		DBGLOG1(_T("Sqlite3 error: %S"), sqlite3_errmsg(db))

int sqlite_query_int_value(sqlite3 *db, LPCSTR szSql);

class CSqlite3  
{
public:
	CSqlite3();
	virtual ~CSqlite3();

	int Open(LPCTSTR szFile);
	void Close();

	int Exec(LPCSTR szSql);

	LPCSTR ErrorMsg();

	int LastInsertRowId() { return (int)sqlite3_last_insert_rowid(m_db); }

public:
	sqlite3			*m_db;

};

class CSqlite3Stmt
{
public:
	CSqlite3Stmt();
	virtual ~CSqlite3Stmt();

	int Prepare(CSqlite3 *db, LPCSTR szSql);
	int Step();
	void Reset();
	void Finalize();

	int BindText(int nField, LPCSTR szText, int nLen = -1);
	int BindStaticText(int nField, LPCSTR szText, int nLen = -1);
	int BindUCS2Text(int nField, LPCWSTR szText, int nLen = -1);
	int BindBlob(int nField, LPVOID lpData, int nLen);
	int BindStaticBlob(int nField, LPCVOID lpData, int nLen);

	int BindInt(int nField, int nData);

	int ColumnInt(int nCol);
	void ColumnText(int nCol, string &strText);
	LPCSTR ColumnText(int nCol);
	void ColumnBlob(int nCol, string &strData);

public:
	sqlite3			*m_db;
	sqlite3_stmt	*m_stmt;

};

#endif // !defined(AFX_SQLITE3_H__6B98E6CE_8F3B_4230_A6A2_FB8A1CA6BB88__INCLUDED_)
