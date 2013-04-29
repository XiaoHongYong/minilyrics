#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageTheme.h"
#include "LyricShowAgentObj.h"
#include "ThemesFile.h"
#include "LyrDisplayClrListWnd.h"

//////////////////////////////////////////////////////////////////////////

class CPagePfLyricsBgPic : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfLyricsBgPic() : CPagePfBase(PAGE_LYR_BG, _T("CMD_THEME_LYR_BG_PIC"))
	{
		m_bIntialized = false;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		m_bIntialized = true;

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseAlbumArtAsBg"), false, _T("CID_C_USE_ALBUM_ART_AS_BG"));

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), false, _T("CID_C_ENABLE_BG_IMAGE"));

		AddOptBool(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("DarkenLyrBg"), true, _T("CID_C_DARKEN_LYR_BG"));

		SetUIObjectText(_T("CID_E_DELAY_TIME"), 
			g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), _T("10")));

		InitCheckButtons();

		SetUIObjectText(_T("CID_E_PIC_FOLDER"), 
			CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());

	}

	void OnDestroy()
	{
		if (m_bIntialized)
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

				if (nDelayTime != g_Profile.GetInt(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), 10))
				{
					CMPlayerSettings::SetLyricsDisplaySettings(_T("SlideDelayTime"), nDelayTime);
				}
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
		if (dlg.DoBrowse(m_pSkin))
		{
			CMPlayerSettings::SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), SZ_TRUE);
			CMPlayerSettings::SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), dlg.GetFolder());
			CMLProfile::WriteDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), dlg.GetFolder());
		}
	}

protected:
	bool		m_bIntialized;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyricsBgPic, _T("PreferPage.LyrBgPic"))

//////////////////////////////////////////////////////////////////////////

