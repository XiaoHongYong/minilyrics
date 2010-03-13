#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageTheme.h"
#include "LyricShowAgentObj.h"
#include "DlgLyrBlendingOpt.h"
#include "ThemesFile.h"
#include "SkinListCtrl.h"
#include "SkinComboBox.h"

//////////////////////////////////////////////////////////////////////////

class CPagePfThemeItems : public CPagePfBase
{
public:
	CPagePfThemeItems() : CPagePfBase(GetPageID())
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_ITEMS"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();
	}

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

};

//////////////////////////////////////////////////////////////////////////

class CPagePfLyricsBgPic : public CPagePfBase
{
public:
	CPagePfLyricsBgPic() : CPagePfBase(GetPageID())
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_LYR_BG"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();

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
			if (g_mpApp.GetMainWnd())
				g_mpApp.GetMainWnd()->OnCommand(IDC_SEL_BKIMAGE, 0);

			SetUIObjectText(_T("CID_E_PIC_FOLDER"), 
				CMLProfile::GetDir(SZ_SECT_LYR_DISPLAY, _T("BgPicFolder"), _T("")).c_str());
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

protected:

};

//////////////////////////////////////////////////////////////////////////

void LoadLyrOverlayBlendingSettings(LPCTSTR szSectName, CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight, bool bHilight);

void CreateGradientFillImage24bpp(CRawImage &image, int nHeight, CLyricShowObj::MLColorEx clrGradient[3]);

tstring GetPatternDir()
{
	tstring strPatternDir = GetWorkingFolder();
	strPatternDir += _T("Patterns") _T(SZ_DIR_SLASH);

#if defined (_DEBUG) && defined (_WIN32)
	if (!IsDirExist(strPatternDir.c_str()))
	{
		strPatternDir = GetInstallShareDir();
		strPatternDir += _T("Patterns\\");
	}
#endif

	return strPatternDir;
}

class CLyrDisplayClrListWnd : public IPopupSkinWndNotify
{
public:
	struct FontClrOpt
	{
		FontClrOpt(COLORREF clr) {
			this->obm = OBM_COLOR;
			this->clr[0] = clr;
		}
		FontClrOpt(COLORREF clr0, COLORREF clr1, COLORREF clr2) {
			this->obm = OBM_GRADIENT_COLOR;
			this->clr[0] = clr0;
			this->clr[1] = clr1;
			this->clr[2] = clr2;
		}
		FontClrOpt(LPCTSTR szFile) {
			this->obm = OBM_PATTERN;
			strPatternFile = szFile;
		}

		OverlayBlendingMode		obm;				// Overlay blending mode
		COLORREF				clr[3];
		tstring					strPatternFile;
	};
	typedef vector<FontClrOpt>	VecFontClr;

public:
	CLyrDisplayClrListWnd()
	{
		m_pWnd = NULL;
		m_pNotify = NULL;
		m_bInitialized = false;
	}

	void Create(CSkinWnd *pWnd, CRect &rc, bool bHighLight, bool bFloatingLyr, IPopupSkinWndNotify *pNotify)
	{
		m_pWnd = pWnd;
		m_pNotify = pNotify;

		if (!m_bInitialized)
		{
			m_popupListWnd.AddImageColumn(_T(""), 100);
			m_bInitialized = true;
		}

		if (bFloatingLyr)
		{
			m_et = ET_LYRICS_FLOATING_SETTINGS;
			m_strSectName = SZ_SECT_FLOATING_LYR;
		}
		else
		{
			m_et = ET_LYRICS_DISPLAY_SETTINGS;
			m_strSectName = SZ_SECT_LYR_DISPLAY;
		}
		m_bHilight = bHighLight;

		m_nWidthMax = 0;
		m_popupListWnd.DeleteAllItems();
		m_vFontClr.clear();

		MLColor	clr(RGB(255, 255, 255));
		ProfileGetColorValue(clr,
			m_strSectName.c_str(),
			GetLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_FILL));
		AddPureColor(clr.get());

		AddGradientColors();

		AddPatternImages();

		SelectCurrentSetting();

		m_popupListWnd.SetLineHeight(HEIGHT_COLOR);

		if (m_nWidthMax > rc.Width())
			rc.right = rc.left + m_nWidthMax + 60;

