#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageUI.h"
#include "PreferenceDlg.h"
#include "SkinListCtrl.h"


//////////////////////////////////////////////////////////////////////////
const int	_CMD_ID_LANGUANG_BEGIN	= 3000;
#define TIMER_ID_SKIN_SEL_CHANGED		1


class CPagePfUI : public CPagePfBase
{
public:
	CPagePfUI() : CPagePfBase(GetPageID(), PAGE_UI, _T("CMD_UI"))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_UI_LANGUAGE"); }

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();
		AddOptBool(ET_NULL, SZ_SECT_UI, _T("topmost"), true, _T("CID_C_ON_TOP"));

#ifdef _WIN32_DESKTOP
		// Click though...
		CStrPrintf str(_T("%d"), g_mpApp.GetSkinFactory()->GetClickThrough());
		SetUIObjectProperty(_T("CID_C_CLICK_THROUGH"), _T("CurStatus"), str.c_str());
#endif

		InitCheckButtons();

		UpdateLanguageInfo();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_C_ON_TOP")))
		{
			bool	bValue = IsButtonChecked(nId);
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("topmost"), bValue);
			g_mpApp.GetSkinFactory()->TopmostAll(bValue);
		}
#ifdef _WIN32_DESKTOP
		else if (nId == GetIDByName(_T("CID_C_CLICK_THROUGH")))
		{
			g_mpApp.GetSkinFactory()->SetClickThrough(IsButtonChecked(nId));
			g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		}
#endif
		else if (nId == GetIDByName(_T("CID_SEL_LANGUAGE")))
		{
			//
			// 创建快捷菜单, 显示可以使用的语言包
			//
			int					nCmd;
			CRect				rc;

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

			if (GetUIObjectRect(GetIDByName(_T("CID_SEL_LANGUAGE")), rc))
				m_pSkinWnd->ClientToScreen(rc);

			m_menuLang.TrackPopupMenu(rc.left, rc.top, m_pSkinWnd);
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

	virtual bool OnCommand(int nId)
	{
		if (CPagePfBase::OnCommand(nId))
			return true;

		//
		// 选择了一个语言包, 则立即切换到该语言
		//
		if (nId >= _CMD_ID_LANGUANG_BEGIN && 
			nId < _CMD_ID_LANGUANG_BEGIN + m_vTransFiles.size())
		{
			TCHAR	szLanFileName[MAX_PATH] = _T("");

			if (g_Profile.GetProfileString(_T("Language"), _T(""), szLanFileName, MAX_PATH))
			{
				if (m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName == szLanFileName)
					return true;
			}

			g_Profile.WriteProfileString(_T("Language"), m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName.c_str());

			g_mpApp.OnLanguageChanged();

			UpdateLanguageInfo();

			InvalidateRect();

			return true;
		}

		return false;
	}

protected:
	void UpdateLanguageInfo()
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

		SetUIObjectText(_T("CID_LANGUAGE_INFO"), strInfo.c_str(), true);
	}

protected:
	CMLMenu			m_menuLang;

	// 每一个菜单项对应的文件
	CLanguageTool::V_TRANSFILEINFO	m_vTransFiles;

};

//////////////////////////////////////////////////////////////////////////

class CPagePfSkin : public CPagePfBase
{
public:
	CPagePfSkin() : CPagePfBase(GetPageID(), PAGE_SKINS, _T("CMD_SKINS"))
	{
		m_pSkinList = NULL;
		m_bIgnoreSkinListNotify = false;
		CID_SKIN_LIST = 0;
	}

	static LPCTSTR GetPageID() { return _T("CID_C_UI_SKIN"); }

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		m_bIgnoreSkinListNotify = true;

		CID_SKIN_LIST = GetIDByName(_T("CID_SKIN_LIST"));
		m_pSkinWnd->RegisterUIObjNotifyHandler(CID_SKIN_LIST, this);
		m_pSkinList = (CSkinListCtrl *)GetUIObjectById(CID_SKIN_LIST, CSkinListCtrl::ClassName());
		if (m_pSkinList)
			m_pSkinList->AddColumn(_T("Skins"), 200);

		m_bIgnoreSkinListNotify = false;

