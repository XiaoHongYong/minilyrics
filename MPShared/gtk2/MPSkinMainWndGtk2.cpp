// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../MPlayerApp.h"
#include "../MPSkinMainWnd.h"
#include "MPSkinMainWndGtk2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinMainWnd::CMPSkinMainWnd()
{
}

CMPSkinMainWnd::~CMPSkinMainWnd()
{
}

bool CMPSkinMainWnd::OnCreate()
{
	if (!CMPSkinMainWndBase::OnCreate())
		return false;

	return true;
}

void CMPSkinMainWnd::OnDestroy()
{
	CMPSkinMainWndBase::OnDestroy();
}

void CMPSkinMainWnd::OnEvent(const IEvent *pEvent)
{
	CMPSkinWnd::OnEvent(pEvent);
}

