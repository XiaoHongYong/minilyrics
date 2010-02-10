#pragma once

#include "MPSkinDialog.h"

class CDlgAbout : public CMPSkinDialog
{
public:
	CDlgAbout(bool bEnterCode = false);
	~CDlgAbout();

	virtual void OnSkinLoaded();

	virtual void CloseSkin();

	virtual bool OnCustomCommand(int nId);

	void OnEvent(const IEvent *pEvent);

	virtual void OnOK();
	virtual void OnCancel();

protected:
	bool OnCheckRegCode();
	void UpdateRegCode();

	void UpdateLicenseInfo();

	void OnAnimationDone(int nID);

	void HideCodeShowAbout();

	bool			m_bEnterCode;

};

