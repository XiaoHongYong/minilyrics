#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageUI.h"
#include "PreferenceDlg.h"

//////////////////////////////////////////////////////////////////////////

CPPageUI::CPPageUI() : CPreferencePageBase(IDD_PAGE_UI_ROOT, _T("User Interface"))
{
}

CPPageUI::~CPPageUI()
{
}

BOOL CPPageUI::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("topmost"), true, IDC_ALWAYS_ON_TOP);

#ifdef _WIN32_DESKTOP
	// Click though...
	CheckButton(IDC_CLICK_THROUGH, g_mpApp.GetSkinFactory()->GetClickThrough());
#endif

	InitCheckButtons();

	UpdateLanguageInfo();

	return TRUE;
}

void CPPageUI::OnDestroy()
{
	m_LangDlgHelper.Destroy();

	m_menuLang.Destroy();
	m_vTransFiles.clear();

	CPreferencePageBase::OnDestroy();
}

const int	_CMD_ID_LANGUANG_BEGIN	= 3000;

void CPPageUI::OnCommand(UINT uID, UINT nNotifyCode)
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

			CLanguageTool::ListTransFiles(NULL, m_vTransFiles);

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
	case IDC_ALWAYS_ON_TOP:
		{
			bool		bValue = IsButtonChecked(uID);

			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("topmost"), bValue);
			g_mpApp.GetSkinFactory()->TopmostAll(bValue);
		}
		break;
#ifdef _WIN32_DESKTOP
	case IDC_CLICK_THROUGH:
		g_mpApp.GetSkinFactory()->SetClickThrough(IsButtonChecked(IDC_CLICK_THROUGH));
		g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		break;
#endif
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
		else
			ProcessCommand(uID, nNotifyCode);
		break;
	}
}

void CPPageUI::UpdateLanguageInfo()
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

#ifndef _MPLAYER
		if (_tcsicmp(szSkin, m_strEmbeddedSkinName.c_str()) == 0)
		{
			// 嵌入到player 中的skin
			g_mpApp.SwitchToEmbeddedSkinMode();
		}
		else
#endif
		{
#ifndef _MPLAYER
			g_mpApp.EndEmbeddedSkin();
#endif
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
