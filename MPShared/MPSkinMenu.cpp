// MPSkinMenu.cpp: implementation of the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "CharsetEncoding.h"


MenuItemCheck _menuItemsCheck[] = 
{
#ifdef _MPLAYER
	{ IDC_SHUFFLE, SZ_SECT_PLAYER, _T("shuffle"), SZ_TRUE, NULL, },
#endif
	{ IDC_SETTOPMOST, NULL, NULL, NULL, NULL, },
	{ IDC_ANTIAlIAS, SZ_SECT_UI, _T("Antialias"), _T("1"), NULL, },
	{ IDC_ENABLE_BK_IMAGE, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), _T("1"), NULL, },
	{ IDC_USE_TRANSPT_BK_COLOR, NULL, NULL, _T("1"), NULL, },
	{ IDC_CLICK_THROUGH, NULL, NULL, _T("1"), NULL, },
	{ IDC_SETTOPMOST, NULL, NULL, _T("1"), NULL, },
	{ IDC_LDO_KARAOKE, NULL, NULL, _T("1"), NULL, },
	{ IDC_FLOATING_LYRICS, NULL, NULL, _T("1"), NULL, },
	{ IDC_CLICK_THROUGH_FLOATING_LYR, SZ_SECT_FLOATING_LYR, _T("ClickThrough"), _T("1"), NULL, },
};

#ifdef _MPLAYER
MenuRadioGroupIDName	_MenuRadioLoop[] = 
{
	{ IDC_REPEAT_OFF, _T("off") },
	{ IDC_REPEAT_ALL, _T("all") },
	{ IDC_REPEAT_TRACK, _T("track") },
	{ 0, NULL },
};
#endif

// MenuRadioGroupIDName	_MenuRadioLyrAlign[] = 
// {
// 	{ IDC_ALIGN_LYRICS_LEFT, _T("left") },
// 	{ IDC_ALIGN_LYRICS_CENTER, _T("center") },
// 	{ IDC_ALIGN_LYRICS_RIGHT, _T("right") },
// 	{ 0, NULL },
// };
// 
// MenuRadioGroupIDName	_MenuRadioLyrDisplayStyle[] = 
// {
// 	{ IDC_LDS_MULTI_LINE, CLyricShowMultiRowObj::ClassName() },
// 	{ IDC_LDS_STATIC_TXT, CLyricShowTxtObj::ClassName() },
// 	{ IDC_LDS_TWO_LINE, CLyricShowTwoRowObj::ClassName() },
// 	{ IDC_LDS_HORZ_SCROLL, CLyricShowHorzScrollObj::ClassName() },
// 	{ IDC_LDS_SINGLE_LINE, CLyricShowSingleRowObj::ClassName() },
// 	{ IDC_LDS_VOBSUB, CLyricShowVobSub::ClassName() },
// 	{ 0, NULL },
// };
// 
// MenuRadioGroupIDName	_MenuRadioLyrDrawOpt[] = 
// {
// 	{ IDC_LDO_NORMAL, _T("normal") },
// 	{ IDC_LDO_FADE_IN, _T("fadein") },
// 	{ IDC_LDO_FADEOUT_BG, _T("fadeout") },
// 	{ 0, NULL },
// };

MenuRadioGroupIDName	_MenuRadioLyrAlign[] = 
{
	{ IDC_ALIGN_LYRICS_LEFT, NULL },
	{ IDC_ALIGN_LYRICS_CENTER, NULL },
	{ IDC_ALIGN_LYRICS_RIGHT, NULL },
	{ 0, NULL },
};

MenuRadioGroupIDName	_MenuRadioLyrDisplayStyle[] = 
{
	{ IDC_LDS_MULTI_LINE, NULL },
	{ IDC_LDS_STATIC_TXT, NULL },
	{ IDC_LDS_TWO_LINE, NULL },
	{ IDC_LDS_HORZ_SCROLL, NULL },
	{ IDC_LDS_SINGLE_LINE, NULL },
	{ IDC_LDS_VOBSUB, NULL },
	{ 0, NULL },
};

MenuRadioGroupIDName	_MenuRadioLyrDrawOpt[] = 
{
	{ IDC_LDO_NORMAL, NULL },
	{ IDC_LDO_FADE_IN, NULL },
	{ IDC_LDO_FADEOUT_BG, NULL },
	{ 0, NULL },
};

