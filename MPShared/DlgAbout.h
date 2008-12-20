#pragma once

class CDlgAbout : public CBaseDialog
{
public:
	CDlgAbout();
	~CDlgAbout();

	void OnLanguageChanged();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();
	BOOL OnInitDialog();

protected:
	CHyperLinkCtrl	m_hlForum, m_hlWeb, m_hlMail;
	CCECommandBar	m_cmdBar;

};
