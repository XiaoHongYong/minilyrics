#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageUI.h"
#include "PreferenceDlg.h"

CPPageUI::CPPageUI() : CPreferencePageBase(IDD_PAGE_UI_ROOT, _T("User Interface"))
{
	m_PageID = PAGE_UI_ROOT;
}

CPPageUI::~CPPageUI()
{
}

BOOL CPPageUI::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("topmost"), true, IDC_ALWAYS_ON_TOP);

#ifndef _MPLAYER
	if (!g_Player.IsFeatureSupported(PF_MinimizeRestoreWithPlayer))
	{
		SetDlgItemText(IDC_ACTIVATE_WITH_WINAMP, TOLOCAL(_T("Activate, minimize and restore with player window (Unsupported)")));
		EnableDlgItem(IDC_ACTIVATE_WITH_WINAMP, false);
	}
	else
		AddOptBool(ET_NULL, SZ_SECT_UI, _T("ActivateWithWinamp"), true, IDC_ACTIVATE_WITH_WINAMP);
#endif

#ifdef _WIN32_DESKTOP
	// 半透明值
	int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WndOpaquePercent"), 70);
	if (nOpaque < 10)
		nOpaque = 10;
	else if (nOpaque > 100)
		nOpaque = 100;

	if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("WndOpacity"), true))
		CheckButton(IDC_SET_OPAQUE, TRUE);
	else
		EnableDlgItem(IDC_OPAQUE, FALSE);
	// 
	// 	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("TransparentBkColor"), FALSE))
	// 	{
	// 		CheckButton(IDC_C_TRANSPT_BK, TRUE);
	// 	}

	// Click though...
	CheckButton(IDC_CLICK_THROUGH, g_mpApp.GetSkinFactory()->GetClickThrough());

	if (!IsLayeredWndSupported())
	{
		EnableDlgItem(IDC_OPAQUE, FALSE);
		EnableDlgItem(IDC_SET_OPAQUE, FALSE);
		EnableDlgItem(IDC_C_TRANSPT_BK, FALSE);
		EnableDlgItem(IDC_CLICK_THROUGH, FALSE);
	}

	SetDlgItemInt(IDC_OPAQUE, nOpaque, TRUE);
#endif

	InitCheckButtons();

	return TRUE;
}

void CPPageUI::OnCommand(UINT uID, UINT nNotifyCode)
{
	CPreferencePageBase::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_ALWAYS_ON_TOP:
		{
			bool		bValue = IsButtonChecked(uID);

			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("topmost"), bValue);
			g_mpApp.GetSkinFactory()->TopmostAll(bValue);
		}
		break;
#ifdef _WIN32_DESKTOP
	case IDC_OPAQUE:
		{
			if (nNotifyCode == EN_CHANGE)
			{
				// GetCurrent Opaque value
				BOOL	bTranslated;
				int		nOpaque;
				nOpaque = GetDlgItemInt(IDC_OPAQUE, &bTranslated, FALSE);

				if (bTranslated)
				{
					g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WndOpaquePercent"), nOpaque);
					g_mpApp.GetSkinFactory()->AllUpdateTransparent();
				}
			}
		}
		break;
	case IDC_SET_OPAQUE:
		{
			bool		bTransparent;

			bTransparent = IsButtonChecked(IDC_SET_OPAQUE);

			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WndOpacity"), !bTransparent);
			EnableDlgItem(IDC_OPAQUE, bTransparent);

			g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		}
		break;
		// 	case IDC_C_TRANSPT_BK:
		// 		g_Profile.WriteProfileInt(SZ_SECT_UI, _T("TransparentBkColor"), IsButtonChecked(IDC_C_TRANSPT_BK));
		// 		g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		// 		break;
	case IDC_CLICK_THROUGH:
		g_mpApp.GetSkinFactory()->SetClickThrough(IsButtonChecked(IDC_CLICK_THROUGH));
		g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		break;
#endif
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

CPPageLocalization::CPPageLocalization() : CPreferencePageBase(IDD_PAGE_LOCALIZATION, _T("Localization"))
{
	m_PageID = PAGE_LOCALIZATION;
}

CPPageLocalization::~CPPageLocalization()
{
}