MenuRadioGroupIDName	_MenuRadioCharEncoding[] = 
{
	{ IDC_ENC_DEFAULT, _T("") },
	{ IDC_ENC_ARABIC, _T("") },
	{ IDC_ENC_BALTIC, _T("") },
	{ IDC_ENC_CYRILLIC, _T("") },
	{ IDC_ENC_CENTRAL_EUROPE, _T("") },
	{ IDC_ENC_WEST_EUROPE, _T("") },
	{ IDC_ENC_GREEK, _T("") },
	{ IDC_ENC_HEBREW, _T("") },
	{ IDC_ENC_JAPANESE, _T("") },
	{ IDC_ENC_KOREAN, _T("") },
	{ IDC_ENC_LATIN9_ISO, _T("") },
	{ IDC_ENC_CHS, _T("") },
	{ IDC_ENC_CHT, _T("") },
	{ IDC_ENC_THAI, _T("") },
	{ IDC_ENC_TURKISH, _T("") },
	{ IDC_ENC_UNICODE, _T("") },
	{ IDC_ENC_UTF8, _T("") },
	{ IDC_ENC_VIETNAMESE, _T("") },
	{ 0, NULL },
};

MenuRadioGroupIDName	_MenuRadioOpaque[] = 
{
	{ IDC_SET_OPAQUE_100, NULL },
	{ IDC_SET_OPAQUE_90, NULL },
	{ IDC_SET_OPAQUE_80, NULL },
	{ IDC_SET_OPAQUE_70, NULL },
	{ IDC_SET_OPAQUE_60, NULL },
	{ IDC_SET_OPAQUE_50, NULL },
	{ IDC_SET_OPAQUE_40, NULL },
	{ IDC_SET_OPAQUE_30, NULL },
	{ IDC_SET_OPAQUE_20, NULL },
	{ IDC_SET_OPAQUE_10, NULL },
	{ 0, NULL },
};

typedef UINT(*FUNGetCheckMenuID)();

static UINT GetCharEncodingCheckMenuID()
{
	ENCODING_CODEPAGE		&encoding = GetCharEncodingByName(g_LyricData.GetEncoding());
	_CMDID_ENCODING		&cmdIdEncoding = EncodingIdToCmd(encoding.nEncodingId);

	return cmdIdEncoding.nCmdId;
}

MenuRadioGroup _menuRadioGroup[] = 
{
#ifdef _MPLAYER
	{ SZ_SECT_PLAYER, _T("repeat"), _MenuRadioLoop, NULL, 0, 0 },
#endif
	{ NULL, NULL, _MenuRadioLyrAlign, NULL, 0, 0 },
	{ NULL, NULL, _MenuRadioLyrDrawOpt, NULL, 0, 0 },
	{ NULL, (LPCTSTR)GetCharEncodingCheckMenuID, _MenuRadioCharEncoding, NULL, 0, 0 },
	{ NULL, NULL, _MenuRadioOpaque, NULL, 0, 0 },
	{ NULL, NULL, _MenuRadioLyrDisplayStyle, NULL, 0, 0 },
};

void CMenuAutoCheck::InitProcSubMenu(HMENU hMenu)
{
	MENUITEMINFO	mii;
	UINT			nIDStart = -1;
	int				nInRadioGroupIndex = -1;
	MenuRadioGroup	radioGroup;

	// don't use get menu item count, under win ce, it will cost a lot time
	for (int i = 0; i < 256; i++)
	{
		memset(&mii, 0, sizeof(mii));

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
		mii.dwTypeData = NULL;
		mii.cch = 0;

		// get menu item by pos.
		if (!GetMenuItemInfo(hMenu, i, TRUE, &mii))
		{
			if (nInRadioGroupIndex != -1)
				m_vMenuItemsRadio.push_back(radioGroup);
			break;
		}

		if (mii.hSubMenu != NULL)
		{
			if (nInRadioGroupIndex != -1)
			{
				m_vMenuItemsRadio.push_back(radioGroup);
				nInRadioGroupIndex = -1;
			}
			InitProcSubMenu(mii.hSubMenu);
		}
		else
		{
			int		k;
			for (k = 0; k < CountOf(_menuItemsCheck); k++)
			{
				if (mii.wID == _menuItemsCheck[k].nID)
				{
					if (nInRadioGroupIndex != -1)
					{
						m_vMenuItemsRadio.push_back(radioGroup);
						nInRadioGroupIndex = -1;
					}

					MenuItemCheck	item = _menuItemsCheck[k];

					item.hMenu = hMenu;
					m_vMenuItemsCheck.push_back(item);
					break;
				}
			}
			if (k >= CountOf(_menuItemsCheck))
			{
				for (int k = 0; k < CountOf(_menuRadioGroup); k++)
				{
					for (int n = 0; _menuRadioGroup[k].idNames[n].nID != 0; n++)
					{
						if (_menuRadioGroup[k].idNames[n].nID == mii.wID)
						{
							if (nInRadioGroupIndex != -1 && nInRadioGroupIndex != k)
							{
								m_vMenuItemsRadio.push_back(radioGroup);
								nInRadioGroupIndex = -1;
							}

							if (nInRadioGroupIndex == -1)
							{
								// first group item.
								radioGroup = _menuRadioGroup[k];
								radioGroup.hMenu = hMenu;
								radioGroup.nRadioStartID = mii.wID;
								radioGroup.nRadioEndID = mii.wID;
								nInRadioGroupIndex = k;
							}
							else
							{
								// next group item.
								radioGroup.nRadioEndID = mii.wID;
							}
							k = CountOf(_menuRadioGroup);
							break;
						}
					}
				}
			}
		}
	}
}

