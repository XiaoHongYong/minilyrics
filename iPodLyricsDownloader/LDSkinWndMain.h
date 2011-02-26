#pragma once

class CLDSkinWndMain : public CLDSkinWnd
{
public:
	CLDSkinWndMain(void);
	~CLDSkinWndMain(void);

	virtual void OnDestroy();

	virtual void OnSkinLoaded();

private:
	CSkinListCtrl		*m_pFileListCtrl;

};
