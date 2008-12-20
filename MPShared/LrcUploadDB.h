// LrcUploadDB.h: interface for the CLrcUploadDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_)
#define AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_

#include "../sqlite_cpp/sqlite3.h"

struct LrcUploadFileInfo
{
	enum UPLOADSTATE
	{
		US_UPLOAD_OK,
		US_EXIST,
		US_UPLOADERROR
	};

	TCHAR	szFileName[MAX_PATH];
	DWORD	dwUploadState;
};

class CLrcUploadDB  
{
public:
	int Init();

	bool SearchByFileName(LPCTSTR szFileName, LrcUploadFileInfo &lyrUploadInfo);

	int Add(LrcUploadFileInfo &lrcUpload);

	static LPCTSTR UploadStateToStr(DWORD dwUploadState);

public:
	CLrcUploadDB();
	virtual ~CLrcUploadDB();

protected:
	CSqlite3		m_db;

};

#endif // !defined(AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_)
