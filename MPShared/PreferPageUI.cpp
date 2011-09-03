#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageUI.h"
#include "PreferenceDlg.h"


//////////////////////////////////////////////////////////////////////////
const int	_CMD_ID_LANGUANG_BEGIN	= 3000;

class CPagePfUI : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfUI() : CPagePfBase(PAGE_UI, _T("CMD_UI"))
	{
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		AddOptBool(ET_NULL, SZ_SECT_UI, _T("topmost"), true, _T("CID_C_ON_TOP"));

#ifdef _WIN32_DESKTOP
		// Click though...
		CStrPrintf str(_T("%d"), CMPlayerAppBase::GetMPSkinFactory()->GetClickThrough());
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
			CMPlayerAppBase::GetMPSkinFactory()->TopmostAll(bValue);
		}
#ifdef _WIN32_DESKTOP
		else if (nId == GetIDByName(_T("CID_C_CLICK_THROUGH")))
		{
			CMPlayerAppBase::GetMPSkinFactory()->SetClickThrough(IsButtonChecked(nId));
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
				m_pSkin->ClientToScreen(rc);

			m_menuLang.TrackPopupMenu(rc.left, rc.top, m_pSkin);
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
			nId < _CMD_ID_LANGUANG_BEGIN + (int)m_vTransFiles.size())
		{
			TCHAR	szLanFileName[MAX_PATH] = _T("");

			if (g_Profile.GetProfileString(_T("Language"), _T(""), szLanFileName, MAX_PATH))
			{
				if (m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName == szLanFileName)
					return true;
			}

			g_Profile.WriteProfileString(_T("Language"), m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName.c_str());

			CMPlayerAppBase::GetInstance()->OnLanguageChanged();

			UpdateLanguageInfo();

			Invalidate();

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

UIOBJECT_CLASS_NAME_IMP(CPagePfUI, _T("PreferPage.UI"))

//////////////////////////////////////////////////////////////////////////

class CPagePfSkin : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfSkin() : CPagePfBase(PAGE_SKINS, _T("CMD_SKINS"))
	{
		m_pSkinList = NULL;
		m_bIgnoreSkinListNotify = false;
		CID_SKIN_LIST = 0;
		TIMER_ID_SKIN_SEL_CHANGED = 0;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		m_bIgnoreSkinListNotify = true;

		CID_SKIN_LIST = GetIDByName(_T("CID_SKIN_LIST"));
		m_pSkin->RegisterUIObjNotifyHandler(CID_SKIN_LIST, this);
		m_pSkinList = (CSkinListCtrl *)GetUIObjectById(CID_SKIN_LIST, CSkinListCtrl::ClassName());
		if (m_pSkinList)
			m_pSkinList->AddColumn(_T("Skins"), 200);

		m_bIgnoreSkinListNotify = false;

		UpdateSkinList();
	}

	void OnDestroy()
	{
		m_pSkin->UnregisterUIObjNotifyHandler(this);

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		return CPagePfBase::OnCustomCommand(nId);
	}

	void OnTimer(int nId)
	{
		if (nId != TIMER_ID_SKIN_SEL_CHANGED)
			return;

		m_pSkin->UnregisterTimerObject(this, TIMER_ID_SKIN_SEL_CHANGED);

		if (!m_pSkinList)
			return;

		int nSel = m_pSkinList->GetNextSelectedItem();
		if (nSel == -1)
			return;

		tstring		strSkinName;
		m_pSkinList->GetItemText(nSel, 0, strSkinName);

		// Set default page, when skin changes.
#ifndef _MPLAYER
		if (_tcsicmp(strSkinName.c_str(), m_strEmbeddedSkinName.c_str()) == 0)
		{
			g_Profile.WriteProfileInt(_T("MPSkinMode"), false);
			// 嵌入到player 中的skin
			CMPlayerAppBase::GetInstance()->OnSwitchToEmbeddedSkinMode(true);
		}
		else
#endif
		{
			CMPlayerAppBase::GetInstance()->ChangeSkinByUserCmd(strSkinName.c_str());
		}
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
				m_pSkin->UnregisterTimerObject(this, TIMER_ID_SKIN_SEL_CHANGED);
				// 设置延时时间为300ms，即在300ms后，用户的选择才会生效
				TIMER_ID_SKIN_SEL_CHANGED = m_pSkin->RegisterTimerObject(this, 300);
			}
		}
	}

	void UpdateSkinList()
	{
		if (!m_pSkinList)
			return;

		m_bIgnoreSkinListNotify = true;

		m_pSkinList->DeleteAllItems(false);

		tstring strDefaultSkin = CSkinApp::GetInstance()->GetDefaultSkin();

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
		m_strEmbeddedSkinName = _T("");
		if (CMPlayerAppBase::GetInstance()->IsSupportEmbedded())
		{
			m_strEmbeddedSkinName = tstring(_T("<< ")) + g_Player.GetEmbeddedSkinName() + _T(" >>");
			m_pSkinList->InsertItem(m_pSkinList->GetItemCount(), m_strEmbeddedSkinName.c_str());

			if (CMPlayerAppBase::GetInstance()->IsEmbeddedMode())
				m_pSkinList->SetItemSelectionState(0, true);
		}
#endif

		// 查找所有的Skin，并且添加到菜单中
		vector<tstring>		vSkins;
		if (CMPlayerAppBase::GetMPSkinFactory()->EnumAllSkins(vSkins))
		{
			for (UINT i = 0; i < vSkins.size(); i++)
			{
				m_pSkinList->InsertItem(m_pSkinList->GetItemCount(), vSkins[i].c_str());
				if (_tcscmp(strDefaultSkin.c_str(), vSkins[i].c_str()) == 0)
					m_pSkinList->SetItemSelectionState(m_pSkinList->GetItemCount() - 1, true);
			}
			UpdateSkinComment(strDefaultSkin.c_str());
		}

		m_bIgnoreSkinListNotify = false;
	}

	BOOL UpdateSkinComment(LPCTSTR szSkin)
	{
		TCHAR		szReadme[MAX_PATH] = _T("");
		tstringex	str;

		// 设置Skin的说明
		if (CMPlayerAppBase::GetMPSkinFactory()->GetResourceMgr()->GetResourcePathName(szReadme, CountOf(szReadme), _T("Readme.txt"))
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
	int				TIMER_ID_SKIN_SEL_CHANGED;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfSkin, _T("PreferPage.Skin"))

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfUIRoot, _T("PreferPage.UIRoot"))

CPagePfUIRoot::CPagePfUIRoot() : CPagePfBase(PAGE_UNKNOWN, _T("CMD_ROOT_UI"))
{
}

void CPagePfUIRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_UI"));
}

void RegisterPfUIPages(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPagePfUIRoot);
	AddUIObjNewer2(pSkinFactory, CPagePfUI);
	AddUIObjNewer2(pSkinFactory, CPagePfSkin);
}

