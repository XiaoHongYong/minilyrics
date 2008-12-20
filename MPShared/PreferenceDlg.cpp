/********************************************************************
	Created  :	2001年12月6日 21:52:18
	FileName :	PreferenceDlg.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferenceDlg.h"
#include "PreferencePages.h"
#include "PreferPageLyrDisplay.h"
#include "PreferPageDownload.h"
#include "PreferPageLyrOutput.h"
#include "PreferPageHotkey.h"
#ifdef _WIN32_DESKTOP
#include "win32/PreferPageTrayIconCtrl.h"
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

// COMMENT:
//		创建Preference对话框
// INPUT:
//		bSmall	-	是否隐藏选项树
//		dwTemplateId	-	创建后的缺省选项页
BOOL CPreferenceDlg::Create(CBaseWnd *pWndParent, PreferPageID idToactivate)
{
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
		{
			pPage->Destroy();
			delete pPage;
		}
	}
	m_vPreferencePages.clear();

	m_LangTreeCtrlHelper.Destroy();
	m_LangDlgHelper.Destroy();

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
				pPage->ShowWindow(SW_HIDE);
		}
		else if (info->nCode == WN_TV_SELECTED)
		{
			// new page
			pPage = (CPreferencePageBase*)m_treePf.GetItemData(tvInfo->iter);
			if (pPage)
			{
				if (!pPage->GetHandle())
					pPage->Create(pPage->m_nIDTemplate, this);

				// move to correct position
				ReplaceChildPos(IDC_POS_PAGE, pPage);

				// show it.
				pPage->ShowWindow(SW_SHOW);
				pPage->SetFocus();
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
	//
	// General Setup
	//
	CPreferencePageBase		*pPageParent, *pPage;

	// setup
	pPageParent = new CPreferPageSetup();
	m_vPreferencePages.push_back(pPageParent);

	// internet
	pPage = new CPreferPageInternet;
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// Options
	pPage = new CPreferPageOptions;
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// Global Hotkeys
	pPageParent = new CPreferPageHotkey();
	m_vPreferencePages.push_back(pPageParent);

	// lyrics display settings
	pPageParent = new CPreferPageLyrDispSettings();
	m_vPreferencePages.push_back(pPageParent);

	// General lyrics window
	pPageParent = new CPreferPageGeneralLyrWindow();
	m_vPreferencePages.push_back(pPageParent);

	// lyrics display
	pPage = new CPreferPageLyrDispStyle();
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// Color theme
	pPage = new CPreferPageLyrClrTheme;
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// Background picture
	pPage = new CPreferPageDispBgPic;
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// floating lyrics
	pPageParent = new CPreferPageFloatingLyr;
	m_vPreferencePages.push_back(pPageParent);

	// floating lyrics style
	pPage = new CPreferPageLyrDispStyle(true);
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// Color theme
	pPage = new CPreferPageLyrClrTheme(true);
	pPage->SetParentPage(pPageParent);
	m_vPreferencePages.push_back(pPage);

	// lyrics download
	pPageParent = new CPreferPageLyricsDownload();
	m_vPreferencePages.push_back(pPageParent);

	// lyrics search
	pPageParent = new CPreferPageSearchDir();
	m_vPreferencePages.push_back(pPageParent);

	// Lyrics Search Database
//	pPage = new CPreferPageLocalLrcDb;
//	pPage->SetParentPage(pPageParent);
//	m_vPreferencePages.push_back(pPage);

	//
	// skin
	//
	pPageParent = new CPreferPageSkin();
	m_vPreferencePages.push_back(pPageParent);

#ifdef _WIN32_DESKTOP
	//
	// TrayIcon controls
	//
	pPageParent = new CPreferPageTrayIconCtrl();
	m_vPreferencePages.push_back(pPageParent);
#endif

	//
	// Lyrics output plug-ins
	//
	pPageParent = new CPreferPageLyrOutput();
	m_vPreferencePages.push_back(pPageParent);

//	pPageParent = new CPreferPageAssociation();
//	m_vPreferencePages.push_back(pPageParent);

// 	pPageParent = new CPreferPageScreen();
// 	m_vPreferencePages.push_back(pPageParent);

}
