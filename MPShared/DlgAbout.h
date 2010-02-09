#pragma once

#include "MPSkinDialog.h"

class CDlgAbout : public CMPSkinDialog
{
public:
	CDlgAbout();
	~CDlgAbout();

	virtual void OnSkinLoaded();

	virtual void CloseSkin();

	virtual bool OnCustomCommand(int nId);

	void OnEvent(const IEvent *pEvent);

protected:
	bool OnCheckRegCode();
	void UpdateRegCode();

	void UpdateLicenseInfo();

	void OnAnimationDone(int nID);


	CCECommandBar	m_cmdBar;

};

