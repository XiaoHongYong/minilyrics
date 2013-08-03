// MPSkinTimeCtrl.cpp: implementation of the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Player.h"
#include "MPSkinTimeCtrl.h"

tstring FormatTime(int nTimeSec)
{
	TCHAR		szTime[64];
	int			nHour, nMinute, nSec;
	
	nSec = nTimeSec % 60;
	nMinute = nTimeSec / 60;
	nHour = nMinute / 60;
	nMinute = nMinute % 60;
	if (nHour > 0)
		_stprintf_s(szTime, CountOf(szTime), _T("%02d:%02d:%02d"), nHour, nMinute, nSec);
	else
		_stprintf_s(szTime, CountOf(szTime), _T("%02d:%02d"), nMinute, nSec);
	
	return szTime;
}

//////////////////////////////////////////////////////////////////////

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinPicText>, _T("PlayingTime"))

template<>
UIOBJECT_CLASS_NAME_IMP(CMPSkinTimeCtrl<CSkinStaticText>, _T("PlayingTimeText"))
