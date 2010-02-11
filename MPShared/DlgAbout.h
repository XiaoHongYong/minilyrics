#pragma once

#include "MPSkinDialog.h"

void ShowRegisterNotice(CBaseWnd *pWnd, BOOL bForceShow = FALSE);


class CDlgAbout : public CMPSkinDialog
{
public:
	CDlgAbout();
	~CDlgAbout();

};