BOOL CPPageLocalization::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	UpdateLanguageInfo();

	return TRUE;
}

void CPPageLocalization::OnDestroy()
{
	m_LangDlgHelper.Destroy();

	m_menuLang.Destroy();
	m_vTransFiles.clear();

	CPreferencePageBase::OnDestroy();
}

const int	_CMD_ID_LANGUANG_BEGIN	= 2000;

void CPPageLocalization::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_SEL_LANGUAGE:

		//
		// 创建快捷菜单, 显示可以使用的语言包
		//
		{
			int					nCmd;
			RECT				rc;

			m_menuLang.Destroy();

			m_menuLang.CreatePopupMenu();

			nCmd = _CMD_ID_LANGUANG_BEGIN;
			m_vTransFiles.clear();

			CLanguageTool::ListTransFiles(GetWorkingFolder(), m_vTransFiles);

			// search for full match
			for (CLanguageTool::V_TRANSFILEINFO::iterator it = m_vTransFiles.begin(); 
				it != m_vTransFiles.end(); it++)
			{
				CLanguageTool::TransFileInfo	&item = *it;

				m_menuLang.AppendItem(nCmd, item.strLanguage.c_str());
				nCmd++;
			}

			GetDlgItemRect(IDC_SEL_LANGUAGE, &rc);

			m_menuLang.TrackPopupMenu(rc.left, rc.top, this);
		}
		break;
	default:
		//
		// 选择了一个语言包, 则立即切换到该语言
		//
		if (uID >= _CMD_ID_LANGUANG_BEGIN && 
			uID < _CMD_ID_LANGUANG_BEGIN + m_vTransFiles.size())
		{
			TCHAR	szLanFileName[MAX_PATH] = _T("");

			if (g_Profile.GetProfileString(_T("Language"), _T(""), szLanFileName, MAX_PATH))
			{
				if (m_vTransFiles[uID - _CMD_ID_LANGUANG_BEGIN].strFileName == szLanFileName)
					break;
			}

			g_Profile.WriteProfileString(_T("Language"), m_vTransFiles[uID - _CMD_ID_LANGUANG_BEGIN].strFileName.c_str());

			g_mpApp.OnLanguageChanged();
			g_wndPreference.OnLanguageChanged();

			UpdateLanguageInfo();
		}
		break;
	}
}

void CPPageLocalization::UpdateLanguageInfo()
{
	tstring		strLangFile;
	tstring		strInfo;

	if (CLanguageTool::GetCurrentLanguageFile(strLangFile))
	{
		CProfile	profile;

		profile.Init(strLangFile.c_str());
		profile.DoCache();

		strInfo = TOLOCAL(_T("Translator:"));
		strInfo += _T(" ");
		strInfo += profile.GetProfileString(_T("Info"), _T("Translator"), _T(""));
		strInfo += _T("\r\n");
		strInfo += TOLOCAL(_T("Contact:"));
		strInfo += _T(" ");
		strInfo += profile.GetProfileString(_T("Info"), _T("Contact"), _T(""));
	}

	SetDlgItemText(IDC_E_LANGUAGE_INFO, strInfo.c_str());
}

//////////////////////////////////////////////////////////////////////////

#define TIMER_ID_SKIN_SEL_CHANGED		1

CPreferPageSkin::CPreferPageSkin() : CPreferencePageBase(IDD_PAGE_SKIN, _T("Skins"))
{
	m_PageID = PAGE_SKINS;
}

CPreferPageSkin::~CPreferPageSkin()
{
}

BOOL CPreferPageSkin::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_hlDlSkin.Attach(this, IDC_MORE_SKINS, GetStrName(SN_HTTP_DLSKIN));
	m_listSkin.Attach(this, IDC_SKIN_LIST);

	UpdateSkinList();

	return TRUE;
}

