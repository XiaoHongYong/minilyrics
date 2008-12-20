// LrcUploadDB.cpp: implementation of the CLrcUploadDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LrcUploadDB.h"

ID_TO_STRING	_id2sUploadState[] = 
{
	{ LrcUploadFileInfo::US_UPLOAD_OK, _T("Uploaded successfully") },
	{ LrcUploadFileInfo::US_EXIST, _T("Already exists") },
	{ LrcUploadFileInfo::US_UPLOADERROR, _T("Uploaded failed") },
	{ 0, NULL }
};

#define SQL_CREATE_LYRUPLOAD		"CREATE TABLE IF NOT EXISTS LyrUploadInfo"\
	"("\
	"filename text,"\
	"state integer"\
	");"\
	"CREATE INDEX IF NOT EXISTS SR_filename on LyrUploadInfo (filename);"

#define SQL_DROP_LYRUPLOAD		"DROP TABLE LyrUploadInfo;"\
	"DROP INDEX SR_filename;"

#define SQL_ADD_LYRUPLOAD	"insert into LyrUploadInfo (filename, state) VALUES ("\
						"?, ?)"

#define SQL_QUERY_LYRUPLOAD	"select * from LyrUploadInfo where filename=?"
//#define SQL_QUERY_SEARCHRESULT	"select * from SearchReuslts"

#define SQL_DEL_LYRUPLOAD	"delete from LyrUploadInfo where filename=?"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLrcUploadDB::CLrcUploadDB()
{
	
}

CLrcUploadDB::~CLrcUploadDB()
{
}

int CLrcUploadDB::Init()
{
	int			nRet;
	TCHAR		szFile[MAX_PATH];
	stringex	strFile;

	// ´ò¿ª
	GetAppDataDir(szFile);
	_tcscat(szFile, _T("dbLyrUpload.db"));

	if (GetFileLength(szFile) >= 1024 * 100)
		DeleteFile(szFile);

	nRet = m_db.Open(szFile);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_db.Exec(SQL_CREATE_LYRUPLOAD);
	if (nRet != SQLITE_OK)
		return nRet;

	return ERR_OK;
}

bool CLrcUploadDB::SearchByFileName(LPCTSTR szFileName, LrcUploadFileInfo &lyrUploadInfo)
{
	int				nRet;
	stringex	strUtf8;
	CSqlite3Stmt	sqlQuery;

#ifdef _UNICODE
	UCS2ToUtf8(szFileName, -1, strUtf8);
#else
	strUtf8 = szFileName;
#endif

	nRet = sqlQuery.Prepare(&m_db, SQL_QUERY_LYRUPLOAD);
	if (nRet != SQLITE_DONE)
		return false;

	sqlQuery.BindStaticText(1, strUtf8.c_str(), strUtf8.size());

	nRet = sqlQuery.Step();
	if (nRet != ERR_SL_OK_ROW)
		return false;

	lyrUploadInfo.dwUploadState = sqlQuery.ColumnInt(1);

	return true;
}

int CLrcUploadDB::Add(LrcUploadFileInfo &lrcUpload)
{
	int			nRet;
	int			n = 1;
	stringex	strUtf8;
	CSqlite3Stmt	sqlAdd, sqlDelete;

#ifdef _UNICODE
	UCS2ToUtf8(lrcUpload.szFileName, -1, strUtf8);
#else
	strUtf8 = lrcUpload.szFileName;
#endif

	// delete old result
	nRet = sqlDelete.Prepare(&m_db, SQL_DEL_LYRUPLOAD);
	if (nRet != ERR_OK)
		return nRet;

	sqlDelete.BindStaticText(1, strUtf8.c_str(), strUtf8.size());
	nRet = sqlDelete.Step();
	if (nRet != SQLITE_DONE)
		return nRet;

	sqlDelete.Finalize();

	// Add result
	nRet = sqlAdd.Prepare(&m_db, SQL_ADD_LYRUPLOAD);
	if (nRet != ERR_OK)
		return nRet;

	n = 1;
	sqlAdd.BindStaticText(n++, strUtf8.c_str(), strUtf8.size());
	sqlAdd.BindInt(n++, lrcUpload.dwUploadState);

	nRet = sqlAdd.Step();
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

LPCTSTR CLrcUploadDB::UploadStateToStr(DWORD dwUploadState)
{
	return IDToString(_id2sUploadState, dwUploadState, _T("Unknown"));
}