class CPagePfDisplayStyle : public CPagePfBase, public IPopupSkinWndNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
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
	CPagePfDisplayStyle() : CPagePfBase(PAGE_LYR_DISPLAY, _T("CMD_THEME_LYR_DISPLAY"))
	{
		CID_DS_MODE = 0;
		CID_DS_STYLE = 0;
		CID_DS_FADEOUT = 0;
		CID_DS_ALIGN = 0;
		CID_DS_LOAD_LATIN_FONT = 0;
		CID_DS_LOAD_OTHER_FONT = 0;
		CID_DS_CLR_HIGH = 0;
		CID_DS_CLR_LOW = 0;
		m_eventType = ET_LYRICS_DISPLAY_SETTINGS;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		if (GetExPoolBool(_T("FloatingLyrMode"), false))
			m_eventType = ET_LYRICS_FLOATING_SETTINGS;
		else
			m_eventType = ET_LYRICS_DISPLAY_SETTINGS;

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
			m_pcbMode->AddString(_TLT("Normal lyrics mode settings"));
			m_pcbMode->AddString(_TLT("Floating lyrics mode settings"));

			if (m_eventType == ET_LYRICS_FLOATING_SETTINGS)
				m_pcbMode->SetCurSel(1);
			else
				m_pcbMode->SetCurSel(0);

			m_pSkin->RegisterUIObjNotifyHandler(CID_DS_MODE, this);
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
			int		nSize, nWeight;
			BYTE	byItalic;
			tstring	strFaceNameLatin9, strFaceNameOthers;
			int		nRet;
			bool	bLatin9 = (nId == CID_DS_LOAD_LATIN_FONT);

			// get default settings in profile
			ProfileGetLyricsFont(m_strSectName.c_str(), nSize, nWeight, 
				byItalic, strFaceNameLatin9, strFaceNameOthers);

			CDlgChooseFont		chooser;

			nRet = chooser.DoModal(m_pSkin, bLatin9 ? strFaceNameLatin9.c_str() : strFaceNameOthers.c_str(), 
				nSize, nWeight, byItalic != 0);
			if (nRet == IDOK)
			{
				nSize = chooser.GetSize();
				if (nSize < 0)
					nSize = -nSize;

				// now save to profile
				ProfileWriteLyricsFont(m_eventType, m_strSectName.c_str(), nSize, chooser.GetWeight(), 
					chooser.GetItalic(), bLatin9 ? chooser.GetFaceName() : NULL, bLatin9 ? NULL : chooser.GetFaceName());
			}
		}
		else if (nId == CID_DS_CLR_HIGH || nId == CID_DS_CLR_LOW)
		{
			CRect	rc;
			GetUIObjectRect(nId, rc);

			m_pSkin->ClientToScreen(rc);

			// Popup list window
			m_popupHighClrListWnd.Create(m_pSkin, rc,
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
					if (m_ColorSet[i].pButton)
					{
						m_ColorSet[i].clr = m_ColorSet[i].pButton->GetColor();

						tstring strColor = ColorToStr(m_ColorSet[i].clr);
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

		CMPlayerApp::GetInstance()->GetCurLyrDisplaySettingName(settingType == ST_FLOATING_LYR, m_strSectName, m_eventType);

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
			LPCTSTR		vszItemName[] = { _TLM("Vertical scrolling"), _TLM("Double lines"), _TLM("Single line"),
				_TLM("Horizontal scrolling"), _TLM("Static text"), _TLM("Movie subtitle") };

			LPCTSTR		vszItemValues[] = { CLyricShowMultiRowObj::ClassName(), CLyricShowTwoRowObj::ClassName(), 
				CLyricShowSingleRowObj::ClassName(), CLyricShowHorzScrollObj::ClassName(), 
				SZ_TXT_LYR_CONTAINER, CLyricShowVobSub::ClassName() };

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
				m_pcbStyle->InsertString(i, _TL(vszItemName[i]));
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
			LPCTSTR		vszItemName[] = { _TLM("Left"), _TLM("Center"), _TLM("Right") };
			LPCTSTR		vszItemValues[] = { _T("left"), _T("center"), _T("right") };

			OptComboStr		opt;
			opt.nIDWidgetCombo = CID_DS_ALIGN;
			opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrAlign"), _T("center"));

			ASSERT(CountOf(vszItemName) == CountOf(vszItemValues));
			m_pcbAlign->ResetContent();
			for (int i = 0; i < CountOf(vszItemValues); i++)
			{
				m_pcbAlign->InsertString(i, _TL(vszItemName[i]));
				opt.AddItemValue(vszItemValues[i]);
			}
			AddOptComboStr(opt);
		}

		CSkinComboBox		*m_pcbFadeOut;
		m_pcbFadeOut = (CSkinComboBox *)GetUIObjectById(CID_DS_FADEOUT, CSkinComboBox::ClassName());
		if (m_pcbFadeOut)
		{
			// lyrics Fade out options
			LPCTSTR		vszItemName[] = { _TLM("Automatic"), _TLM("Normal"), _TLM("Fade in lyrics"),
				_TLM("Fade out background") };

			OptComboStr		opt;
			opt.nIDWidgetCombo = CID_DS_FADEOUT;
			opt.Set(m_eventType, m_strSectName.c_str(), _T("LyrDrawOpt"), _T("fadein"));

			ASSERT(CountOf(vszItemName) == DO_MAX);
			m_pcbFadeOut->ResetContent();
			for (int i = 0; i < CountOf(vszItemName); i++)
			{
				m_pcbFadeOut->InsertString(i, _TL(vszItemName[i]));
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

		m_pSkin->InvalidateRect();
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

UIOBJECT_CLASS_NAME_IMP(CPagePfDisplayStyle, _T("PreferPage.LyrDisplayStyle"))

//////////////////////////////////////////////////////////////////////////

class CPagePfTheme : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfTheme() : CPagePfBase(PAGE_THEMES, _T("CMD_THEME_THEME"))
	{
		m_nThemeListID = 0;
	}

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
		m_pSkin->RegisterUIObjNotifyHandler(m_nThemeListID, this);

		m_pThemeList = (CSkinListCtrl *)GetUIObjectById(m_nThemeListID, CSkinListCtrl::ClassName());
		ASSERT(m_pThemeList);

		if (m_pThemeList)
			m_pThemeList->AddColumn(_T("Theme"), 300);

		UpdateThemesList();
	}

	void OnDestroy()
	{
		m_pSkin->UnregisterUIObjNotifyHandler(this);

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_LOAD_THEME")))
		{
			LoadTheme();
		}
		else if (nId == GetIDByName(_T("CID_SAVE_THEME")))
		{
			SaveTheme();
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
			m_pSkin->MessageOut(_TLT("Please select the theme from list."));
			return;
		}

		CThemesFile		file;
		tstring			strSelTheme;

		m_pThemeList->GetItemText(nSel, 0, strSelTheme);
		file.Open();

		g_Profile.WriteString(_T("CurTheme"), strSelTheme.c_str());
		file.SetCurTheme(strSelTheme.c_str());
	}

	void SaveTheme()
	{
		// Save theme
		CThemesFile		file;

		tstring strThemeName = GetUIObjectText(_T("CID_E_THEMENAME"));
		if (strThemeName.empty())
		{
			m_pSkin->MessageOut(_TLT("Please enter the theme name."));
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
			m_pSkin->MessageOut(_TLT("Please select the theme from list."));
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

		strCurTheme = g_Profile.GetString(_T("CurTheme"), _T(""));

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

UIOBJECT_CLASS_NAME_IMP(CPagePfTheme, _T("PreferPage.Theme"))

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfThemeRoot, _T("PreferPage.ThemeRoot"))

CPagePfThemeRoot::CPagePfThemeRoot() : CPagePfBase(PAGE_UNKNOWN, _T("CMD_ROOT_THEME"))
{
}

void CPagePfThemeRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_THEME"));
}

void RegisterPfThemePages(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPagePfThemeRoot);
	AddUIObjNewer2(pSkinFactory, CPagePfTheme);
	AddUIObjNewer2(pSkinFactory, CPagePfDisplayStyle);
	AddUIObjNewer2(pSkinFactory, CPagePfLyricsBgPic);
}
