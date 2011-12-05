// DlgUpdateLibrary.h: interface for the CDlgUpdateLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_)
#define AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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


class CDlgUpdateLibrary : public CBaseDialog  
{
public:
	CDlgUpdateLibrary();
	virtual ~CDlgUpdateLibrary();

	BOOL OnInitDialog();

	virtual void OnCancel();
	virtual void OnOK();

	BOOL DoModal(CBaseWnd *pWndParent, CUpdateLibraryObj *pWorkObj);

#ifdef _WIN32
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

protected:
	CUpdateLibraryObj			*m_pWorkObj;
	bool						m_bCanQuit;

};

#endif // !defined(AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_)
