// DlgUpdateLibrary.h: interface for the CDlgUpdateLibrary class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CUpdateLibraryObj
{
public:
	CUpdateLibraryObj();
	~CUpdateLibraryObj();

	virtual void DoUpdating() = 0;

	void CreateWorkThread(CBaseWnd *hWndNotify);

	static void WorkThread(LPVOID lpParam);

	void SetMsg1(LPCTSTR szMsg);
	void SetMsg2(LPCTSTR szMsg);

protected:
	CThread			m_thread;
	CBaseWnd		*m_pWnd;

};

class CAddMediaObj : public CUpdateLibraryObj
{
public:
	CAddMediaObj()
	{
		m_bAddFiles = false;
	}
	virtual void DoUpdating();

	bool				m_bAddFiles;
	tstring				m_strDir;
	vector<tstring>		m_vFiles;

protected:
	void ListMedia(LPCTSTR szDir);

};


class CDlgUpdateLibrary
{
public:
	CDlgUpdateLibrary();
	virtual ~CDlgUpdateLibrary();

	BOOL DoModal(CBaseWnd *pWndParent, CUpdateLibraryObj *pWorkObj);

protected:
	CUpdateLibraryObj			*m_pWorkObj;
	bool						m_bCanQuit;

};
