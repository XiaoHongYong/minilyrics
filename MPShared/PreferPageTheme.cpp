#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageTheme.h"
#include "LyricShowAgentObj.h"
#include "DlgLyrBlendingOpt.h"
#include "ThemesFile.h"


class CPagePfTheme : public CPagePfBase
{
public:
	CPagePfTheme() : CPagePfBase(GetPageID())
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();
	}

	void OnDestroy()
	{
	}

	bool OnCustomCommand(int nId)
	{
		return CPagePfBase::OnCustomCommand(nId);
	}

protected:

};

//////////////////////////////////////////////////////////////////////////

CPagePfThemeRoot::CPagePfThemeRoot() : CPagePfBase(GetPageID())
{
	AddPage(new CPagePfTheme());
}

LPCTSTR CPagePfThemeRoot::GetPageID()
{
	return _T("CID_C_THEME_ROOT");
}

void CPagePfThemeRoot::OnCreate()
{
	CPagePfBase::OnCreate();

	CheckToolbarButton(_T("CID_TOOLBAR_THEME"), _T("CMD_THEME_THEME"), true);
}

bool CPagePfThemeRoot::OnCustomCommand(int nId)
{
	if (nId == GetIDByName(_T("CMD_THEME_THEME")))
		SwitchToPage(CPagePfTheme::GetPageID());
// 	else if (nId == GetIDByName(_T("CMD_THEME_LYR_DISPLAY")))
// 		SwitchToPage(CPagePfSkin::GetPageID());
// 	else if (nId == GetIDByName(_T("CMD_THEME_LYR_BG_PIC")))
// 		SwitchToPage(CPagePfSkin::GetPageID());
	else
		return CPagePfBase::OnCustomCommand(nId);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
class CDlgThemeItems : public CBaseDialog  
{
public:
	CDlgThemeItems() : CBaseDialog(IDD_THEME_ITEMS)
	{
	}
	virtual BOOL OnInitDialog()
	{
		CBaseDialog::OnInitDialog();

		if (!m_strTitle.empty())
			SetDlgItemText(IDC_INFO, m_strTitle.c_str());

		for (int i = 0; i < g_themeItemsCount; i++)
		{
			g_themeItems[i].bValid = g_Profile.GetProfileBool(_T("ThemeItems"), g_themeItems[i].szName, g_themeItems[i].bValid);
			CheckButton(g_themeItems[i].nIDCtrl, g_themeItems[i].bValid);
		}

		return TRUE;
	}

	virtual void OnOK()
	{
		for (int i = 0; i < g_themeItemsCount; i++)
		{
			g_themeItems[i].bValid = IsButtonChecked(g_themeItems[i].nIDCtrl);
			g_Profile.WriteProfileInt(_T("ThemeItems"), g_themeItems[i].szName, g_themeItems[i].bValid);
		}

		CBaseDialog::OnOK();
	}

	tstring				m_strTitle;

};

//////////////////////////////////////////////////////////////////////////

CPPageThemes::CPPageThemes() : CPreferencePageBase(IDD_PAGE_THEMES, _T("Themes"))
{
	m_PageID = PAGE_THEMES;
}

CPPageThemes::~CPPageThemes()
{
}

BOOL CPPageThemes::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_listThemes.Attach(this, IDC_LIST_THEMES);
	m_listThemes.InsertColumn(0, TOLOCAL(_T("Themes")), 0, 350);
	m_listThemes.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	UpdateThemesList();

	InitCheckButtons();

	return TRUE;
}

