// MPSkinTimeCtrl.cpp: implementation of the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Player.h"
#include "MPSkinTimeCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinPicText>, _T("PlayingTime"))

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinStaticText>, _T("PlayingTimeText"))