		// Popup list window
		m_popupListWnd.Create(pWnd->GetSkinFactory(), _T("ComboBoxPopup.xml"),
			pWnd, this, rc);
	}

	virtual void PopupSkinWndOnSelected()
	{
		int	nSel = m_popupListWnd.GetCurSel();
		m_popupListWnd.PostDestroy();

		if (nSel == -1)
			return;
		ASSERT(nSel >= 0 && nSel < m_vFontClr.size());

		// Save selected color
		FontClrOpt &fontClr = m_vFontClr[nSel];
		tstring strColor;

		if (fontClr.obm == OBM_COLOR)
		{
			CDlgChooseColor		dlg;

			if (dlg.DoModal(m_pWnd, fontClr.clr[0]) != IDOK)
				return;

			fontClr.clr[0] = dlg.GetColor().get();

			ColorToStr(fontClr.clr[0], strColor);
			CMPlayerSettings::SetSettings(m_et, m_strSectName.c_str(), 
				GetLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_FILL), strColor.c_str());
		}
		else if (fontClr.obm == OBM_GRADIENT_COLOR)
		{
			for (int i = CLyricShowObj::TCI_GRADIENT1; i <= CLyricShowObj::TCI_GRADIENT3; i++)
			{
				ColorToStr(fontClr.clr[i - CLyricShowObj::TCI_GRADIENT1], strColor);
				CMPlayerSettings::SetSettings(m_et, m_strSectName.c_str(), 
					GetLyrTOBSettingName(m_bHilight, (CLyricShowObj::TOBColorIndex)i), strColor.c_str());
			}
		}
		else
		{
			ASSERT(fontClr.obm == OBM_PATTERN);
			CMPlayerSettings::SetSettings(m_et, m_strSectName.c_str(), 
				GetLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_PATTERN), fontClr.strPatternFile.c_str());
		}

		CMPlayerSettings::SetSettings(m_et, m_strSectName.c_str(), 
			GetLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_OBM), fontClr.obm);

		if (m_pNotify)
			m_pNotify->PopupSkinWndOnSelected();
	}

protected:
	enum {
		HEIGHT_COLOR = 40,
		WIDTH_COLOR = 100
	};

	void SelectCurrentSetting()
	{
		CLyricShowObj::TextOverlayBlending		tob;
		int		nToSelect = 0;

		LoadLyrOverlayBlendingSettings(m_strSectName.c_str(), tob, HEIGHT_COLOR, m_bHilight);

		if (tob.obm == OBM_COLOR)
		{
			nToSelect = 0;
			// AddPureColor(tob.clr[CLyricShowObj::TCI_FILL].get());
		}
		else if (tob.obm == OBM_GRADIENT_COLOR)
		{
			nToSelect = FindGradientColor(tob.clr[CLyricShowObj::TCI_GRADIENT1].get(), 
				tob.clr[CLyricShowObj::TCI_GRADIENT2].get(), 
				tob.clr[CLyricShowObj::TCI_GRADIENT3].get());
			if (nToSelect == -1)
			{
				AddGradientColor(tob.clr[CLyricShowObj::TCI_GRADIENT1].get(), 
								tob.clr[CLyricShowObj::TCI_GRADIENT2].get(), 
								tob.clr[CLyricShowObj::TCI_GRADIENT3].get());
				nToSelect = m_vFontClr.size() - 1;
			}
		}
		else if (tob.obm == OBM_PATTERN)
		{
			nToSelect = FindPattern(tob.strPatternFile.c_str());
			if (nToSelect == -1 && tob.imgPattern.IsValid())
			{
				AddImage(tob.imgPattern.GetHandle());
				m_vFontClr.push_back(FontClrOpt(tob.strPatternFile.c_str()));
				tob.imgPattern.Detach();
				nToSelect = m_vFontClr.size() - 1;
			}
		}

		m_popupListWnd.SetCurSel(nToSelect);
	}

	int FindGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2)
	{
		for (int i = 0; i < (int)m_vFontClr.size(); i++)
		{
			FontClrOpt &opt = m_vFontClr[i];
			if (opt.obm == OBM_GRADIENT_COLOR)
			{
				if (opt.clr[0] == clr0 && opt.clr[1] == clr1 && opt.clr[2] == clr2)
					return i;
			}
		}

		return -1;
	}

	int FindPattern(LPCTSTR szPatternFile)
	{
		for (int i = 0; i < (int)m_vFontClr.size(); i++)
		{
			FontClrOpt &opt = m_vFontClr[i];
			if (opt.obm == OBM_PATTERN)
			{
				if (_tcsicmp(opt.strPatternFile.c_str(), szPatternFile) == 0)
					return i;
			}
		}

		return -1;
	}

	void AddPureColor(COLORREF clr)
	{
		PRAW_IMAGE_DATA		pRawImage;
		MLColor	color(clr);

		pRawImage = new RAW_IMAGE_DATA;
		pRawImage->Create(WIDTH_COLOR, HEIGHT_COLOR, 24);
		RawImageSet(pRawImage, color.r(), 
			color.g(), 
			color.b(),
			255);

		AddImage(pRawImage);
		m_vFontClr.push_back(FontClrOpt(clr));
	}

	void AddGradientColors()
	{
		AddGradientColor(RGB(255, 255, 128), RGB(255, 128, 0), RGB(255, 128, 128));
		AddGradientColor(RGB(128, 255, 255), RGB(0, 128, 255), RGB(128, 255, 255));
		// AddGradientColor(RGB(, , ), RGB(, , ), RGB(, , ));
	}

	void AddGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2)
	{
		CRawImage	image;
		CLyricShowObj::MLColorEx clr[3];

		clr[0].set(clr0);
		clr[1].set(clr1);
		clr[2].set(clr2);

		CreateGradientFillImage24bpp(image, HEIGHT_COLOR, clr);

		if (image.IsValid())
		{
			AddImage(image.GetHandle());
			image.Detach();

			m_vFontClr.push_back(FontClrOpt(clr0, clr1, clr2));
		}
	}

	void AddPatternImages()
	{
		tstring		strPatternDir = GetPatternDir();

		V_Str		vFiles;
		EnumFilesInDir(strPatternDir.c_str(), NULL, vFiles, true);

		for (int i = 0; i < (int)vFiles.size(); i++)
		{
			PRAW_IMAGE_DATA	hImage = LoadRawImageDataFromFile(vFiles[i].c_str());
			if (hImage)
			{
				AddImage(hImage);
				m_vFontClr.push_back(FontClrOpt(FileGetName(vFiles[i].c_str())));
			}
		}
	}

	void AddImage(PRAW_IMAGE_DATA hImage)
	{
		ASSERT(hImage);
		if (!hImage)
			return;

		ASSERT(m_vFontClr.size() == m_popupListWnd.GetItemCount());

		int n = m_popupListWnd.InsertItem(m_popupListWnd.GetItemCount(), _T(""), 0, 0);

		m_popupListWnd.SetItemImage(n, 0, hImage);
		// const int LINE_SPACE = 4;
		// if (m_popupListWnd.GetLineHeight() < hImage->height + LINE_SPACE)
		//	m_popupListWnd.SetLineHeight(hImage->height + LINE_SPACE);
		if (m_nWidthMax < hImage->width)
			m_nWidthMax = hImage->width;
	}

