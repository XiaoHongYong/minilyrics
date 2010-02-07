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
	void UpdateLicenseInfo();

	CCECommandBar	m_cmdBar;

};

