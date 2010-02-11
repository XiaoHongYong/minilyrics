#pragma once

#include "MPSkinDialog.h"

class CDlgAbout : public CMPSkinDialog
{
public:
	CDlgAbout(bool bEnterCode = false);
	~CDlgAbout();

	virtual void OnSkinLoaded();

	virtual void CloseSkin();

protected:
	bool			m_bEnterCode;

};

