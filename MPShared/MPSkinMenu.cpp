// MPSkinMenu.cpp: implementation of the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"


typedef struct _CMDID_ENCODING
{
	int								nCmdId;
	CHAR_ENCODING	nEncodingId;
}_CMDID_ENCODING;

_CMDID_ENCODING	_arrCmdIdCharset[] = 
{
	{IDC_ENC_DEFAULT, ED_SYSDEF },
	{IDC_ENC_UNICODE, ED_UNICODE },
	{IDC_ENC_UTF8, ED_UTF8 },
	{IDC_ENC_BALTIC, ED_BALTIC_WINDOWS },
	{IDC_ENC_WEST_EUROPE, ED_WESTERN_EUROPEAN_WINDOWS},
	{IDC_ENC_CENTRAL_EUROPE, ED_CENTRAL_EUROPEAN_WINDOWS},
	{IDC_ENC_GREEK, ED_GREEK_WINDOWS },
	{IDC_ENC_CYRILLIC, ED_CYRILLIC_WINDOWS },
	{IDC_ENC_TURKISH, ED_TURKISH_WINDOWS },
	{IDC_ENC_VIETNAMESE, ED_VIETNAMESE },
	{IDC_ENC_KOREAN, ED_KOREAN },
	{IDC_ENC_ARABIC, ED_ARABIC },
	{IDC_ENC_HEBREW, ED_HEBREW_WINDOWS },
	{IDC_ENC_THAI, ED_THAI},
	{IDC_ENC_JAPANESE, ED_JAPANESE_SHIFT_JIS},
	{IDC_ENC_CHT, ED_BIG5 },
	{IDC_ENC_CHS, ED_GB2312 },
	{IDC_ENC_LATIN9_ISO, ED_LATIN9_ISO },
	{-1, ED_SYSDEF}
};

int CmdIdToEncoding(int nCmdId)
{
	for (int i = 0; i < CountOf(_arrCmdIdCharset); i++)
	{
		if (_arrCmdIdCharset[i].nCmdId == nCmdId)
		{
			return _arrCmdIdCharset[i].nEncodingId;
		}
	}

	return -1;
}