void CPPageThemes::OnCommand(UINT uID, UINT nNotifyCode)
{
	CPreferencePageBase::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_LOAD:
	case IDC_DELETE:
		{
			int				nSel;

			nSel = m_listThemes.GetNextSelectedItem(-1);
			if (nSel == -1)
			{
				MessageOut(TOLOCAL(_T("Please select the theme from list.")));
				break;
			}

			CThemesFile		file;
			tstring			strSelTheme;

			m_listThemes.GetItemText(nSel, 0, strSelTheme);
			file.Open();

			if (uID == IDC_DELETE)
			{
				file.Delete(strSelTheme.c_str());
				file.Save();
				SetDlgItemText(IDC_E_THEME, _T(""));
				UpdateThemesList();
			}
			else
			{
				CDlgThemeItems	dlg;
				dlg.m_strTitle = TOLOCAL(_T("Please choose the theme items to load:"));
				if (dlg.DoModal(this) != IDOK)
					return;

				g_Profile.WriteProfileString(_T("CurTheme"), strSelTheme.c_str());
				file.SetCurTheme(strSelTheme.c_str());
			}
		}
		break;
	case IDC_SAVE:
		{
			tstring		strThemeName;
			CThemesFile		file;

			GetDlgItemText(IDC_E_THEME, strThemeName);
			if (strThemeName.empty())
			{
				MessageOut(TOLOCAL(_T("Please enter the theme name.")));
				break;
			}

			CDlgThemeItems	dlg;
			dlg.m_strTitle = TOLOCAL(_T("Please choose the theme items to save:"));
			if (dlg.DoModal(this) != IDOK)
				return;

			file.Open();

			file.UseCurSettingAsTheme(strThemeName.c_str());

			file.Save();

			UpdateThemesList();
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}

BOOL CPPageThemes::OnNotify(WIDGET_NOTIFY_INFO *info)
{
	if (info->widgetType == WT_LIST_CTRL && info->nCtrlID == IDC_LIST_THEMES)
	{
		if (info->nCode == WN_GEN_DBL_CLICK)
		{
			OnCommand(IDC_LOAD, 0);
		}
		else if (info->nCode == WN_LV_SELECTED)
		{
			int				nSel;

			nSel = m_listThemes.GetNextSelectedItem(-1);
			if (nSel != -1)
			{
				tstring			strSelTheme;

				m_listThemes.GetItemText(nSel, 0, strSelTheme);
				SetDlgItemText(IDC_E_THEME, strSelTheme.c_str());
			}
		}
	}

	return TRUE;
}

void CPPageThemes::UpdateThemesList()
{
	CThemesFile		file;
	V_Str			vThemes;
	tstring			strCurTheme;
	int				nSel =  -1;

	file.Open();
	file.EnumAllThemes(vThemes);

	m_listThemes.DeleteAllItems();

	strCurTheme = g_Profile.GetProfileString(_T("CurTheme"), _T(""));

	for (int i = 0; i < (int)vThemes.size(); i++)
	{
		m_listThemes.InsertItem(i, vThemes[i].c_str());
		if (nSel == -1 && _tcscmp(strCurTheme.c_str(), vThemes[i].c_str()))
			nSel = i;
	}

	if (nSel != -1)
		m_listThemes.SetItemSelectionState(nSel);
}

//////////////////////////////////////////////////////////////////////////
static COLORREF 	_clrCustom[16];

CPPageLyrDisplay::CPPageLyrDisplay(bool bFloatingLyrics) 
: CPreferencePageBase(IDD_PAGE_LYR_DISPLAY, _T("Lyrics Display"))
{
	if (bFloatingLyrics)
		m_eventType = ET_LYRICS_FLOATING_SETTINGS;
	else
		m_eventType = ET_LYRICS_DISPLAY_SETTINGS;

	m_PageID = PAGE_LYR_DISPLAY;

	m_nColorSetCount = CS_SIZE;
#ifdef _WIN32
	m_SelColorType = CS_NONE;
#endif
}

CPPageLyrDisplay::~CPPageLyrDisplay()
{
}

BOOL CPPageLyrDisplay::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_cbSettingType.Attach(this, IDC_CB_SETTING_TYPE);
	m_cbSettingType.InsertString(ST_NORMAL_LYR, TOLOCAL(_T("Normal lyrics mode settings")));
	m_cbSettingType.InsertString(ST_FLOATING_LYR, TOLOCAL(_T("Floating lyrics mode settings")));

	if (m_eventType == ET_LYRICS_FLOATING_SETTINGS)
		m_cbSettingType.SetCurSel(1);
	else
		m_cbSettingType.SetCurSel(0);

	m_cbLyrStyle.Attach(this, IDC_CB_LYR_STYLE);
	m_cbFadeMode.Attach(this, IDC_CB_FADE_MODE);
	m_cbAlign.Attach(this, IDC_CB_ALIGN);

	// Lyrics colors
	m_ColorSet[CS_OUTLINED_HIGH_BORDER].strProfileKey = _T("HilightBorderColor");
	m_ColorSet[CS_OUTLINED_LOW_BORDER].strProfileKey = _T("LowlightBorderColor");
	m_ColorSet[CS_BG].strProfileKey = _T("BgColor");

	m_ColorSet[CS_OUTLINED_HIGH_BORDER].clrDefault = RGB(255, 0, 0);
	m_ColorSet[CS_OUTLINED_LOW_BORDER].clrDefault = RGB(0, 0, 255);
	m_ColorSet[CS_BG].clrDefault = RGB(79, 88, 110);

	if (m_eventType == ET_LYRICS_FLOATING_SETTINGS)
	{
		m_ColorSet[CS_OUTLINED_HIGH_BORDER].clrDefault = RGB(0, 0, 255);
		m_ColorSet[CS_OUTLINED_LOW_BORDER].clrDefault = RGB(128, 128, 255);
	}

	m_ColorSet[CS_OUTLINED_HIGH_BORDER].dwButtonID = IDC_OBM_HIGHLIGHT_BORDER;
	m_ColorSet[CS_OUTLINED_LOW_BORDER].dwButtonID = IDC_OBM_LOWLIGHT_BORDER;
	m_ColorSet[CS_BG].dwButtonID = IDC_BG_COLOR;

	// Set drag rect of every color
	for (int i = 0; i < m_nColorSetCount; i++)
	{
		m_ColorSet[i].button.Attach(this, m_ColorSet[i].dwButtonID);
		GetDlgItemRect(m_ColorSet[i].dwButtonID, &(m_ColorSet[i].rc));
		ScreenToClient(m_ColorSet[i].rc);
		m_ColorSet[i].rc.right = m_ColorSet[i].rc.left + m_ColorSet[i].rc.Height();
		m_ColorSet[i].rc.OffsetRect(-m_ColorSet[i].rc.Width() - 3, 0);
	}

	m_btLOBHilight.Attach(this, IDC_OBM_HIGHLIGHT);
	m_btLOBLowlight.Attach(this, IDC_OBM_LOWGHLIGHT);

	SetSettingType(m_eventType == ET_LYRICS_FLOATING_SETTINGS ? ST_FLOATING_LYR : ST_NORMAL_LYR);

#ifdef _WIN32
	m_SelColorType = CS_NONE;
	m_hcursorSelecting = LoadCursor(GetAppInstance(), MAKEINTRESOURCE(IDC_SELCOLOR));
#endif

	return TRUE;
}

