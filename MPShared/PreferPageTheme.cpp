#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageTheme.h"
#include "LyricShowAgentObj.h"
#include "ThemesFile.h"
#include "LyrDisplayClrListWnd.h"

//////////////////////////////////////////////////////////////////////////
/*
class CPagePfThemeItems : public CPagePfBase
{
public:
	CPagePfThemeItems() : CPagePfBase(GetPageID(), PAGE_UNKNOWN, _T(""))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_ITEMS"); }

	void OnOK()
	{
		m_pParent->SetProperty(_T("ThemeItemsActionResult"), IDOK);
		m_pParent->SwitchToLastPage();
	}

	void OnCancel()
	{
		m_pParent->SetProperty(_T("ThemeItemsActionResult"), IDCANCEL);
		m_pParent->SwitchToLastPage();
	}

	void OnShow()
	{
		CPagePfBase::OnShow();

		if (m_pParent->GetPropertyInt(_T("ThemeItemsLoadAction")))
		{
			// Load theme
			SetUIObjectText(_T("CID_THEME_ITEMS_INFO"), 
				_T("Please choose the theme items to load:"), false);
		}
		else
		{
			// Save theme
			SetUIObjectText(_T("CID_THEME_ITEMS_INFO"), 
				_T("Please choose the theme items to save:"), false);
		}
	}

	bool OnCustomCommand(int nId)
	{
		return CPagePfBase::OnCustomCommand(nId);
	}

protected:

};*/

//////////////////////////////////////////////////////////////////////////

class CPagePfLyricsBgPic : public CPagePfBase
{
public:
	CPagePfLyricsBgPic() : CPagePfBase(GetPageID(), PAGE_LYR_BG, _T("CMD_THEME_LYR_BG_PIC"))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_LYR_BG"); }

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseAlbumArtAsBg"), false, _T("CID_C_USE_ALBUM_ART_AS_BG"));

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), false, _T("CID_C_ENABLE_BG_IMAGE"));

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("DarkenLyrBg"), true, _T("CID_C_DARKEN_LYR_BG"));

		SetUIObjectText(_T("CID_E_DELAY_TIME"), 
			g_Profile.GetProfileString(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), _T("10")));

		InitCheckButtons();

		SetUIObjectText(_T("CID_E_PIC_FOLDER"), 
			CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());

	}

	void OnDestroy()
	{
		int		nDelayTime;

		tstring strDelayTime = GetUIObjectText(_T("CID_E_DELAY_TIME"));
		nDelayTime = _ttoi(strDelayTime.c_str());
		if (nDelayTime > 0)
		{
			if (nDelayTime < 10)
				nDelayTime = 10;
			if (nDelayTime > 10000)
				nDelayTime = 10000;

			if (nDelayTime != g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), 10))
			{
				CMPlayerSettings::SetLyricsDisplaySettings(_T("SlideDelayTime"), nDelayTime);
			}
		}

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_BR_PIC_FOLDER")))
		{
			OnCommandSetBkImg();

			SetUIObjectText(_T("CID_E_PIC_FOLDER"), 
				CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

	void OnCommandSetBkImg()
	{
		CFolderDialog	dlg;

		dlg.SetInitFolder(CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());
		if (dlg.DoBrowse(m_pSkinWnd))
		{
			CMPlayerSettings::SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), SZ_TRUE);
			CMPlayerSettings::SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), dlg.GetFolder());
			CMLProfile::WriteDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), dlg.GetFolder());
		}
	}

protected:

};

//////////////////////////////////////////////////////////////////////////

class CPagePfDisplayStyle : public CPagePfBase, public IPopupSkinWndNotify
{
protected:
	enum COLOR_SET_TYPE
	{
		CS_OUTLINED_HIGH_BORDER,
		CS_OUTLINED_LOW_BORDER,
		CS_BG,
		CS_SIZE,
	};

	struct ColorOption
	{
		ColorOption() {
			pButton = NULL;
			nButtonID = 0;
			clrDefault = 0;
		}

		CSkinImageButton	*pButton;
		MLColor		clr;
		COLORREF	clrDefault;
		int			nButtonID;
		tstring		strProfileKey;
	};

