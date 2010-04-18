#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyrDisplayClrListWnd.h"


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

CLyrDisplayClrListWnd::CLyrDisplayClrListWnd()
{
	m_pWnd = NULL;
	m_pNotify = NULL;
	m_bInitialized = false;
	m_bHilight = true;
	m_et = ET_LYRICS_DISPLAY_SETTINGS;
	m_nWidthMax = 200;
}

void CLyrDisplayClrListWnd::Create(CSkinWnd *pWnd, CRect &rc, bool bHighLight, bool bFloatingLyr, IPopupSkinWndNotify *pNotify)
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

void CLyrDisplayClrListWnd::PopupSkinWndOnSelected()
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

void CLyrDisplayClrListWnd::SelectCurrentSetting()
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

int CLyrDisplayClrListWnd::FindGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2)
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

int CLyrDisplayClrListWnd::FindPattern(LPCTSTR szPatternFile)
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

void CLyrDisplayClrListWnd::AddPureColor(COLORREF clr)
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

void CLyrDisplayClrListWnd::AddGradientColors()
{
	AddGradientColor(RGB(255, 255, 128), RGB(255, 128, 0), RGB(255, 128, 128));
	AddGradientColor(RGB(128, 255, 255), RGB(0, 128, 255), RGB(128, 255, 255));
	// AddGradientColor(RGB(, , ), RGB(, , ), RGB(, , ));
}

void CLyrDisplayClrListWnd::AddGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2)
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

void CLyrDisplayClrListWnd::AddPatternImages()
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

void CLyrDisplayClrListWnd::AddImage(PRAW_IMAGE_DATA hImage)
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
