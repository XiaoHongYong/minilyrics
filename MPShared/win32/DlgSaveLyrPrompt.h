#pragma once

int SaveLyrDialogBox(CBaseWnd *pWnd);

class CDlgSaveLyrPrompt : public CBaseDialog
{
public:
	CDlgSaveLyrPrompt(LPCTSTR szInfo) : CBaseDialog(IDD_MSG) { m_strInfo = szInfo; }
	
	BOOL OnInitDialog();

	void OnCommand(UINT uID, UINT nNotifyCode);
	
protected:
	tstring			m_strInfo;
	
};