	ColorOption		m_ColorSet[CS_SIZE];

public:
	virtual void PopupSkinWndOnSelected()
	{
		CreateLOBXlightBmp(true);
		CreateLOBXlightBmp(false);
		InvalidateUIObject(CID_DS_CLR_HIGH);
		InvalidateUIObject(CID_DS_CLR_LOW);
	}

public:
	CPagePfDisplayStyle(bool bFloatingLyrMode) : CPagePfBase(GetPageID(), PAGE_LYR_DISPLAY, _T("CMD_THEME_LYR_DISPLAY"))
	{
 		if (bFloatingLyrMode)
 			m_eventType = ET_LYRICS_FLOATING_SETTINGS;
 		else
			m_eventType = ET_LYRICS_DISPLAY_SETTINGS;
		CID_DS_MODE = 0;
		CID_DS_STYLE = 0;
		CID_DS_FADEOUT = 0;
		CID_DS_ALIGN = 0;
		CID_DS_LOAD_LATIN_FONT = 0;
		CID_DS_LOAD_OTHER_FONT = 0;
		CID_DS_CLR_HIGH = 0;
		CID_DS_CLR_LOW = 0;
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_DISPLAY_STYLE"); }

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		GET_ID_BY_NAME(CID_DS_MODE);
		GET_ID_BY_NAME(CID_DS_STYLE);
		GET_ID_BY_NAME(CID_DS_FADEOUT);
		GET_ID_BY_NAME(CID_DS_ALIGN);
		GET_ID_BY_NAME(CID_DS_LOAD_LATIN_FONT);
		GET_ID_BY_NAME(CID_DS_LOAD_OTHER_FONT);
		GET_ID_BY_NAME(CID_DS_CLR_HIGH);
		GET_ID_BY_NAME(CID_DS_CLR_LOW);

		m_pcbMode = (CSkinComboBox *)GetUIObjectById(CID_DS_MODE, CSkinComboBox::ClassName());
		if (m_pcbMode)
		{
			m_pcbMode->AddString(TOLOCAL(_T("Normal lyrics mode settings")));
			m_pcbMode->AddString(TOLOCAL(_T("Floating lyrics mode settings")));

			if (m_eventType == ET_LYRICS_FLOATING_SETTINGS)
				m_pcbMode->SetCurSel(1);
			else
				m_pcbMode->SetCurSel(0);

			m_pSkinWnd->RegisterUIObjNotifyHandler(CID_DS_MODE, this);
		}

		SetSettingType(m_eventType == ET_LYRICS_FLOATING_SETTINGS ? ST_FLOATING_LYR : ST_NORMAL_LYR);
	}