void CPPageLyrDisplay::OnShowPage()
{
	SetSettingType(m_eventType == ET_LYRICS_FLOATING_SETTINGS ? ST_FLOATING_LYR : ST_NORMAL_LYR);
}

void CPPageLyrDisplay::SetSettingType(SettingType settingType)
{
	EventType	m_eventTypeOld;
	m_eventTypeOld = m_eventType;
	m_nColorSetCount = CS_SIZE;
	if (settingType == ST_FLOATING_LYR)
	{
		m_eventType = ET_LYRICS_FLOATING_SETTINGS;
		m_strSectName = SZ_SECT_FLOATING_LYR;
	}
	else
	{
		m_eventType = ET_LYRICS_DISPLAY_SETTINGS;
		m_strSectName = SZ_SECT_LYR_DISPLAY;
	}

	int		nItemsToHide[] = { IDC_S_FOCUSLINE_COLOR, IDC_S_TAG_COLOR, IDC_EDIT_ROW_BK_COLOR, IDC_TAG_CLR };
	for (int i = 0; i < CountOf(nItemsToHide); i++)
		ShowDlgItem(nItemsToHide[i], settingType == ST_FLOATING_LYR ? SW_HIDE : SW_SHOW);

	ClearOptions();

	{
		//
		// Add lyrics display style options
		//
		LPCTSTR		vszItemName[] = { _T("Vertical scrolling"), _T("Double lines"), _T("Single line"),
			_T("Horizontal scrolling"), _T("Static text"), _T("Movie subtitle") };

		LPCTSTR		vszItemValues[] = { CLyricShowMultiRowObj::ClassName(), CLyricShowTwoRowObj::ClassName(), 
			CLyricShowSingleRowObj::ClassName(), CLyricShowHorzScrollObj::ClassName(), 
			CLyricShowTxtObj::ClassName(), CLyricShowVobSub::ClassName() };

		OptComboStr		opt;
		opt.nIDWidgetCombo = IDC_CB_LYR_STYLE;
		opt.Set(ET_UI_SETTINGS_CHANGED,
			settingType == ST_FLOATING_LYR ? SZ_SECT_FLOATING_LYR : SZ_SECT_LYR_DISPLAY, 
			_T("LyrDisplayStyle"),
			CLyricShowMultiRowObj::ClassName());

		m_cbLyrStyle.ResetContent();
		ASSERT(CountOf(vszItemName) == CountOf(vszItemValues));
		for (int i = 0; i < CountOf(vszItemValues); i++)
		{
			m_cbLyrStyle.InsertString(i, TOLOCAL(vszItemName[i]));
			opt.AddItemValue(vszItemValues[i]);
		}
		AddOptComboStr(opt);
	}

	{
		//
		// lyrics align
		//
		LPCTSTR		vszItemName[] = { _T("Left"), _T("Center"), _T("Right") };
		LPCTSTR		vszItemValues[] = { _T("left"), _T("center"), _T("right") };

		OptComboStr		opt;
		opt.nIDWidgetCombo = IDC_CB_ALIGN;
		opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrAlign"), _T("center"));

		m_cbAlign.ResetContent();
		ASSERT(CountOf(vszItemName) == CountOf(vszItemValues));
		for (int i = 0; i < CountOf(vszItemValues); i++)
		{
			m_cbAlign.InsertString(i, TOLOCAL(vszItemName[i]));
			opt.AddItemValue(vszItemValues[i]);
		}
		AddOptComboStr(opt);
	}

	{
		// lyrics Fade out options
		LPCTSTR		vszItemName[] = { _T("Normal"), _T("Fade in lyrics"), _T("Fade out background") };

		OptComboStr		opt;
		opt.nIDWidgetCombo = IDC_CB_FADE_MODE;
		opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrDrawOpt"), _T("fadein"));

		m_cbFadeMode.ResetContent();
		ASSERT(CountOf(vszItemName) == DO_MAX);
		for (int i = 0; i < CountOf(vszItemName); i++)
		{
			m_cbFadeMode.InsertString(i, TOLOCAL(vszItemName[i]));
			opt.AddItemValue(g_idsLyrDisplayOpt[i].szId);
		}
		AddOptComboStr(opt);
	}

	// karaoke option
	AddOptBool(m_eventType, m_strSectName.c_str(), _T("Karaoke"), false, IDC_C_KARAOKE);

	AddOptBool(m_eventType, m_strSectName.c_str(), _T("OutlineLyrText"), false, IDC_C_OUTLINE_LYR);

	RecreateBrush();

	InitCheckButtons();

	CreateLOBXlightBmp(true);
	CreateLOBXlightBmp(false);

	InvalidateRect(NULL, true);
}