void CMenuAutoCheck::UpdateMenuCheckStatus()
{
	{
		//
		// Update check item status.
		//
		LIST_MENU_ITEMS_CHECK::iterator	it, itEnd;
		itEnd = m_vMenuItemsCheck.end();
		for (it = m_vMenuItemsCheck.begin(); it != itEnd; ++it)
		{
			MenuItemCheck	&item = *it;
			if (!item.hMenu)
				continue;
			bool		bCheck;
			if (item.szSection == NULL || item.szName == NULL)
				bCheck = GetChecked(item.nID);
			else
				bCheck = _tcsicmp(item.szCheckValue, g_Profile.GetProfileString(item.szSection, item.szName, _T(""))) == 0;
			::CheckMenuItem(item.hMenu, item.nID, bCheck ? MF_CHECKED : MF_UNCHECKED);
		}
	}

	{
		//
		// Update radio item status.
		//
		tstring				strValue;
		LIST_MENU_ITEMS_RADIO::iterator	it, itEnd;
		itEnd = m_vMenuItemsRadio.end();
		for (it = m_vMenuItemsRadio.begin(); it != itEnd; ++it)
		{
			MenuRadioGroup	&item = *it;
			if (!item.hMenu)
				continue;
			if (item.szSection)
			{
				strValue = g_Profile.GetProfileString(item.szSection, item.szName, _T(""));
				for (int i = 0; item.idNames[i].szCheckValue != NULL; i++)
				{
					if (_tcsicmp(item.idNames[i].szCheckValue, strValue.c_str()) == 0)
					{
						::CheckMenuRadioItem(item.hMenu, item.nRadioStartID, item.nRadioEndID, 
							item.idNames[i].nID, MF_BYCOMMAND);
						break;
					}
				}
			}
			else
			{
				if (item.szName)
				{
					// user function
					FUNGetCheckMenuID		fun = (FUNGetCheckMenuID)item.szName;
					UINT		nID;
					nID = (*fun)();
					::CheckMenuRadioItem(item.hMenu, item.nRadioStartID, item.nRadioEndID, 
						nID, MF_BYCOMMAND);
				}
				else
				{
					UINT		nIDChecked;
					if (GetRadioChecked(item.idNames, nIDChecked))
					{
						::CheckMenuRadioItem(item.hMenu, item.nRadioStartID, item.nRadioEndID, 
							nIDChecked, MF_BYCOMMAND);
					}
				}
			}
		}
	}
}

void CMenuAutoCheck::AddUICheckStatusIf(IUICheckStatus *pIfUICheckStatus)
{
	m_listUICheckStatusIf.push_back(pIfUICheckStatus);
}

bool CMenuAutoCheck::GetChecked(UINT nID)
{
	LIST_UI_CHECK_STATUS::iterator	it, itEnd;

	itEnd = m_listUICheckStatusIf.end();
	for (it = m_listUICheckStatusIf.begin(); it != itEnd; ++it)
	{
		IUICheckStatus		*p = *it;
		bool		bChecked;
		if (p->GetChecked(nID, bChecked))
			return bChecked;
	}
	return false;
}

bool CMenuAutoCheck::GetRadioChecked(MenuRadioGroupIDName *idNames, UINT &nIDChecked)
{
	LIST_UI_CHECK_STATUS::iterator	it, itEnd;

	vector<UINT>	vIDs;

	while (idNames->nID != 0)
	{
		vIDs.push_back(idNames->nID);
		idNames++;
	}

	itEnd = m_listUICheckStatusIf.end();
	for (it = m_listUICheckStatusIf.begin(); it != itEnd; ++it)
	{
		IUICheckStatus		*p = *it;
		if (p->GetRadioChecked(vIDs, nIDChecked))
			return true;
	}
	return false;
}

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
	m_autoCheckMenu.InitProcSubMenu(m_hMenu);
}


void CMPSkinMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd)
{
	m_autoCheckMenu.UpdateMenuCheckStatus();

	CSkinMenu::TrackPopupMenu(x, y, pWnd);
}
