#pragma once

bool DeleteFile(LPCTSTR lpFileName);
bool RemoveDirectory(LPCSTR lpPathName);
bool MoveFile(const char *oldname, const char *newname);
bool CopyFile(const char *oldname, const char *newname, BOOL bFailIfExists);

class CMLFileFind
{
public:
	CMLFileFind();
	virtual ~CMLFileFind();

	bool OpenDir(LPCTSTR szDir, LPCTSTR szFilter = NULL);

	void Close();

	bool FindNext();

	LPCTSTR GetCurName() { return m_dirp->d_name; }

	bool IsCurDir();

protected:
	bool GetCurStat();

	dirent		*m_dirp;
	DIR			*m_dp;
	bool		m_bStatValid;
	struct stat	m_stat;
	TCHAR		m_szDir[MAX_PATH];

};
