/********************************************************************
	Created  :	2001年12月6日 21:52:18
	FileName :	PreferenceDlg.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferenceDlg.h"
#include "PreferPageSystem.h"
#include "PreferPageUI.h"
#include "PreferPageLyrics.h"
#include "PreferPageTheme.h"
#include "PreferPageLyrOutput.h"
#include "PreferPageHotkey.h"
#ifdef _WIN32_DESKTOP
#include "win32/PreferPageTrayIconCtrl.h"
#endif

#ifdef _MPLAYER
#include "win32/PreferPageAssociation.h"
#endif

// 选项窗口
CPreferenceDlg			g_wndPreference;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPreferenceDlg::CPreferenceDlg() : CBaseDialog(IDD_PREFERENCE)
{
}

CPreferenceDlg::~CPreferenceDlg()
{
}

BOOL CPreferenceDlg::Create(CBaseWnd *pWndParent, PreferPageID idToactivate, bool bFloatingLyrMode)
{
	if (IsValid() && bFloatingLyrMode != m_bFloatingLyrMode)
	{
		Destroy();
	}

	m_bFloatingLyrMode = bFloatingLyrMode;

	if (IsValid())
	{
		// let's show old
		ActivateWindow();
    }
    else
    {
		ConstructPreferencePages();

		if (!CBaseDialog::Create(IDD_PREFERENCE, pWndParent))
			return FALSE;

		// show it
		ShowWindow(SW_SHOW);
	}

	// 设置缺省选择的页
	SetCurPage(idToactivate);

    return IsValid();
}

BOOL CPreferenceDlg::OnInitDialog()
{
	CPreferencePageBase	*pPage;

	m_treePf.Attach(this, IDC_TVPAGES);

	TreeItemIter		iter;

	for (int i = 0; i < (int)m_vPreferencePages.size(); i++)
	{
		pPage = m_vPreferencePages[i];
		iter = m_treePf.AppendItem(pPage->GetParentPage() ? pPage->GetParentPage()->GetTreeItemIter() : NULL, 
			pPage->GetCaption(), (UINT)pPage);
		pPage->SetTreeItemIter(iter);
	}

	m_treePf.ExpandAll(true);

	// 它的初始化必须在树IDC_TVPAGES创建好了之后才行。
	m_LangTreeCtrlHelper.Init(this, IDC_TVPAGES);

	CBaseDialog::OnInitDialog();

    return TRUE;
}

void CPreferenceDlg::OnDestroy()
{
    int         i = 0;

	for (i = 0; i < (int)m_vPreferencePages.size(); i++)
	{
		CPreferencePageBase		*pPage = m_vPreferencePages[i];

		if (pPage->GetHandle())
			pPage->Destroy();

		delete pPage;
	}
	m_vPreferencePages.clear();

	m_LangTreeCtrlHelper.Destroy();

	CBaseDialog::OnDestroy();
}


BOOL CPreferenceDlg::OnNotify(WIDGET_NOTIFY_INFO *info)
{
	if (info->nCtrlID == IDC_TVPAGES)
    {
		CPreferencePageBase		*pPage;
		WIDGET_TREEVIEW_NOTIFY_INFO		*tvInfo = (WIDGET_TREEVIEW_NOTIFY_INFO*)info;
		if (info->nCode == WN_TV_UNSELECTED)
		{
			// hide old page
			pPage = (CPreferencePageBase*)m_treePf.GetItemData(tvInfo->iter);
			if (pPage && pPage->IsValid())
			{
				pPage->OnHidePage();
				pPage->ShowWindow(SW_HIDE);
			}
		}
		else if (info->nCode == WN_TV_SELECTED)
		{
			// new page
			pPage = (CPreferencePageBase*)m_treePf.GetItemData(tvInfo->iter);
			if (pPage)
			{
				if (pPage->m_nIDTemplate == 0)
				{
					// Show first child item
					CPreferencePageBase *pChild = GetChild(pPage);
					pPage = pChild;
					if (pChild)
						m_treePf.SetSelected(*(pChild->GetTreeItemIter()));
				}
				else
				{
					if (!pPage->GetHandle())
						pPage->Create(pPage->m_nIDTemplate, this);
					else
						pPage->OnShowPage();

					// move to correct position
					ReplaceChildPos(IDC_POS_PAGE, pPage);

					// show it.
					pPage->ShowWindow(SW_SHOW);
					pPage->SetFocus();
				}
			}
		}
    }

    return TRUE;
}

// COMMENT:
//		设置对话框模板为dwTemplateId的页面为当前的页面
//		同时将左边的树展开
void CPreferenceDlg::SetCurPage(PreferPageID id)
{
	CPreferencePageBase	*pPage;

	for (int i = 0; i < (int)m_vPreferencePages.size(); i++)
	{
		pPage = m_vPreferencePages[i];

		if (pPage->GetPageID() == id)
		{
			m_treePf.SetSelected(*pPage->GetTreeItemIter());
			return;
		}
	}

	m_treePf.ExpandAll(true);
}

void CPreferenceDlg::OnLanguageChanged()
{
	// 向子窗口发送消息, 选择语言

	for (int i = 0; i < (int)m_vPreferencePages.size(); i++)
	{
#ifdef _WIN32
		CPreferencePageBase	*pPage = m_vPreferencePages[i];
		if (pPage->GetHandle())
			PostMessage(pPage->GetHandle(), ML_WM_LANGUAGE_CHANGED, ML_LANGUAGE_CHANGED_PARAM, ML_LANGUAGE_CHANGED_PARAM);
#endif
	}

	m_LangTreeCtrlHelper.ToLocal();

	CBaseDialog::OnLanguageChanged();
}

void CPreferenceDlg::ConstructPreferencePages()
{
	m_vPreferencePages.clear();

	// Construct preference pages

	CPreferencePageBase		*pPageParent, *pPage;

	{
		//
		// User interface pages
		//

		// User interface root
		pPageParent = new CPPageUI;
		m_vPreferencePages.push_back(pPageParent);

		// Localization
		pPage = new CPPageLocalization;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

		// Skins
		pPage = new CPreferPageSkin;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

		// Advanced Options
		pPage = new CPPageUIAdvanced;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
	}

	{
		//
		// Theme pages
		//
		pPageParent = new CPPageThemes;
		m_vPreferencePages.push_back(pPageParent);

		// Color theme
		pPage = new CPPageLyrDisplay(m_bFloatingLyrMode);
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

		// Background picture
		pPage = new CPreferPageDispBgPic;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
	}

	{
		//
		// System pages
		//
		pPageParent = new CPreferencePageBase(0, _T("System"));
		m_vPreferencePages.push_back(pPageParent);

#ifdef _MPLAYER
		// File association
		pPage = new CPreferPageAssociation;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
#else
#endif // _MPLAYER

		// HotKey
		pPage = new CPreferPageHotkey;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

#ifdef _WIN32_DESKTOP
		//
		// TrayIcon controls
		//
		pPage = new CPreferPageTrayIconCtrl();
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
#endif

		// internet
		pPage = new CPreferPageInternet;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

		// Startup
		pPage = new CPPageStartup;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
	}

	{
		//
		// Lyrics
		//

		// lyrics
		pPageParent = new CPreferPageLyrics();
		m_vPreferencePages.push_back(pPageParent);

		// lyrics download
		pPage = new CPreferPageLyricsDownload;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);

		// lyrics download
		pPage = new CPreferPageLyrOutput;
		pPage->SetParentPage(pPageParent);
		m_vPreferencePages.push_back(pPage);
	}
}

CPreferencePageBase *CPreferenceDlg::GetChild(CPreferencePageBase *pPageParent)
{
	for (int i = 0; i + 1 < (int)m_vPreferencePages.size(); i++)
	{
		if (m_vPreferencePages[i] == pPageParent)
		{
			return m_vPreferencePages[i + 1];
		}
	}

	return NULL;
}