	void OnUIObjNotify(IUIObjNotify *pNotify)
	{
		CPagePfBase::OnUIObjNotify(pNotify);

		if (pNotify->IsKindOf(CSkinComboBox::ClassName()))
		{
			if (pNotify->nID == CID_DS_MODE)
			{
				ASSERT(m_pcbMode);
				if (m_pcbMode)
				{
					SetSettingType(m_pcbMode->GetCurSel() == 0 ? ST_NORMAL_LYR : ST_FLOATING_LYR);
				}
			}
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_DS_LOAD_LATIN_FONT || nId == CID_DS_LOAD_OTHER_FONT)
		{
			int		nHeight, nWeight;
			BYTE	byItalic;
			tstring	strFaceNameLatin9, strFaceNameOthers;
			int		nRet;
			bool	bLatin9 = (nId == CID_DS_LOAD_LATIN_FONT);

			// get default settings in profile
			ProfileGetLyricsFont(m_strSectName.c_str(), nHeight, nWeight, 
				byItalic, strFaceNameLatin9, strFaceNameOthers);

			CDlgChooseFont		chooser;

			nRet = chooser.DoModal(m_pSkinWnd, bLatin9 ? strFaceNameLatin9.c_str() : strFaceNameOthers.c_str(), 
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
		else if (nId == CID_DS_CLR_HIGH || nId == CID_DS_CLR_LOW)
		{
			CRect	rc;
			GetUIObjectRect(nId, rc);

			m_pSkinWnd->ClientToScreen(rc);

			// Popup list window
			m_popupHighClrListWnd.Create(m_pSkinWnd, rc,
				nId == CID_DS_CLR_HIGH, m_eventType == ET_LYRICS_FLOATING_SETTINGS, this);
		}
		else
		{
			//
			// lyrics display Color settings
			//
			for (int i = 0; i < CountOf(m_ColorSet); i++)
			{
				if (nId == m_ColorSet[i].nButtonID)
				{
					tstring		strColor;

					if (m_ColorSet[i].pButton)
					{
						m_ColorSet[i].clr = m_ColorSet[i].pButton->GetColor();

						ColorToStr(m_ColorSet[i].clr, strColor);
						CMPlayerSettings::SetSettings(m_eventType, m_strSectName.c_str(), m_ColorSet[i].strProfileKey.c_str(), strColor.c_str());
					}
					return true;
				}
			}

			return CPagePfBase::OnCustomCommand(nId);
		}

		return true;
	}

protected:
	enum SettingType
	{
		ST_NORMAL_LYR		= 0,
		ST_FLOATING_LYR		= 1,
	};

	void SetSettingType(SettingType settingType)
	{
		ClearOptions();

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

		{
			// Lyrics border colors and background
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

			m_ColorSet[CS_OUTLINED_HIGH_BORDER].nButtonID = GetIDByName(_T("CID_DS_CLR_HIGH_BORDER"));
			m_ColorSet[CS_OUTLINED_LOW_BORDER].nButtonID = GetIDByName(_T("CID_DS_CLR_LOW_BORDER"));
			m_ColorSet[CS_BG].nButtonID = GetIDByName(_T("CID_DS_CLR_BG"));

			for (int i = 0; i < CountOf(m_ColorSet); i++)
			{
				m_ColorSet[i].pButton = (CSkinImageButton*)GetUIObjectById(m_ColorSet[i].nButtonID, CSkinImageButton::ClassName());
				if (m_ColorSet[i].pButton)
				{
					m_ColorSet[i].clr.set(m_ColorSet[i].clrDefault);

					ProfileGetColorValue(m_ColorSet[i].clr, 
						m_strSectName.c_str(), m_ColorSet[i].strProfileKey.c_str());

					m_ColorSet[i].pButton->SetColor(m_ColorSet[i].clr, true);
				}
			}
		}

		CSkinComboBox		*m_pcbStyle;
		m_pcbStyle = (CSkinComboBox *)GetUIObjectById(CID_DS_STYLE, CSkinComboBox::ClassName());
		if (m_pcbStyle)
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
			opt.nIDWidgetCombo = CID_DS_STYLE;
			opt.Set(ET_UI_SETTINGS_CHANGED,
				m_strSectName.c_str(), 
				_T("LyrDisplayStyle"),
				CLyricShowMultiRowObj::ClassName());

			m_pcbStyle->ResetContent();
			ASSERT(CountOf(vszItemName) == CountOf(vszItemValues));
			for (int i = 0; i < CountOf(vszItemValues); i++)
			{
				m_pcbStyle->InsertString(i, TOLOCAL(vszItemName[i]));
				opt.AddItemValue(vszItemValues[i]);
			}
			AddOptComboStr(opt);
		}

		CSkinComboBox		*m_pcbAlign;
		m_pcbAlign = (CSkinComboBox *)GetUIObjectById(CID_DS_ALIGN, CSkinComboBox::ClassName());
		if (m_pcbAlign)
		{
			//
			// lyrics align
			//
			LPCTSTR		vszItemName[] = { _T("Left"), _T("Center"), _T("Right") };
			LPCTSTR		vszItemValues[] = { _T("left"), _T("center"), _T("right") };

			OptComboStr		opt;
			opt.nIDWidgetCombo = CID_DS_ALIGN;
			opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrAlign"), _T("center"));

			ASSERT(CountOf(vszItemName) == CountOf(vszItemValues));
			m_pcbAlign->ResetContent();
			for (int i = 0; i < CountOf(vszItemValues); i++)
			{
				m_pcbAlign->InsertString(i, TOLOCAL(vszItemName[i]));
				opt.AddItemValue(vszItemValues[i]);
			}
			AddOptComboStr(opt);
		}

		CSkinComboBox		*m_pcbFadeOut;
		m_pcbFadeOut = (CSkinComboBox *)GetUIObjectById(CID_DS_FADEOUT, CSkinComboBox::ClassName());
		if (m_pcbFadeOut)
		{
			// lyrics Fade out options
			LPCTSTR		vszItemName[] = { _T("Normal"), _T("Fade in lyrics"), _T("Fade out background") };

			OptComboStr		opt;
			opt.nIDWidgetCombo = CID_DS_FADEOUT;
			opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrDrawOpt"), _T("fadein"));

			ASSERT(CountOf(vszItemName) == DO_MAX);
			m_pcbFadeOut->ResetContent();
			for (int i = 0; i < CountOf(vszItemName); i++)
			{
				m_pcbFadeOut->InsertString(i, TOLOCAL(vszItemName[i]));
				opt.AddItemValue(g_idsLyrDisplayOpt[i].szId);
			}
			AddOptComboStr(opt);
		}

		// karaoke option
		AddOptBool(m_eventType, m_strSectName.c_str(), _T("Karaoke"), false, _T("CID_C_KARAOKE"));

		AddOptBool(m_eventType, m_strSectName.c_str(), _T("OutlineLyrText"), false, _T("CID_C_DRAW_SHADOW"));

		InitCheckButtons();

		CreateLOBXlightBmp(true);
		CreateLOBXlightBmp(false);

		m_pSkinWnd->InvalidateRect();
	}

	void CreateLOBXlightBmp(bool bHilight)
	{
		CRect					rc;
		CLyricShowObj::TextOverlayBlending		tob;

		if (!GetUIObjectRect(GetIDByName(_T("CID_DS_CLR_HIGH")), rc))
			return;

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
			CSkinImageButton	*pButton;
			pButton = (CSkinImageButton*)GetUIObjectById(
				bHilight ? CID_DS_CLR_HIGH : CID_DS_CLR_LOW, CSkinImageButton::ClassName());

			if (pButton)
			{
				pButton->SetContentImage(tob.imgPattern.GetHandle());
				tob.imgPattern.Detach();
			}
		}
	}

protected:
	EventType			m_eventType;
	tstring				m_strSectName;