void LoadLyrOverlayBlendingSettings(LPCTSTR szSectName, CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight, bool bHilight);

void CPPageLyrDisplay::CreateLOBXlightBmp(bool bHilight)
{
	CRect					rc;
	CLyricShowObj::TextOverlayBlending		tob;

	GetDlgItemRect(IDC_OBM_HIGHLIGHT, &rc);

	LoadLyrOverlayBlendingSettings(m_strSectName.c_str(), tob, rc.Height(), bHilight);

	if (tob.obm == OBM_COLOR)
	{
		PRAW_IMAGE_DATA		pRawImage;

		pRawImage = new RAW_IMAGE_DATA;
		pRawImage->Create(rc.Width(), rc.Height(), 24);
		RawImageSet(pRawImage, tob.clr[CLyricShowObj::TCI_FILL].r(), 
			tob.clr[CLyricShowObj::TCI_FILL].g(), 
			tob.clr[CLyricShowObj::TCI_FILL].b(),
			tob.clr[CLyricShowObj::TCI_FILL].getAlpha());
		tob.imgPattern.Attach(pRawImage);
	}

	if (tob.imgPattern.IsValid())
	{
		CMLImage	*pImage = RawImageToMLImage(tob.imgPattern);
		if (bHilight)
			m_btLOBHilight.SetImage(pImage);
		else
			m_btLOBLowlight.SetImage(pImage);
	}
}

