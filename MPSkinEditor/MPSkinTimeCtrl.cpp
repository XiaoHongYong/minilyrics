// MPSkinTimeCtrl.cpp: implementation of the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinTimeCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl, _T("PlayingTime"))

CMPSkinTimeCtrl::CMPSkinTimeCtrl()
{
	m_strText = _T("01:21");
}

CMPSkinTimeCtrl::~CMPSkinTimeCtrl()
{
}
