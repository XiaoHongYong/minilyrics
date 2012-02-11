
#pragma once

class CDlgSaveEmbeddedLyrics : public CBaseDialog  
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

protected:
	void AddEmbeddedItem(VecString &vAlreadyHave, LPCTSTR szToAdd);

	int DoSaveAction(VecString &vLyrNames, CBaseWnd *pWnd);

protected:
	tstring				m_strMediaUrl;
	CCharBuffer			m_bufLyrics;

	CWidgetListCtrl		m_ctrlListLyrics;
	uint32				m_nDefaultEmbededLST;
	bool				m_bHasEmbeddedLyrAlready;

};
