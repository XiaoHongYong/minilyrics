#pragma once

class CLDSkinWnd : public CSkinWnd
{
public:
	CLDSkinWnd(void);
	~CLDSkinWnd(void);

	virtual void OnCommand(UINT uID, UINT nNotifyCode);

	virtual bool OnCustomCommand(int nId);

};