void CPPageLyrDisplay::RecreateBrush()
{
	for (int i = 0; i < m_nColorSetCount; i++)
	{
		m_ColorSet[i].clr.set(m_ColorSet[i].clrDefault);

		ProfileGetColorValue(m_ColorSet[i].clr, 
			m_strSectName.c_str(), m_ColorSet[i].strProfileKey.c_str());

		m_ColorSet[i].button.SetColor(m_ColorSet[i].clr);
		m_ColorSet[i].button.InvalidateRect();
	}
}


void CPPageLyrDisplay::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_CB_SETTING_TYPE:
		{
			if (nNotifyCode == CBN_SELCHANGE)
				SetSettingType((SettingType)m_cbSettingType.GetCurSel());
		}
		break;
	case IDC_SEL_FONT_ENGLISH:
	case IDC_SEL_FONT_OTHERS:
		{
			int		nHeight, nWeight;
			BYTE	byItalic;
			tstring	strFaceNameLatin9, strFaceNameOthers;
			int		nRet;
			bool	bLatin9 = uID == IDC_SEL_FONT_ENGLISH;

			// get default settings in profile
			ProfileGetLyricsFont(m_strSectName.c_str(), nHeight, nWeight, 
				byItalic, strFaceNameLatin9, strFaceNameOthers);

			CDlgChooseFont		chooser;

			nRet = chooser.DoModal(this, bLatin9 ? strFaceNameLatin9.c_str() : strFaceNameOthers.c_str(), 
				nHeight, nWeight, byItalic != 0);
			if (nRet == IDOK)
			{
				nHeight = chooser.GetHeight();
				if (nHeight < 0)
					nHeight = -nHeight;

				// now save to profile
				ProfileWriteLyricsFont(m_eventType, m_strSectName.c_str(), nHeight, chooser.GetWeight(), 
					chooser.GetItalic(), bLatin9 ? chooser.GetFaceName() : NULL, bLatin9 ? NULL : chooser.GetFaceName());
			}
		}
		break;
	case IDC_BG_COLOR:
	case IDC_EDIT_ROW_BK_COLOR:
	case IDC_TAG_CLR:
	case IDC_OBM_HIGHLIGHT_BORDER:
	case IDC_OBM_LOWLIGHT_BORDER:
		{
			//
			// lyrics display Color settings
			//
			for (int i = 0; i < m_nColorSetCount; i++)
			{
				if (uID == m_ColorSet[i].dwButtonID)
				{
					tstring		strColor;

					m_ColorSet[i].button.GetColor(m_ColorSet[i].clr);

					ColorToStr(m_ColorSet[i].clr, strColor);
					CMPlayerSettings::SetSettings(m_eventType, m_strSectName.c_str(), m_ColorSet[i].strProfileKey.c_str(), strColor.c_str());

					break;
				}
			}
		}
		break;
	case IDC_OBM_HIGHLIGHT:
		{
			CDlgLyrBlendingOpt		dlg;

			dlg.DoModal(this, m_eventType, m_strSectName.c_str(), true);

			CreateLOBXlightBmp(true);
			m_btLOBHilight.InvalidateRect();
		}
		break;
	case IDC_OBM_LOWGHLIGHT:
		{
			CDlgLyrBlendingOpt		dlg;

			dlg.DoModal(this, m_eventType, m_strSectName.c_str(), false);

			CreateLOBXlightBmp(false);
			m_btLOBLowlight.InvalidateRect();
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}

#ifdef _WIN32
void CPPageLyrDisplay::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_SelColorType != CS_NONE)
	{
		ReleaseCapture();

		InvalidateRect(NULL, TRUE);

		// notify to update
		tstring		strColor;

		ColorToStr(m_ColorSet[m_SelColorType].clr, strColor);
		CMPlayerSettings::SetSettings(m_eventType, m_strSectName.c_str(), m_ColorSet[m_SelColorType].strProfileKey.c_str(), strColor.c_str());

		m_SelColorType = CS_NONE;
	}
}

void CPPageLyrDisplay::OnLButtonDown(UINT nFlags, CPoint point)
{
	for (int i = 0; i < m_nColorSetCount; i++)
	{
		if (m_ColorSet[i].rc.PtInRect(point))
		{
			SetCapture();
			SetCursor(m_hcursorSelecting);
			m_SelColorType = (COLOR_SET_TYPE)i;

			InvalidateRect(NULL, TRUE);
		}
	}
}