protected:
	bool				m_bInitialized;
	CSkinWnd			*m_pWnd;
	IPopupSkinWndNotify	*m_pNotify;
	CPopupSkinListWnd	m_popupListWnd;
	int					m_nWidthMax;

	VecFontClr			m_vFontClr;

	EventType			m_et;
	tstring				m_strSectName;
	bool				m_bHilight;

};


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
	CPagePfDisplayStyle(bool bFloatingLyrMode) : CPagePfBase(GetPageID())
	{
 		if (bFloatingLyrMode)
 			m_eventType = ET_LYRICS_FLOATING_SETTINGS;
 		else
			m_eventType = ET_LYRICS_DISPLAY_SETTINGS;
	}

	static LPCTSTR GetPageID() { return _T("CID_C_THEME_DISPLAY_STYLE"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();

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
	CPagePfTheme() : CPagePfBase(GetPageID())
	{
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

	void OnCreate()
	{
		CPagePfBase::OnCreate();

		m_nThemeListID = GetIDByName(_T("CID_THME_LIST"));
		m_pSkinWnd->RegisterUIObjNotifyHandler(m_nThemeListID, this);

		m_pThemeList = (CSkinListCtrl *)GetUIObjectById(m_nThemeListID, CSkinListCtrl::ClassName());
		ASSERT(m_pThemeList);

		if (m_pThemeList)
			m_pThemeList->AddColumn(_T("Theme"), 300);

		UpdateThemesList();
	}

	void OnShow()
	{
		if (!m_pThemeList)
			return;

// 		if (m_pParent->GetPropertyInt(_T("ThemeItemsActionResult")) == IDOK)
// 		{
// 			m_pParent->SetProperty(_T("ThemeItemsActionResult"), IDCANCEL);
// 
// 			if (m_pParent->GetPropertyInt(_T("ThemeItemsLoadAction")))
// 			{
// 				LoadTheme();
// 			}
// 			else
// 			{
// 				SaveTheme();
// 			}
// 		}
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

CPagePfThemeRoot::CPagePfThemeRoot(bool bFloatingLyrMode) : CPagePfBase(GetPageID())
{
	AddPage(new CPagePfTheme());
	AddPage(new CPagePfDisplayStyle(bFloatingLyrMode));
	AddPage(new CPagePfLyricsBgPic());
	AddPage(new CPagePfThemeItems());
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
	else if (nId == GetIDByName(_T("CMD_THEME_LYR_DISPLAY")))
		SwitchToPage(CPagePfDisplayStyle::GetPageID());
	else if (nId == GetIDByName(_T("CMD_THEME_LYR_BG_PIC")))
		SwitchToPage(CPagePfLyricsBgPic::GetPageID());
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