void CPreferPageSkin::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == TIMER_ID_SKIN_SEL_CHANGED)
	{
		TCHAR		szSkin[MAX_PATH];
		int			nIndex;

		KillTimer(TIMER_ID_SKIN_SEL_CHANGED);

		// 取得当前选择的skin
		nIndex = m_listSkin.GetCurSel();

		m_listSkin.GetText(nIndex, szSkin);

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
		if (_tcsicmp(szSkin, m_strEmbeddedSkinName.c_str()) == 0)
		{
			// 嵌入到player 中的skin
			g_mpApp.SwitchToEmbeddedSkinMode(true);
		}
		else
#endif
		{
			g_mpApp.ChangeSkin(szSkin, true);
		}
		g_wndPreference.ActivateWindow();

		UpdateSkinComment(szSkin);
	}
}

void CPreferPageSkin::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_SKIN_LIST:
		if (nNotifyCode == LBN_SELCHANGE)
		{
			KillTimer(TIMER_ID_SKIN_SEL_CHANGED);
			// 设置延时时间为300ms，即在300ms后，用户的选择才会生效
			SetTimer(TIMER_ID_SKIN_SEL_CHANGED, 300);
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}

void CPreferPageSkin::UpdateSkinList()
{
	TCHAR		szSkinDef[MAX_PATH];

	m_listSkin.ResetContent();

	// 取得上次加载的Skin
	g_Profile.GetProfileString(_T("DefaultSkin"), _T(""), szSkinDef, CountOf(szSkinDef));

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
	m_strEmbeddedSkinName = _T("");
	if (g_mpApp.IsSupportEmbedded())
	{
		TCHAR		szName[256];

		g_Player.GetEmbeddedSkinName(szName, CountOf(szName));
		m_strEmbeddedSkinName = tstring(_T("<< ")) + szName + _T(" >>");
		m_listSkin.AddString(m_strEmbeddedSkinName.c_str());
	}
#endif

	// 查找所有的Skin，并且添加到菜单中
	vector<tstring>		vSkins;
	if (g_mpApp.GetSkinFactory()->EnumAllSkins(vSkins))
	{
		for (UINT i = 0; i < vSkins.size(); i++)
		{
			m_listSkin.AddString(vSkins[i].c_str());
		}
		UpdateSkinComment(szSkinDef);
	}

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
	if (g_mpApp.IsEmbeddedMode())
		m_listSkin.SelectString(-1, m_strEmbeddedSkinName.c_str());
	else
#endif
		m_listSkin.SelectString(-1, szSkinDef);
}

BOOL CPreferPageSkin::UpdateSkinComment(LPCTSTR szSkin)
{
	TCHAR		szReadme[MAX_PATH] = _T("");
	tstringex	str;

	// 设置Skin的说明
	if (g_mpApp.GetSkinFactory()->GetResourceMgr()->GetResourcePathName(szReadme, CountOf(szReadme), _T("Readme.txt"))
		&& OpenFileToTString(szReadme, str))
	{
		SetDlgItemText(IDC_SKIN_COMMENT, str.c_str());
		return TRUE;
	}

	SetDlgItemText(IDC_SKIN_COMMENT, _T(""));
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

CPPageUIAdvanced::CPPageUIAdvanced() : CPreferencePageBase(IDD_PAGE_UI_ADVANCED, _T("Advanced"))
{
	m_PageID = PAGE_UI_ROOT;
}

CPPageUIAdvanced::~CPPageUIAdvanced()
{
}

BOOL CPPageUIAdvanced::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndWidth"), false, IDC_ATUO_WIDTH);

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndHeight"), true, IDC_ATUO_HEIGHT);

#ifndef _MPLAYER
	// 自动隐藏迷你歌词如果没有歌词
	AddOptBool(ET_NULL, SZ_SECT_UI, _T("AutoHideIfNoLyrics"), false, IDC_AUTO_HIDE_NO_LYRICS2);

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true, IDC_AUTO_HIDE_ON_MOVIE);
#endif

	AddOptBool(ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, _T("HideToolbar"), false, IDC_HIDE_TOOLBAR);

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("HideRateLink"), true, IDC_HIDE_RATE_LINK);

	AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("EnableAdjustVertAlign"), false, IDC_C_ENABLE_ADJUST_VERT_LYR_POS);

	InitCheckButtons();

	return TRUE;
}

void CPPageUIAdvanced::OnCommand(UINT uID, UINT nNotifyCode)
{
	CPreferencePageBase::OnCommand(uID, nNotifyCode);

	ProcessCommand(uID, nNotifyCode);
}