int EncodingIdToCmd(CHAR_ENCODING nEncodingId)
{
	for (int i = 0; i < CountOf(_arrCmdIdCharset); i++)
	{
		if (_arrCmdIdCharset[i].nEncodingId == nEncodingId)
		{
			return _arrCmdIdCharset[i].nCmdId;
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////

MenuItemCheck _menuItemsCheck[] = 
{
#ifdef _MPLAYER
	{ IDC_SHUFFLE, SZ_SECT_PLAYER, _T("shuffle"), SZ_TRUE, NULL, },
#endif
	{ IDC_SETTOPMOST, NULL, NULL, NULL, },
	{ IDC_ANTIAlIAS, SZ_SECT_UI, _T("Antialias"), _T("1"), },
	{ IDC_CLICK_THROUGH, NULL, NULL, _T("1"),  },
	{ IDC_SETTOPMOST, NULL, NULL, _T("1"),  },
	{ IDC_LDO_KARAOKE, NULL, NULL, _T("1"),  },
	{ IDC_FLOATING_LYRICS, NULL, NULL, _T("1"), },
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
	{ IDC_LDO_AUTO, NULL },
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
	CHAR_ENCODING	encoding = GetCharEncodingID(g_LyricData.Properties().GetEncoding());
	return EncodingIdToCmd(encoding);
}

MenuRadioGroup _menuRadioGroup[] = 
{
#ifdef _MPLAYER
	{ SZ_SECT_PLAYER, _T("repeat"), _MenuRadioLoop, NULL, 0, 0 },
#endif
	{ 0, 0, _MenuRadioLyrDrawOpt, },
	{ 0, 0, _MenuRadioCharEncoding, },
	{ 0, 0, _MenuRadioOpaque, },
	{ 0, 0, _MenuRadioLyrDisplayStyle, },
};

void CMenuAutoCheck::InitProcSubMenu(CMLMenu &menu)
{
	int				nInRadioGroupIndex = -1;
	MenuRadioGroup	radioGroup;

	for (int i = 0; i < 255; i++)
	{
		tstring strText;
		UINT nID;
		bool bSubMenu;
		
		if (!menu.GetMenuItemInfo(i, strText, nID, bSubMenu, true))
		{
			if (nInRadioGroupIndex != -1)
				m_vMenuItemsRadio.push_back(radioGroup);
			break;
		}
		
		if (bSubMenu)
		{
			if (nInRadioGroupIndex != -1)
			{
				m_vMenuItemsRadio.push_back(radioGroup);
				nInRadioGroupIndex = -1;
			}
			CMLMenu subMenu = menu.GetSubmenu(i);
			InitProcSubMenu(subMenu);
		}
		else
		{
			if (nID == IDC_SKIN_POS)
			{
				m_hInsertSkinsMenu = menu;
				m_nInsertSkinsPos = i;
				continue;
			}
			else if (nID == IDC_NEW_MENU_POS)
			{
				m_hInsertNewItemMenu = menu;
				m_nInsertNewItemPos = i;
				continue;
			}

			int		k;
			for (k = 0; k < CountOf(_menuItemsCheck); k++)
			{
				if (nID == _menuItemsCheck[k].nID)
				{
					if (nInRadioGroupIndex != -1)
					{
						m_vMenuItemsRadio.push_back(radioGroup);
						nInRadioGroupIndex = -1;
					}

					MenuItemCheck	item = _menuItemsCheck[k];

					item.menu = menu;
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
						if (_menuRadioGroup[k].idNames[n].nID == nID)
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
								radioGroup.menu = menu;
								radioGroup.nRadioStartID = nID;
								radioGroup.nRadioEndID = nID;
								nInRadioGroupIndex = k;
							}
							else
							{
								// next group item.
								radioGroup.nRadioEndID = nID;
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
			if (!item.menu.IsValid())
				continue;
			bool		bCheck;
			if (item.szSection == NULL || item.szName == NULL)
				bCheck = GetChecked(item.nID);
			else
				bCheck = _tcsicmp(item.szCheckValue, g_Profile.GetProfileString(item.szSection, item.szName, _T(""))) == 0;
			item.menu.CheckItem(item.nID, bCheck);
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
			if (!item.menu.IsValid())
				continue;
			
			if (item.nRadioStartID == IDC_ENC_DEFAULT)
			{
				// user function
				UINT		nID;
				nID = GetCharEncodingCheckMenuID();
				item.menu.CheckRadioItem(item.nRadioStartID, item.nRadioEndID, nID);
			}
			else
			{
				UINT		nIDChecked;
				if (GetRadioChecked(item.idNames, nIDChecked))
				{
					item.menu.CheckRadioItem(item.nRadioStartID, item.nRadioEndID, nIDChecked);
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

CMPSkinMenu::CMPSkinMenu()
{
}

CMPSkinMenu::~CMPSkinMenu()
{
}

void CMPSkinMenu::OnLoadMenu()
{
	m_autoCheckMenu.InitProcSubMenu(*this);
	if (m_autoCheckMenu.m_nInsertNewItemPos != -1)
	{
		SetOrgAppendPos(m_autoCheckMenu.m_nInsertNewItemPos);
		m_autoCheckMenu.m_hInsertNewItemMenu.RemoveItem(m_autoCheckMenu.m_nInsertNewItemPos);
	}

#ifdef _DEBUG
	AppendItem(3999, _T("Test"));
#endif
}

void CMPSkinMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	m_autoCheckMenu.UpdateMenuCheckStatus();

	if (m_autoCheckMenu.m_hInsertSkinsMenu.IsValid())
	{
		InsertSkinMenu(m_autoCheckMenu.m_hInsertSkinsMenu,
			m_autoCheckMenu.m_nInsertSkinsPos);
	}

	CSkinMenu::TrackPopupMenu(x, y, pWnd, prcNotOverlap);
}

void CMPSkinMenu::TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	m_autoCheckMenu.UpdateMenuCheckStatus();

	if (m_autoCheckMenu.m_hInsertSkinsMenu.IsValid())
	{
		InsertSkinMenu(m_autoCheckMenu.m_hInsertSkinsMenu,
			m_autoCheckMenu.m_nInsertSkinsPos);
	}

	CSkinMenu::TrackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}

bool CMPSkinMenu::GetShortcutKey(int nMenuID, tstring &strShortcut)
{
	int			nCmdID;
	nCmdID = CMPlayerApp::GetMPSkinFactory()->GetUIDByMenuID(nMenuID);
	if (nCmdID != UID_INVALID)
	{
		CMPlayerAppBase::GetHotkey().GetHotkeyText(nCmdID, strShortcut);
		return true;
	}

	return false;
}

static int		__vSkinMenuId[] = {IDC_SKIN_0, IDC_SKIN_1, IDC_SKIN_2, IDC_SKIN_3, IDC_SKIN_4, IDC_SKIN_5,
				IDC_SKIN_6, IDC_SKIN_7, IDC_SKIN_8, IDC_SKIN_9, IDC_SKIN_10, IDC_SKIN_11, IDC_SKIN_12, 
				IDC_SKIN_13, IDC_SKIN_14, IDC_SKIN_15, IDC_SKIN_16, IDC_SKIN_17, IDC_SKIN_18, IDC_SKIN_19,
				IDC_SKIN_20 };
#define __SkinMenuCount CountOf(__vSkinMenuId)

void CMPSkinMenu::InsertSkinMenu(CMLMenu &menuSkin, int nPosStart)
{
	// 取得上次加载的Skin
	tstring strDefaultSkin = CSkinApp::GetInstance()->GetDefaultSkin();

	// 取得原来的SKIN子菜单，并且删除之
	{
		for (int i = 0; i < 32; i++)
		{
			if (!menuSkin.HasItem(0))
				break;
			menuSkin.RemoveItem(0);
		}
	}

#ifdef _MINILYRICS_WIN32
	if (g_Player.IsSupportEmbedded())
	{
		menuSkin.AppendItem(IDC_WINAMP_SKIN, _TL(g_Player.GetEmbeddedSkinName()));
		menuSkin.AppendSeperator();

		if (CMPlayerAppBase::GetInstance()->IsEmbeddedMode())
			strDefaultSkin.clear();

		menuSkin.CheckItem(IDC_WINAMP_SKIN, CMPlayerAppBase::GetInstance()->IsEmbeddedMode());
	}
#endif

	// 查找所有的Skin，并且添加到菜单中
	vector<tstring>	vSkins;
	if (CMPlayerAppBase::GetMPSkinFactory()->EnumAllSkins(vSkins))
	{
		for (int i = 0; i < (int)vSkins.size() && i < __SkinMenuCount; i++)
		{
			menuSkin.AppendItem(__vSkinMenuId[i], vSkins[i].c_str());
			if (_tcsicmp(strDefaultSkin.c_str(), vSkins[i].c_str()) == 0)
				menuSkin.CheckItem(__vSkinMenuId[i], true);
		}
	}
}

bool OnCommandSkin(int nCmdId)
{
	int			i;

#ifdef _MINILYRICS_WIN32
	if (nCmdId == IDC_WINAMP_SKIN)
	{
		CMPlayerAppBase::GetInstance()->SetReloadEmbeddedTheme();
		g_Player.SwitchToEmbeddedSkin();
		return true;
	}
#endif

	for (i = 0; i < __SkinMenuCount; i++)
	{
		if (nCmdId == __vSkinMenuId[i])
		{
			break;
		}
	}
	if (i == __SkinMenuCount)
		return false;

	// 取得上次加载的Skin
	tstring strDefaultSkin = CSkinApp::GetInstance()->GetDefaultSkin();
#ifdef _MINILYRICS_WIN32
	if (CMPlayerAppBase::GetInstance()->IsEmbeddedMode())
		strDefaultSkin.clear();
#endif

	vector<tstring>	vSkins;
	tstring			strSkin;
	if (CMPlayerAppBase::GetMPSkinFactory()->EnumAllSkins(vSkins))
	{
		if (i >= (int)vSkins.size())
			return false;
	}

	strSkin = vSkins[i];
	if (_tcscmp(strSkin.c_str(), strDefaultSkin.c_str()) == 0)
		return true;

	CMPlayerAppBase::GetInstance()->ChangeSkinByUserCmd(strSkin.c_str());

	return true;
}

void CLyrEditorMenu::TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	if (nSubMenu == 2)
	{
		bool		bEnable;
		CSkinWnd	*pSkinWnd = (CSkinWnd *)pWnd;
		CMLMenu		menu = GetSubmenu(nSubMenu);

		CLyricShowTextEditObj *pEditor = (CLyricShowTextEditObj*)pSkinWnd->GetUIObjectByClassName(CLyricShowTextEditObj::ClassName());
		if (pEditor)
		{
			bEnable = pEditor->IsSelected();
			menu.EnableItem(IDC_EDIT_CUT, bEnable);
			menu.EnableItem(IDC_EDIT_COPY, bEnable);

			bEnable = pEditor->CanUndo();
			menu.EnableItem(IDC_EDIT_UNDO, bEnable);

			bEnable = pEditor->CanRedo();
			menu.EnableItem(IDC_EDIT_REDO, bEnable);
		}
	}

	CMPSkinMenu::TrackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}
