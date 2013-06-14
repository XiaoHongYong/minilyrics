
#pragma once

class CDlgSaveEmbeddedLyrics
{
public:
	CDlgSaveEmbeddedLyrics();
	virtual ~CDlgSaveEmbeddedLyrics();

	int DoModal(CBaseWnd *pWnd);

	virtual BOOL OnInitDialog();

	virtual void OnOK();

	enum {
		COL_DESC = 0,
		COL_NAME,
	};

	LPVOID GetControllerHandle() { return m_pDlgController; }
	
protected:
	void AddEmbeddedItem(VecString &vAlreadyHave, LPCTSTR szToAdd);

	int DoSaveAction(VecString &vLyrNames, CBaseWnd *pWnd);
	
protected:
	tstring				m_strMediaUrl;
	CCharBuffer			m_bufLyrics;

	LPVOID				m_pDlgController;
	uint32				m_nDefaultEmbededLST;
	bool				m_bHasEmbeddedLyrAlready;
	
	CBaseWnd			*m_pParentWnd;

};