	int					CID_DS_MODE, CID_DS_STYLE, CID_DS_FADEOUT, CID_DS_ALIGN;
	int					CID_DS_LOAD_LATIN_FONT, CID_DS_LOAD_OTHER_FONT, CID_DS_CLR_HIGH, CID_DS_CLR_LOW;

	CSkinComboBox		*m_pcbMode;

	CLyrDisplayClrListWnd	m_popupHighClrListWnd;
	CLyrDisplayClrListWnd	m_popupLowClrListWnd;

};

//////////////////////////////////////////////////////////////////////////

class CPagePfTheme : public CPagePfBase
{
public:
	CPagePfTheme() : CPagePfBase(GetPageID(), PAGE_THEMES, _T("CMD_THEME_THEME"))
	{
		m_nThemeListID = 0;
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME"); }

	virtual void OnUIObjNotify(IUIObjNotify *pNotify)
	{
		if (pNotify->nID == m_nThemeListID && pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED)
			{
				tstring	strTheme;
				int nSel = m_pThemeList->GetNextSelectedItem();
				if (nSel != -1)
					m_pThemeList->GetItemText(nSel, 0, strTheme);
				SetUIObjectText(_T("CID_E_THEMENAME"), strTheme.c_str(), true);
			}
		}
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		m_nThemeListID = GetIDByName(_T("CID_THME_LIST"));
		m_pSkinWnd->RegisterUIObjNotifyHandler(m_nThemeListID, this);

		m_pThemeList = (CSkinListCtrl *)GetUIObjectById(m_nThemeListID, CSkinListCtrl::ClassName());
		ASSERT(m_pThemeList);

		if (m_pThemeList)
			m_pThemeList->AddColumn(_T("Theme"), 300);

		UpdateThemesList();
	}

