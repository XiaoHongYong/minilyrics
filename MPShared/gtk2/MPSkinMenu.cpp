// MPSkinMenu.cpp: implementation of the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "CharsetEncoding.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinMenu::CMPSkinMenu()
{

}

CMPSkinMenu::~CMPSkinMenu()
{

}

void CMPSkinMenu::OnLoadMenu()
{
}


void CMPSkinMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd)
{

	CSkinMenu::TrackPopupMenu(x, y, pWnd);
}