void CPPageLyrDisplay::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_SelColorType != CS_NONE)
	{
		CPoint	pt;
		CRect	rcw;

		pt = point;
		ClientToScreen(pt);

		HDC	hdc;

		hdc = ::GetDC(NULL);
		m_ColorSet[m_SelColorType].clr.set(GetPixel(hdc, pt.x, pt.y));

		m_ColorSet[m_SelColorType].button.SetColor(m_ColorSet[m_SelColorType].clr);
		m_ColorSet[m_SelColorType].button.InvalidateRect();

		::ReleaseDC(NULL, hdc);
	}
}

void CPPageLyrDisplay::OnPaint(IMLGraphicsBase *pGraph)
{
	HICON	hIconDragBefore, hIconAfter;
	HDC		hdc;

	hdc = pGraph->GetHandle();

	hIconDragBefore = (HICON)::LoadImage(GetAppInstance(), MAKEINTRESOURCE(IDI_DRAGBEFORE), IMAGE_ICON, 16, 16, LR_VGACOLOR);
	hIconAfter = (HICON)::LoadImage(GetAppInstance(), MAKEINTRESOURCE(IDI_DRAGAFTER), IMAGE_ICON, 16, 16, LR_VGACOLOR);

	// hIconDragBefore = LoadIcon(GetAppInstance(), MAKEINTRESOURCE(IDI_DRAGAFTER));
	// hIconAfter = LoadIcon(GetAppInstance(), MAKEINTRESOURCE(IDI_DRAGBEFORE));

	for (int i = 0; i < m_nColorSetCount; i++)
	{
		if (m_SelColorType == i)
			DrawIconEx(hdc, m_ColorSet[i].rc.left, m_ColorSet[i].rc.top, hIconAfter, 16, 16, 0, NULL, DI_NORMAL);
		else
			DrawIconEx(hdc, m_ColorSet[i].rc.left, m_ColorSet[i].rc.top, hIconDragBefore, 16, 16, 0, NULL, DI_NORMAL);
	}
}

#endif

//////////////////////////////////////////////////////////////////////////
//
CPreferPageDispBgPic::CPreferPageDispBgPic() : 
CPreferencePageBase(IDD_PAGE_LYR_BG_PIC, _T("Lyrics Background Picture"))
{
	m_PageID = PAGE_LYR_BG;
}

BOOL CPreferPageDispBgPic::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseAlbumArtAsBg"), false, IDC_C_ALBUMART);

	AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), false, IDC_C_BKIMAGE);

	AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("DarkenLyrBg"), true, IDC_C_DARKEN_LYR_BG);

	SetDlgItemText(IDC_E_BG_PIC_FOLDER,
		CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());

	SetDlgItemInt(IDC_E_DELAYTIME, 
		g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), 10), false);

	InitCheckButtons();

	return TRUE;
}

void CPreferPageDispBgPic::OnDestroy()
{
	BOOL	bTranslated;
	int		nDelayTime;

	nDelayTime = GetDlgItemInt(IDC_E_DELAYTIME, &bTranslated, FALSE);
	if (bTranslated)
	{
		if (nDelayTime < 1)
			nDelayTime = 1;
		if (nDelayTime > 10000)
			nDelayTime = 10000;

		if (nDelayTime != g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), 10))
		{
			CMPlayerSettings::SetLyricsDisplaySettings(_T("SlideDelayTime"), nDelayTime);
		}
	}

	CPreferencePageBase::OnDestroy();
}

void CPreferPageDispBgPic::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
		/*	case IDC_CB_POSITION:
		{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
		int		nItem;
		nItem = m_cbPicPos.GetCurSel();
		if (nItem == -1)
		break;
		CMPlayerSettings::SetLyricsDisplaySettings(_T("BgPicPos"), nItem);
		}
		break;
		}*/
	case IDC_BR_BG_PIC_FOLDER:
		{
			if (g_mpApp.GetMainWnd())
				g_mpApp.GetMainWnd()->OnCommand(IDC_SEL_BKIMAGE, 0);

			SetDlgItemText(IDC_E_BG_PIC_FOLDER,
				CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}