		UpdateSkinList();
	}

	void OnDestroy()
	{
		m_pSkinWnd->UnregisterUIObjNotifyHandler(this);

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		return CPagePfBase::OnCustomCommand(nId);
	}

	bool OnTimer(UINT nTimerId)
	{
		if (nTimerId != TIMER_ID_SKIN_SEL_CHANGED)
			return false;

		m_pSkinWnd->KillTimer(TIMER_ID_SKIN_SEL_CHANGED);

		if (!m_pSkinList)
			return true;

		int nSel = m_pSkinList->GetNextSelectedItem();
		if (nSel == -1)
			return true;

		tstring		strSkinName;
		m_pSkinList->GetItemText(nSel, 0, strSkinName);

#ifndef _MPLAYER
		if (_tcsicmp(strSkinName.c_str(), m_strEmbeddedSkinName.c_str()) == 0)
		{
			// 嵌入到player 中的skin
			g_mpApp.SwitchToEmbeddedSkinMode(true);
		}
		else
#endif
		{
#ifndef _MPLAYER
			g_mpApp.EndEmbeddedSkin();
#endif
			g_mpApp.ChangeSkin(strSkinName.c_str(), true);
		}

		UpdateSkinComment(strSkinName.c_str());

		return true;
	}

	virtual void OnUIObjNotify(IUIObjNotify *pNotify)
	{
		if (pNotify->nID == CID_SKIN_LIST && pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			if (m_bIgnoreSkinListNotify)
				return;

			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED)
			{
				m_pSkinWnd->KillTimer(TIMER_ID_SKIN_SEL_CHANGED);
				// 设置延时时间为300ms，即在300ms后，用户的选择才会生效
				m_pSkinWnd->SetTimer(TIMER_ID_SKIN_SEL_CHANGED, 300);
			}
		}
	}

	void UpdateSkinList()
	{
		if (!m_pSkinList)
			return;

		m_bIgnoreSkinListNotify = true;

		m_pSkinList->DeleteAllItems(false);

		TCHAR		szSkinDef[MAX_PATH];

		// 取得上次加载的Skin
		g_Profile.GetProfileString(_T("DefaultSkin"), _T(""), szSkinDef, CountOf(szSkinDef));

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
		m_strEmbeddedSkinName = _T("");
		if (g_mpApp.IsSupportEmbedded())
		{
			TCHAR		szName[256];

			g_Player.GetEmbeddedSkinName(szName, CountOf(szName));
			m_strEmbeddedSkinName = tstring(_T("<< ")) + szName + _T(" >>");
			m_pSkinList->InsertItem(m_pSkinList->GetItemCount(), m_strEmbeddedSkinName.c_str());

			if (g_mpApp.IsEmbeddedMode())
				m_pSkinList->SetItemSelectionState(0, true);
		}
#endif

		// 查找所有的Skin，并且添加到菜单中
		vector<tstring>		vSkins;
		if (g_mpApp.GetSkinFactory()->EnumAllSkins(vSkins))
		{
			for (UINT i = 0; i < vSkins.size(); i++)
			{
				m_pSkinList->InsertItem(m_pSkinList->GetItemCount(), vSkins[i].c_str());
				if (_tcscmp(szSkinDef, vSkins[i].c_str()) == 0)
					m_pSkinList->SetItemSelectionState(m_pSkinList->GetItemCount() - 1, true);
			}
			UpdateSkinComment(szSkinDef);
		}

		m_bIgnoreSkinListNotify = false;
	}

	BOOL UpdateSkinComment(LPCTSTR szSkin)
	{
		TCHAR		szReadme[MAX_PATH] = _T("");
		tstringex	str;

		// 设置Skin的说明
		if (g_mpApp.GetSkinFactory()->GetResourceMgr()->GetResourcePathName(szReadme, CountOf(szReadme), _T("Readme.txt"))
			&& OpenFileToTString(szReadme, str))
		{
			SetUIObjectText(_T("CID_SKIN_INFO"), str.c_str(), true);
			return TRUE;
		}

		SetUIObjectText(_T("CID_SKIN_INFO"), _T(""), true);
		return FALSE;
	}

protected:
	int				CID_SKIN_LIST;
	CSkinListCtrl	*m_pSkinList;
	tstring			m_strEmbeddedSkinName;
	bool			m_bIgnoreSkinListNotify;

};

//////////////////////////////////////////////////////////////////////////

CPagePfUIRoot::CPagePfUIRoot() : CPagePfBase(GetPageID(), PAGE_UNKNOWN, _T("CMD_ROOT_UI"))
{
	AddPage(new CPagePfUI());
	AddPage(new CPagePfSkin());
}

LPCTSTR CPagePfUIRoot::GetPageID()
{
	return _T("CID_C_UIROOT");
}

void CPagePfUIRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_UI"));
}
