#pragma once

class CLDSkinWndMain : public CLDSkinWnd
{
public:
	CLDSkinWndMain(void);
	~CLDSkinWndMain(void);

	virtual void OnDestroy();

	virtual void OnSkinLoaded();

	virtual void OnCommand(UINT uID, UINT nNotifyCode);

private:
	CSkinListCtrl		*m_pFileListCtrl;

};