	void OnDestroy()
	{
		m_pSkinWnd->UnregisterUIObjNotifyHandler(this);

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_LOAD_THEME")))
		{
// 			m_pParent->SetProperty(_T("ThemeItemsLoadAction"), true);
// 			m_pParent->SwitchToPage(CPagePfThemeItems::GetPageID());
			LoadTheme();
		}
		else if (nId == GetIDByName(_T("CID_SAVE_THEME")))
		{
			SaveTheme();
// 			tstring strThemeName = GetUIObjectText(_T("CID_E_THEMENAME"));
// 			if (strThemeName.empty())
// 			{
// 				m_pSkinWnd->MessageOut(TOLOCAL(_T("Please enter the theme name.")));
// 				return true;
// 			}
// 
// 			m_pParent->SetProperty(_T("ThemeItemsLoadAction"), false);
// 			m_pParent->SwitchToPage(CPagePfThemeItems::GetPageID());
		}
		else if (nId == GetIDByName(_T("CID_DELETE_THEME")))
		{
			DeleteTheme();
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

protected:
	void LoadTheme()
	{
		// Load theme
		int				nSel;

		nSel = m_pThemeList->GetNextSelectedItem(-1);
		if (nSel == -1)
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Please select the theme from list.")));
			return;
		}

		CThemesFile		file;
		tstring			strSelTheme;

		m_pThemeList->GetItemText(nSel, 0, strSelTheme);
		file.Open();

		g_Profile.WriteProfileString(_T("CurTheme"), strSelTheme.c_str());
		file.SetCurTheme(strSelTheme.c_str());
	}

	void SaveTheme()
	{
		// Save theme
		CThemesFile		file;

		tstring strThemeName = GetUIObjectText(_T("CID_E_THEMENAME"));
		if (strThemeName.empty())
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Please enter the theme name.")));
			return;
		}

		file.Open();

		file.UseCurSettingAsTheme(strThemeName.c_str());

		file.Save();

		UpdateThemesList();
	}

	void DeleteTheme()
	{
		int				nSel;

		nSel = m_pThemeList->GetNextSelectedItem(-1);
		if (nSel == -1)
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Please select the theme from list.")));
			return;
		}

		CThemesFile		file;
		tstring			strSelTheme;

		m_pThemeList->GetItemText(nSel, 0, strSelTheme);
		file.Open();

		file.Delete(strSelTheme.c_str());
		file.Save();
		UpdateThemesList();

		SetUIObjectText(_T("CID_E_THEMENAME"), _T(""), true);
	}

	void UpdateThemesList()
	{
		if (!m_pThemeList)
			return;

		CThemesFile		file;
		V_Str			vThemes;
		tstring			strCurTheme;
		int				nSel =  -1;

		file.Open();
		file.EnumAllThemes(vThemes);

		m_pThemeList->DeleteAllItems();

		strCurTheme = g_Profile.GetProfileString(_T("CurTheme"), _T(""));

		for (int i = 0; i < (int)vThemes.size(); i++)
		{
			m_pThemeList->InsertItem(i, vThemes[i].c_str());
			if (nSel == -1 && _tcscmp(strCurTheme.c_str(), vThemes[i].c_str()))
				nSel = i;
		}

		if (nSel != -1)
			m_pThemeList->SetItemSelectionState(nSel, true);
	}

protected:
	CSkinListCtrl		*m_pThemeList;
	int					m_nThemeListID;

};

//////////////////////////////////////////////////////////////////////////

CPagePfThemeRoot::CPagePfThemeRoot(bool bFloatingLyrMode) : CPagePfBase(GetPageID(), PAGE_UNKNOWN, _T("CMD_ROOT_THEME"))
{
	AddPage(new CPagePfTheme());
	AddPage(new CPagePfDisplayStyle(bFloatingLyrMode));
	AddPage(new CPagePfLyricsBgPic());
	// AddPage(new CPagePfThemeItems());
}

LPCTSTR CPagePfThemeRoot::GetPageID()
{
	return _T("CID_C_THEME_ROOT");
}

void CPagePfThemeRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_THEME"));
}

/*
bool CPagePfThemeRoot::OnCustomCommand(int nId)
{
	if (nId == GetIDByName(_T("CMD_THEME_THEME")))
		SwitchToPage(CPagePfTheme::GetPageID());
	else if (nId == GetIDByName(_T("CMD_THEME_LYR_DISPLAY")))
		SwitchToPage(CPagePfDisplayStyle::GetPageID());
	else if (nId == GetIDByName(_T("CMD_THEME_LYR_BG_PIC")))
		SwitchToPage(CPagePfLyricsBgPic::GetPageID());
	else
		return CPagePfBase::OnCustomCommand(nId);

	return true;
}*/
