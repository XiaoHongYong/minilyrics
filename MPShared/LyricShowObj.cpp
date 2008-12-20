key/********************************************************************
	Created  :	2002年1月3日 16:21:56
	FileName :	LyricShowObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowObj.h"
#ifndef _MPLAYER
#include "DlgRateLyrics.h"
#endif

// 每次调整MiniLyrics的速度的基本单位
const int SET_SPEED_SPAN		= 500;


void AlphaBlendColor(MLColor &clr1, MLColor &clr2, int nAlpha, MLColor &clrOut)
{
	ASSERT(nAlpha >= 0 && nAlpha <= 255);
	int		nPart1 = 255 - nAlpha;

	clrOut.set((clr1.r() * nPart1 + clr2.r() * nAlpha) / 255,
		(clr1.g() * nPart1 + clr2.g() * nAlpha) / 255,
		(clr1.b() * nPart1 + clr2.b() * nAlpha) / 255);
}

void GradientFillImage24bpp(PRAW_IMAGE_DATA pImage, CLyricShowObj::MLColorEx clrGradient[3])
{
	ASSERT(pImage->bitCount == 24);
//	agg::rendering_buffer	buff(pImageData->buff, pImageData->width, pImageData->height, pImageData->stride);

	int		r0 = clrGradient[0].r(), g0 = clrGradient[0].g(), b0 = clrGradient[0].b();
	int		r1 = clrGradient[1].r(), g1 = clrGradient[1].g(), b1 = clrGradient[1].b();
	int		r2 = clrGradient[2].r(), g2 = clrGradient[2].g(), b2 = clrGradient[2].b();
	int		nMiddle;
	float	dr, dg, db, rN, gN, bN;
	uint8	*row, *pixl;
	int		x, y;

	nMiddle = pImage->height / 2;

	// fill top
	dr = float(r1 - r0) / nMiddle;
	dg = float(g1 - g0) / nMiddle;
	db = float(b1 - b0) / nMiddle;

	rN = (float)(r0 + 0.5); gN = (float)(g0 + 0.5); bN = (float)(b0 + 0.5);
	row = pImage->RowPtr(0);
	for (y = 0; y < nMiddle; y++)
	{
		pixl = row;
		for (x = 0; x < pImage->width; x++)
		{
			pixl[0] = (uint8)bN;
			pixl[1] = (uint8)gN;
			pixl[2] = (uint8)rN;
			pixl += 3;
		}

		bN += db; gN += dg; rN += dr;
		row += pImage->stride;
	}

	// fill bottom
	dr = float(r2 - r1) / nMiddle;
	dg = float(g2 - g1) / nMiddle;
	db = float(b2 - b1) / nMiddle;

	rN = (float)(r1 + 0.5); gN = (float)(g1 + 0.5); bN = (float)(b1 + 0.5);
	row = pImage->RowPtr(nMiddle);
	for (y = nMiddle; y < pImage->height; y++)
	{
		pixl = row;
		for (x = 0; x < pImage->width; x++)
		{
			pixl[0] = (uint8)bN;
			pixl[1] = (uint8)gN;
			pixl[2] = (uint8)rN;
			pixl += 3;
		}

		bN += db; gN += dg; rN += dr;
		row += pImage->stride;
	}
}

void CreateGradientFillImage24bpp(CRawImage &image, int nHeight, CLyricShowObj::MLColorEx clrGradient[3])
{
	PRAW_IMAGE_DATA		pImage = image.GetHandle();
	image.Detach();
	if (pImage)
	{
		if (pImage->height != nHeight)
		{
			FreeRawImage(pImage);
			pImage = NULL;
		}
	}
	if (!pImage)
	{
		pImage = new RAW_IMAGE_DATA;
		if (pImage && !pImage->Create(nHeight * 2, nHeight, 24))
		{
			delete pImage;
			pImage = NULL;
		}
	}

	if (pImage)
	{
		GradientFillImage24bpp(pImage, clrGradient);
		image.Attach(pImage);
	}
}

void CreateGradientFillImage24bpp(CRawImage &image, int nHeight, MLColor clrSimpGradient)
{
	CLyricShowObj::MLColorEx		clrGradient[3];

#define		PATTERN_COLOR_RANGE			100
	int		r = clrSimpGradient.r(), g = clrSimpGradient.g(), b = clrSimpGradient.b();
	int		r1, g1, b1;

	if (r + b + g >= 128 * 3)
	{
		r1 = r - PATTERN_COLOR_RANGE;
		g1 = g - PATTERN_COLOR_RANGE;
		b1 = b - PATTERN_COLOR_RANGE;
		if (r1 < 0) r1 = 0;
		if (g1 < 0) g1 = 0;
		if (b1 < 0) b1 = 0;
	}
	else
	{
		r1 = r + PATTERN_COLOR_RANGE;
		g1 = g + PATTERN_COLOR_RANGE;
		b1 = b + PATTERN_COLOR_RANGE;
		if (r1 > 255) r1 = 255;
		if (g1 > 255) g1 = 255;
		if (b1 > 255) b1 = 255;
	}

	clrGradient[0].set(r1, g1, b1);
	clrGradient[1] = clrSimpGradient;
	clrGradient[2].set(r1, g1, b1);

	CreateGradientFillImage24bpp(image, nHeight, clrGradient);
}

static ID_TO_STRING g_idsLOB[] = 
{
	DEFINE_IDT(OBM_COLOR),
	DEFINE_IDT(OBM_GRADIENT_COLOR),
	DEFINE_IDT(OBM_AUTO_GRADIENT),
	DEFINE_IDT(OBM_PATTERN),
	{ 0, NULL },
};

struct LyrShowTOBFieldName
{
	LPCTSTR			szOBM;
	LPCTSTR			szFill;
	LPCTSTR			szGradient1, szGradient2, szGradient3;
	LPCTSTR			szAutoGradient;
	LPCTSTR			szPattern;
	LPCTSTR			szBorder;
};

LPCTSTR		_LyrShowTOBFieldNameHilight[] = 
{
	_T("LOBHilightColor"),
	_T("HilightGradient1"), _T("HilightGradient2"), _T("HilightGradient3"),
	_T("HilightAutoGradient"),
	_T("LOBHilightBorder"),
};

LPCTSTR		_LyrShowTOBFieldNameLowlight[] = 
{
	_T("LOBLowlightColor"),
	_T("LowlightGradient1"), _T("LowlightGradient2"), _T("LowlightGradient3"),
	_T("LowlightAutoGradient"),
	_T("LOBLowlightBorder"),
};

void LoadLyrOverlayBlendingSettings(LPCTSTR szSectName, CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight, bool bHilight)
{
	ASSERT(CountOf(_LyrShowTOBFieldNameLowlight) == CLyricShowObj::TCI_COUNT);
	ASSERT(CountOf(_LyrShowTOBFieldNameHilight) == CLyricShowObj::TCI_COUNT);

	LPCTSTR		*pFiledName;

	if (bHilight)
		pFiledName = _LyrShowTOBFieldNameHilight;
	else
		pFiledName = _LyrShowTOBFieldNameLowlight;

	ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_BORDER], szSectName, pFiledName[CLyricShowObj::TCI_BORDER]);

	tob.obm = (OverlayBlendingMode)g_Profile.GetProfileInt(szSectName, bHilight ? _T("LOBHilight") : _T("LOBLowlight"),  OBM_COLOR);
	if (tob.obm == OBM_PATTERN)
	{
		tob.imgPattern.Destroy();

		tob.strPatternFile = g_Profile.GetProfileString(szSectName, bHilight ? _T("HilightPattern") : _T("LowlightPattern"), _T(""));
		if (!tob.strPatternFile.empty())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataFromFile(tob.strPatternFile.c_str());
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				tob.imgPattern.Attach(pImage);
		}
	}
	else if (tob.obm == OBM_GRADIENT_COLOR)
	{
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT1], szSectName, pFiledName[CLyricShowObj::TCI_GRADIENT1]);
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT2], szSectName, pFiledName[CLyricShowObj::TCI_GRADIENT2]);
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT3], szSectName, pFiledName[CLyricShowObj::TCI_GRADIENT3]);

		CreateGradientFillImage24bpp(tob.imgPattern, nGradientPatternHeight, tob.clr + CLyricShowObj::TCI_GRADIENT1);
	}
	else if (tob.obm == OBM_AUTO_GRADIENT)
	{
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_AUTO_GRADIENT], szSectName, pFiledName[CLyricShowObj::TCI_AUTO_GRADIENT]);

		CreateGradientFillImage24bpp(tob.imgPattern, nGradientPatternHeight, tob.clr[CLyricShowObj::TCI_AUTO_GRADIENT]);
	}
	else
	{
		tob.imgPattern.Destroy();
		tob.obm = OBM_COLOR;
	}

	if (!tob.imgPattern.IsValid())
	{
		tob.obm = OBM_COLOR;
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_FILL], szSectName, pFiledName[CLyricShowObj::TCI_FILL]);
	}
}

void CreateLyrOverlayBlendingPattern(CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight)
{
	ASSERT(nGradientPatternHeight > 0 && nGradientPatternHeight < 300);

	if (tob.obm == OBM_PATTERN)
	{
		tob.imgPattern.Destroy();

		if (!tob.strPatternFile.empty())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataFromFile(tob.strPatternFile.c_str());
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				tob.imgPattern.Attach(pImage);
		}
	}
	else if (tob.obm == OBM_GRADIENT_COLOR)
	{
		CreateGradientFillImage24bpp(tob.imgPattern, nGradientPatternHeight, tob.clr + CLyricShowObj::TCI_GRADIENT1);
	}
	else if (tob.obm == OBM_AUTO_GRADIENT)
	{
		CreateGradientFillImage24bpp(tob.imgPattern, nGradientPatternHeight, tob.clr[CLyricShowObj::TCI_AUTO_GRADIENT]);
	}
	else
		tob.imgPattern.Destroy();

	if (!tob.imgPattern.IsValid())
		tob.obm = OBM_COLOR;
}

bool ProfileGetLyricsFont(LPCTSTR szSectName, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers)
{
	tstring		strFont;

#ifdef _WIN32
	strFont = g_Profile.GetProfileString(szSectName, _T("Font"), _T("Verdana, 13, bold, 0, 0, Tahoma"));
#else
	strFont = g_Profile.GetProfileString(szSectName, _T("Font"), _T(", 13, bold, 0, 0, "));
#endif

	return FontPropertyValueFromStr(strFont.c_str(), nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers);

/*	nHeight = g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("FontHeight"), 13);
	nWeight = g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("FontWeight"), FW_BOLD);
	byItalic = g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("FontItalic"), FALSE);
	if (szFaceNameLatin9)
		g_Profile.GetProfileString(SZ_SECT_LYR_DISPLAY, _T("FontNameLatin"), _T("Verdana"), szFaceNameLatin9, nFaceNameSize);
	if (szFaceNameOthers)
		g_Profile.GetProfileString(SZ_SECT_LYR_DISPLAY, _T("FontNameOthers"), _T("Tahoma"), szFaceNameOthers, nFaceNameSize);*/
}

void ProfileWriteLyricsFont(EventType etColorTheme, LPCTSTR szSectName, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers)
{
	tstring		strFont;
	tstring		strLatin9, strOthers;

	if (szFaceNameLatin9 == NULL || szFaceNameOthers == NULL || IsEmptyString(szFaceNameLatin9) || IsEmptyString(szFaceNameOthers))
	{
		if ((szFaceNameLatin9 == NULL || IsEmptyString(szFaceNameLatin9)) && (szFaceNameOthers == NULL || IsEmptyString(szFaceNameOthers))
			&& nHeight == -1 && nWeight == -1 && byItalic == 0xFF)
			return;

		int		nHeightOld, nWeightOld;
		BYTE	nItalicOld;

		ProfileGetLyricsFont(szSectName, nHeightOld, nWeightOld, nItalicOld, strLatin9, strOthers);
		if (!szFaceNameLatin9 || IsEmptyString(szFaceNameLatin9))
			szFaceNameLatin9 = strLatin9.c_str();
		if (!szFaceNameOthers || IsEmptyString(szFaceNameOthers))
			szFaceNameOthers = strOthers.c_str();
		if (nHeight == -1)
			nHeight = nHeightOld;
		if (nWeight == -1)
			nWeight = nWeightOld;
		if (byItalic == 0xFF)
			byItalic = nItalicOld;
	}

	FontPropertyValueToStr(strFont, nHeight, nWeight, byItalic, szFaceNameLatin9, szFaceNameOthers);

	CMPlayerSettings::SetSettings(etColorTheme, szSectName, _T("Font"), strFont.c_str());
}

void ProfileWriteTOBToOld(bool bTobEnabled, 
	 const CLyricShowObj::TextOverlayBlending &tobHilight, 
	 const CLyricShowObj::TextOverlayBlending &tobLowlight)
{
	tstring					str;
	static const LPCTSTR	SZ_SECT_TOB_OLD = _T("TOBOld");

	for (int i = CLyricShowObj::TCI_START; i < CLyricShowObj::TCI_COUNT; i++)
	{
		ColorToStr((MLColor)tobHilight.clr[i], str);
		g_Profile.WriteProfileString(SZ_SECT_TOB_OLD, _LyrShowTOBFieldNameHilight[i], str.c_str());
		ColorToStr((MLColor)tobLowlight.clr[i], str);
		g_Profile.WriteProfileString(SZ_SECT_TOB_OLD, _LyrShowTOBFieldNameLowlight[i], str.c_str());
	}

	g_Profile.WriteProfileInt(SZ_SECT_TOB_OLD, _T("BlendingLyrText"), bTobEnabled);

	g_Profile.WriteProfileInt(SZ_SECT_TOB_OLD, _T("LOBHilight"), tobHilight.obm);
	g_Profile.WriteProfileInt(SZ_SECT_TOB_OLD, _T("LOBLowlight"), tobLowlight.obm);
}

void ProfileWriteTOBCustomizedToCurrent()
{
	static const LPCTSTR	SZ_SECT_TOB_OLD = _T("TOBOld");
	tstring		strValue;

	for (int i = CLyricShowObj::TCI_START; i < CLyricShowObj::TCI_COUNT; i++)
	{
		strValue = g_Profile.GetProfileString(SZ_SECT_TOB_OLD, _LyrShowTOBFieldNameHilight[i], _T(""));
		if (!strValue.empty())
			g_Profile.WriteProfileString(SZ_SECT_LYR_DISPLAY, _LyrShowTOBFieldNameHilight[i], strValue.c_str());

		strValue = g_Profile.GetProfileString(SZ_SECT_TOB_OLD, _LyrShowTOBFieldNameLowlight[i], _T(""));
		if (!strValue.empty())
			g_Profile.WriteProfileString(SZ_SECT_LYR_DISPLAY, _LyrShowTOBFieldNameLowlight[i], strValue.c_str());
	}

	strValue = g_Profile.GetProfileString(SZ_SECT_TOB_OLD, _T("BlendingLyrText"), _T(""));
	if (!strValue.empty())
	{
		g_Profile.WriteProfileInt(SZ_SECT_LYR_DISPLAY, _T("BlendingLyrText"), _ttoi(strValue.c_str()));

		strValue = g_Profile.GetProfileString(SZ_SECT_TOB_OLD, _T("LOBHilight"), _T(""));
		if (!strValue.empty())
			g_Profile.WriteProfileString(SZ_SECT_LYR_DISPLAY, _T("LOBHilight"), strValue.c_str());

		strValue = g_Profile.GetProfileString(SZ_SECT_TOB_OLD, _T("LOBLowlight"), _T(""));
		if (!strValue.empty())
			g_Profile.WriteProfileString(SZ_SECT_LYR_DISPLAY, _T("LOBLowlight"), strValue.c_str());
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowObj, _T("LyricShow"))

CLyricShowObj::CLyricShowObj()
{
	m_nXMargin = 5;
	m_nYMargin = 2;

	m_pMLData = NULL;

	m_bCanWrapLines = false;

	m_bEnableAutoResize = true;
	m_bAlwaysAutoWidth = false;

	m_bUseSkinStyle = false;

	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

	m_etDispSettings = ET_LYRICS_DISPLAY_SETTINGS;
	m_strSectName = SZ_SECT_LYR_DISPLAY;

	m_font.Create(_T(""), _T(""), 13, FW_NORMAL, 0, false);
	m_font.SetThickOutline(false);

	m_nFontHeight = m_font.GetHeight();


	m_nAlignment = LA_CENTER;

	m_LyricsDisplayOpt = DO_FADEOUT_BG;

	m_hue = 0.0;

	m_clrBg.set(RGB(79, 88, 110));
	m_clrFg.set(RGB(236, 236, 236));
	m_clrFgLow.set(RGB(131, 148, 158));
	m_clrTag.set(RGB(255, 128, 64));
	m_clrFocusLineBg.set(RGB(64, 71, 88));

	m_tobHilight.obm = OBM_COLOR;
	m_tobHilight.clr[TCI_FILL].set(RGB(255, 255, 255));
	m_tobHilight.clr[TCI_BORDER].set(RGB(255, 0, 0));

	m_tobLowlight.obm = OBM_COLOR;
	m_tobLowlight.clr[TCI_FILL].set(RGB(255, 255, 255));
	m_tobLowlight.clr[TCI_BORDER].set(RGB(0, 0, 255));

	m_skin_bTobEnabled = false;

	m_nLineSpacing = 2;

	m_bTobEnabled = false;
	m_bKaraoke = false;
	m_lyrShadowMode = CRawBmpFont::SM_SHADOW;

	m_bUseAlbumArtAsBg = false;
	m_bUseBgImg = false;
	m_bTobEnabledOnBgPic = true;

	m_bDarkenLyrBgOnImg = true;
	m_nDarkenTopArea = float(1.25);
	m_nDarkenBottomArea = float(1.25);
	m_clrDarken.set(RGB(0x18, 0x3C, 0x7B));
	m_clrDarken.setAlpha(128);

	m_bgImgPos = BIP_CENTER;

	m_nNextPic = 0;

	m_nNextPicInFolder = 0;

#ifndef _WIN32_WCE
	m_Cursor.LoadCursorFromRes(IDC_MLHAND);

	m_curAdjustVertAlign.LoadCursorFromRes(IDC_MLHAND);
	m_yBeginDrag = -1;
	m_bEnableAdjustVertAlign = true;
	m_bEnableAdjustVertAlignUser = g_Profile.GetProfileBool(SZ_SECT_LYR_DISPLAY, _T("EnableAdjustVertAlign"), false);
	m_vertLineAlign = VLA_CENTER;
	m_nVertLineOffset = 0;
#endif

#ifdef _WIN32_DESKTOP
	m_bThickOutline = false;
	m_bFadeOutBorder = false;
#endif

	m_etDispSettings = ET_LYRICS_DISPLAY_SETTINGS;
	m_strSectName = SZ_SECT_LYR_DISPLAY;
}

CLyricShowObj::~CLyricShowObj()
{
	g_eventDispather.UnregisterHanlder(m_etDispSettings, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
	g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	g_Profile.WriteProfileInt(_T("NextPicInFolder"), m_nNextPicInFolder);
	m_pSkin->UnregisterTimerObject(this);
}

void CLyricShowObj::OnCreate()
{
	m_pMLData = &g_LyricData;

	// ReloadThemeOfSkin(false);

	LoadAllSettings();

	if (m_bUseSkinStyle)
	{
		int		i;

		for (i = TCI_START; i < TCI_COUNT; i++)
		{
			m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_tobLowlight.clr[TOBColorIndex(i)]);
			m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_tobHilight.clr[TOBColorIndex(i)]);
		}

		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrBg);
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrFg);
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrFgLow);
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrTag);
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrFocusLineBg);
	}

	g_eventDispather.RegisterHanlder(m_etDispSettings, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_CHANGED, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
	g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	if (m_bUseBgImg)
		m_pSkin->RegisterTimerObject(this, g_Profile.GetProfileInt(_T("SlideDelayTime"), 10) * 1000);

	m_pContainer->RecalculateUIObjSizePos(this);

	tstring			strColor;

	ColorToStr(m_clrBg, strColor);
	m_pSkin->SetProperty(_T("BgColor"), strColor.c_str());

	OnLyricsChanged();
}

void CLyricShowObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == m_etDispSettings)
	{
		OnLyrDisplaySettings(pEvent->strName.c_str(), pEvent->strValue.c_str());
	}
	else if (pEvent->eventType == ET_LYRICS_CHANGED)
	{
		OnLyricsChanged();
	}
	else if (pEvent->eventType == ET_LYRICS_DRAW_UPDATE)
	{
		OnPlayTimeChangedUpdate();
	}
	else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
	{
		UpdateBgImage();
		Invalidate();
	}
}

// COMMENT:
//		从右向左滚动歌词显示
void CLyricShowObj::Draw(CRawGraph *pGraph)
{
#ifndef _WIN32_WCE
	m_vads.clear();
#endif
	m_font.EnableGdiplusAntialias(CMLGraphicsFactory::GetGraphicsType() == MGT_GDIPLUS);

	pGraph->SetFont(&m_font);

	FastDraw(pGraph, NULL);
}

// 快速绘画
// OUTPUT:
//		rcUpdate	-	更新的矩形区域
void CLyricShowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	if (prcUpdate)
	{
		prcUpdate->left = prcUpdate->right = m_rcObj.left;
		prcUpdate->top = prcUpdate->bottom = m_rcObj.top;
	}
}

void CLyricShowObj::SetLyricData(CMLData *pMLData)
{
	m_pMLData = pMLData;
}

// Draw lyrics line according to current display style and linePos
//
// Set line color according to linePos and display style
//
// 1). Fadeout to low light color
//		a. LP_ABOVE_CUR_LINE :	higlight -> low light
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	
//			(i) karaok : Low light
//			(ii)other  : lowlight -> high light
// 2). Fadeout to background
//		a. LP_ABOVE_CUR_LINE :	higlight -> bg
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	
//			(i) karaok : bg -> Low light
//			(ii)other  : bg -> high light
// 2). Normal
//		a. LP_ABOVE_CUR_LINE :	lowlight
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	lowlight

enum	LYR_COLOR_TRANSFORM
{
	LCT_HIGH_TO_LOW,
	LCT_HIGH_TO_BG,
	LCT_BG_TO_LOW,
	LCT_BG_TO_HIGH,
	LCT_LOW_TO_HIGH,
	LCT_HIGH,
	LCT_LOW,
};

bool CLyricShowObj::DrawRow(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y, LinePos linePos)
{
	int			nSize;
	bool		bRet = true;
	MLColor		clrTxt, clrTxtBorder;
	int			nAlpha = 255, alphaOld;
	bool		bSetTextAlpha = false;
	LYR_COLOR_TRANSFORM		lct;

	if (pLyricRow == NULL)
		return false;

// 1). Fadeout to low light color
//		a. LP_ABOVE_CUR_LINE :	higlight -> low light
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	
//			(i) karaok : Low light
//			(ii)other  : lowlight -> high light
	if (m_LyricsDisplayOpt == DO_FADEOUT_LOWCOLOR)
	{
		if (linePos == LP_ABOVE_CUR_LINE)
		{
			lct = LCT_HIGH_TO_LOW;
			nAlpha = GetAlpha(pLyricRow);
			if (nAlpha == 255)
				lct = LCT_LOW;
			else if (nAlpha == 0)
				lct = LCT_LOW;
		}
		else if (linePos == LP_BELOW_CUR_LINE)
		{
			if (IsKaraoke())
				lct = LCT_LOW;
			else
			{
				lct = LCT_LOW_TO_HIGH;
				nAlpha = 255 - GetAlpha(pLyricRow);
				if (nAlpha == 255)
					lct = LCT_LOW;
				else if (nAlpha == 0)
					lct = LCT_LOW;
			}
		}
		else
			lct = LCT_HIGH;
	}
// 2). Fadeout to background
//		a. LP_ABOVE_CUR_LINE :	higlight -> bg
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	
//			(i) karaok : bg -> Low light
//			(ii)other  : bg -> high light
	else if (m_LyricsDisplayOpt == DO_FADEOUT_BG)
	{
		if (linePos == LP_ABOVE_CUR_LINE)
		{
			lct = LCT_HIGH_TO_BG;
			nAlpha = 255 - GetAlpha(pLyricRow);
			if (nAlpha == 0)
				return false;
			if (m_pSkin->GetEnableTranslucencyLayered() || m_img.IsValid()
					|| m_tobHilight.obm != OBM_COLOR)
			{
				bSetTextAlpha = true;
				lct = LCT_HIGH;
				nAlpha = 255 - nAlpha;
			}
		}
		else if (linePos == LP_BELOW_CUR_LINE)
		{
			if (IsKaraoke())
				lct = LCT_BG_TO_LOW;
			else
				lct = LCT_BG_TO_HIGH;
			nAlpha = GetAlpha(pLyricRow);
			if (nAlpha == 0)
				return false;
			if (m_pSkin->GetEnableTranslucencyLayered() || m_img.IsValid()
					|| m_tobHilight.obm != OBM_COLOR)
			{
				bSetTextAlpha = true;
				if (lct == LCT_BG_TO_HIGH)
					lct = LCT_HIGH;
				else
					lct = LCT_LOW;
			}
		}
		else
			lct = LCT_HIGH;
	}
// 2). Normal
//		a. LP_ABOVE_CUR_LINE :	lowlight
//		b. LP_CUR_LINE :		hilight
//		c. LP_BELOW_CUR_LINE :	lowlight
	else
	{
		if (linePos == LP_ABOVE_CUR_LINE)
		{
			if (IsKaraoke())
				lct = LCT_HIGH;
			else
				lct = LCT_LOW;
		}
		else if (linePos == LP_BELOW_CUR_LINE)
			lct = LCT_LOW;
		else
			lct = LCT_HIGH;
	}


	switch (lct)
	{
	case LCT_LOW_TO_HIGH:
		nAlpha = 255 - nAlpha;
	case LCT_HIGH_TO_LOW:
		{
			AlphaBlendColor(m_tobLowlight.clr[TCI_BORDER], m_tobHilight.clr[TCI_BORDER], nAlpha, clrTxtBorder);
			if (m_tobHilight.obm == OBM_COLOR)
			{
				if (m_tobLowlight.obm == OBM_COLOR)
				{
					m_font.UseColorOverlay();
					AlphaBlendColor(GetLowlightColor(), GetHighlightColor(), nAlpha, clrTxt);
				}
				else
				{
					m_font.SetOverlayPattern(&m_tobLowlight.imgPattern, 255 - nAlpha, m_tobHilight.clr[TCI_FILL], nAlpha);
				}
			}
			else
			{
				// hilight is image
				if (m_tobLowlight.obm == OBM_COLOR)
				{
					m_font.SetOverlayPattern(&m_tobHilight.imgPattern, nAlpha, m_tobLowlight.clr[TCI_FILL], 255 - nAlpha);
				}
				else
				{
					m_font.SetOverlayPattern(&m_tobHilight.imgPattern, &m_tobLowlight.imgPattern, nAlpha, 255 - nAlpha);
				}
			}
		}
		break;
	case LCT_BG_TO_HIGH:
		nAlpha = 255 - nAlpha;
	case LCT_HIGH_TO_BG:
		{
			AlphaBlendColor(m_tobHilight.clr[TCI_BORDER], m_clrBg, nAlpha, clrTxtBorder);
			if (m_tobHilight.obm == OBM_COLOR)
			{
				m_font.UseColorOverlay();
				AlphaBlendColor(GetHighlightColor(), m_clrBg, nAlpha, clrTxt);
			}
			else
			{
				m_font.SetOverlayPattern(&m_tobHilight.imgPattern, nAlpha, m_clrBg, 255 - nAlpha);
			}
		}
		break;
	case LCT_BG_TO_LOW:
		{
			AlphaBlendColor(m_clrBg, m_tobLowlight.clr[TCI_BORDER], nAlpha, clrTxtBorder);
			if (m_tobLowlight.obm == OBM_COLOR)
			{
				m_font.UseColorOverlay();
				AlphaBlendColor(m_clrBg, GetLowlightColor(), nAlpha, clrTxt);
			}
			else
			{
				m_font.SetOverlayPattern(&m_tobLowlight.imgPattern, nAlpha, m_clrBg, 255 - nAlpha);
			}
		}
		break;
	case LCT_HIGH:
		{
			clrTxtBorder = m_tobHilight.clr[TCI_BORDER];
			if (m_tobHilight.obm == OBM_COLOR)
			{
				m_font.UseColorOverlay();
				clrTxt = GetHighlightColor();
			}
			else
			{
				m_font.SetOverlayPattern(&m_tobHilight.imgPattern);
			}
		}
		break;
	case LCT_LOW:
		{
			clrTxtBorder = m_tobLowlight.clr[TCI_BORDER];
			if (m_tobLowlight.obm == OBM_COLOR)
			{
				m_font.UseColorOverlay();
				clrTxt = GetLowlightColor();
			}
			else
			{
				m_font.SetOverlayPattern(&m_tobLowlight.imgPattern);
			}
		}
		break;
	}


	if (x == AUTO_CAL_X)
		x = GetLyricRowAlignPos(pGraphics, pLyricRow);

	// 画出此行歌词
	nSize = pLyricRow->vFrags.size();
	for (int i = 0; i < nSize; i++)
	{
		PLYRIC_FRAG		pFrag;

		pFrag = pLyricRow->vFrags[i];

/*#ifndef _WIN32_WCE
		if (pFrag->IsAdFrag())
		{
			CMLFont		*pOldFont;

			pOldFont = pGraphics->GetFont();
			ASSERT(pOldFont);
			if (pOldFont)
			{
				CMLFont		font;
				font.Create(m_font.GetName(), font.GetNameOthers(), m_font.GetHeight(), m_font.GetWeight(), m_font.GetItalic(), true);

				pGraphics->SetFont(&font);

				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, pFrag->GetDrawWidth(pGraphics), GetFontHeight());

				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
				x += pFrag->GetDrawWidth(pGraphics);

				pGraphics->SetFont(pOldFont);
			}
		}
		else
#endif*/
		{
			if (bSetTextAlpha)
			{
				alphaOld = pGraphics->GetTextAlpha();
				pGraphics->SetTextAlpha(nAlpha);
			}

			if (IsBlendingLyrText())
			{
				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y, pFrag->szLyric, pFrag->nLen, clrTxt, clrTxtBorder);
			}
			else
			{
				pGraphics->SetTextColor(clrTxt);

				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}

			if (bSetTextAlpha)
				pGraphics->SetTextAlpha(alphaOld);

#ifndef _WIN32_WCE
			if (pFrag->IsAdFrag())
			{
				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, pFrag->GetDrawWidth(pGraphics), GetFontHeight());
			}
#endif

			x += pFrag->GetDrawWidth(pGraphics);
		}
	}

	return true;
}

void CLyricShowObj::DrawRow(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y, MLColor &clrTxt, MLColor &clrTxtBorder)
{
	int			nSize;

	if (x == AUTO_CAL_X)
		x = GetLyricRowAlignPos(pGraphics, pLyricRow);

	// 画出此行歌词
	nSize = pLyricRow->vFrags.size();
	for (int i = 0; i < nSize; i++)
	{
		PLYRIC_FRAG		pFrag;

		pFrag = pLyricRow->vFrags[i];

/*#ifndef _WIN32_WCE
		if (pFrag->IsAdFrag())
		{
			CMLFont		*pOldFont;

			pOldFont = pGraphics->GetFont();
			ASSERT(pOldFont);
			if (pOldFont)
			{
				CMLFont		font;
				font.Create(m_font.GetName(), font.GetNameOthers(), m_font.GetHeight(), m_font.GetWeight(), m_font.GetItalic(), true);

				pGraphics->SetFont(&font);

				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, pFrag->GetDrawWidth(pGraphics), GetFontHeight());

				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
				x += pFrag->GetDrawWidth(pGraphics);

				pGraphics->SetFont(pOldFont);
			}
		}
		else
#endif*/
		{
			if (IsBlendingLyrText())
			{
				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y, pFrag->szLyric, pFrag->nLen, clrTxt, clrTxtBorder);
			}
			else
			{
				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}
#ifndef _WIN32_WCE
			if (pFrag->IsAdFrag())
			{
				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, pFrag->GetDrawWidth(pGraphics), GetFontHeight());
			}
#endif
			x += pFrag->GetDrawWidth(pGraphics);
		}
	}
}

// COMMENT:
//		逐字显示歌词, like Karaoke
void CLyricShowObj::DrawRowKaraoke(CRawGraph	*pGraphics, PML_ROW pLyricRow, int x, int y)
{
	assert(m_pMLData);
	if (pLyricRow == NULL)
		return;

	// exp. "Said I love you but I lie"
	//               p^
	// 1. Get pCurWord Position
	// 2. Draw all words before pCurWord.
	// 3. Draw word pCurWord
	// 4. draw words after pCurWords
	// 5. ok
	PLYRIC_FRAG	pFrag;
	int			nSize;
	int			nCurPos;

	if (pLyricRow->IsAdRow())
	{
		DrawRow(pGraphics, pLyricRow, x, y, LP_CUR_LINE);
		return;
	}

	nSize = pLyricRow->vFrags.size();

	// 取得该行歌词显示的X座标
	if (x == AUTO_CAL_X)
		x = GetLyricRowAlignPos(pGraphics, pLyricRow);

	// 先将在时间已过的歌词段显示出来
	for (nCurPos = 0; nCurPos < nSize; nCurPos++)
	{
		pFrag = pLyricRow->vFrags[nCurPos];

		if (pFrag->nEndTime >= m_pMLData->GetPlayElapsedTime())
		{
			// 时间未过的歌词段
			break;
		}
		else
		{
			// 时间已过的歌词段
			//
			// Draw lyrics with hilight
			//
			if (IsBlendingLyrText())
			{
				if (m_tobHilight.obm == OBM_COLOR)
					m_font.UseColorOverlay();
				else
					m_font.SetOverlayPattern(&m_tobHilight.imgPattern);

				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y, pFrag->szLyric, pFrag->nLen, m_tobHilight.clr[TCI_FILL], m_tobHilight.clr[TCI_BORDER]);
			}
			else
			{
				pGraphics->SetTextColor(m_clrFg);

				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}
			x += pFrag->GetDrawWidth(pGraphics);;
		}
	}

	// 显示当前时间段的歌词
	if (nCurPos < nSize)
	{
		//    I LOVE YOU		// 1. draw
		//    I lov				// 2. draw ok
		//    I lovE YOU        // 3. result
		int		nFragDrawWidth;
		int		nKaraokePos = 0;

		pFrag = pLyricRow->vFrags[nCurPos];

		// get width of the lyrics fragment
		nFragDrawWidth = pFrag->GetDrawWidth(pGraphics);

		if (pFrag->nBegTime < m_pMLData->GetPlayElapsedTime())
		{
			// 如果时间 m_pMLData->GetPlayElapsedTime() 在当前歌词段之间
			//    |1word2|   |1word2|
			//                  ^ pFrag
			int dt = (pFrag->nEndTime - pFrag->nBegTime);
			if (dt != 0)
			{

				nKaraokePos = nFragDrawWidth * (m_pMLData->GetPlayElapsedTime() - pFrag->nBegTime) / dt;
				// rcText.SetRect(x, y, x + nKaraokePos + 1, y + GetFontHeight() + 1);
			}
		}
		else
		{
			//    |1word2|   |1word2|
			//             ^  pFrag
			nKaraokePos = nFragDrawWidth;
		}

		CRect	rcText;

		// Draw lowlight part lyrics first
		if (nKaraokePos != nFragDrawWidth)
		{
			rcText.SetRect(x, y, x + nFragDrawWidth + 1000, y + GetFontHeight());

			if (IsBlendingLyrText())
			{
				if (m_tobLowlight.obm == OBM_COLOR)
					m_font.UseColorOverlay();
				else
					m_font.SetOverlayPattern(&m_tobLowlight.imgPattern);

				pGraphics->DrawTextClipOutlined(pFrag->szLyric, pFrag->nLen, &rcText, m_tobLowlight.clr[TCI_FILL], m_tobLowlight.clr[TCI_BORDER], nKaraokePos);
			}
			else
			{
				pGraphics->SetTextColor(m_clrFgLow);
				pGraphics->DrawTextClip(pFrag->szLyric, pFrag->nLen, &rcText, nKaraokePos);
			}
		}

		// Draw hilight part lyrics
		rcText.SetRect(x, y, x + nKaraokePos, y + GetFontHeight());
		if (IsBlendingLyrText())
		{
			if (m_tobHilight.obm == OBM_COLOR)
				m_font.UseColorOverlay();
			else
				m_font.SetOverlayPattern(&m_tobHilight.imgPattern);

			// Draw lyrics text outlined
			pGraphics->DrawTextClipOutlined(pFrag->szLyric, pFrag->nLen, &rcText, m_tobHilight.clr[TCI_FILL], m_tobHilight.clr[TCI_BORDER]);
		}
		else
		{
			pGraphics->SetTextColor(m_clrFg);
			pGraphics->DrawTextClip(pFrag->szLyric, pFrag->nLen, &rcText);
		}

		// 增加 x 的偏移位置
		x += nFragDrawWidth;
		nCurPos++;
	}

	// 显示时间未到的歌词段
	for (; nCurPos < nSize; nCurPos++)
	{
		pFrag = pLyricRow->vFrags[nCurPos];

		// draw text border
		if (IsBlendingLyrText())
		{
			// Draw lyrics text outlined
			pGraphics->TextOutOutlined(x, y, pFrag->szLyric, pFrag->nLen, m_tobLowlight.clr[TCI_FILL], m_tobLowlight.clr[TCI_BORDER]);
		}
		else
		{
			pGraphics->SetTextColor(m_clrFgLow);
			pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
		}
		x += pFrag->GetDrawWidth(pGraphics);
	}
	// 5. ok
}

void CLyricShowObj::DrawCurrentRow(CRawGraph	*pGraphics, PML_ROW pLyricRow, int x, int y)
{
	assert(m_pMLData);
	assert(pGraphics);

	if (IsKaraoke())
		DrawRowKaraoke(pGraphics, pLyricRow, x, y);
	else
		DrawRow(pGraphics, pLyricRow, x, y, LP_CUR_LINE);
}

// COMMENT:
//		根据当前的 Graphics 环境，取得歌词pLyricRow行的宽度
int CLyricShowObj::GetLyricRowTextWidth(CRawGraph	*pGraphics, PML_ROW pLyricRow)
{
	assert(m_pMLData);

	int			nTxtWidth = 0;
	int			nSize;
	PLYRIC_FRAG	pFrag;

	nSize = pLyricRow->vFrags.size();
	for (int i = 0; i < nSize; i++)
	{
		pFrag = pLyricRow->vFrags[i];

		nTxtWidth += pFrag->GetDrawWidth(pGraphics);
	}
	
	return nTxtWidth;
}

void CLyricShowObj::UpdateLyricDrawBufferBackground(CRawGraph *pGraphics, CRect &rc)
{
	if (m_img.IsValid())
	{
		m_img.TileBltEx(pGraphics, m_rcObj.left, m_rcObj.top, rc.left, rc.top, rc.Width(), rc.Height());

		if (m_bDarkenLyrBgOnImg)
			DarkenLyricsBg(pGraphics, rc);
	}
	else
	{
		m_pSkin->RedrawBackground(&rc);
		// pGraphics->FillRect(rc.left, rc.top, rc.Width(), rc.Height(), &m_brBg);
	}

/*#ifndef _WIN32_WCE
	if (m_bHasAd)
	{
		if (rc.PtInRect(CPoint(m_rcAdWords.left, m_rcAdWords.top))
			|| rc.PtInRect(CPoint(m_rcAdWords.right, m_rcAdWords.top))
			|| rc.PtInRect(CPoint(m_rcAdWords.left, m_rcAdWords.bottom))
			|| rc.PtInRect(CPoint(m_rcAdWords.right, m_rcAdWords.bottom)))
			m_bHasAd = false;
	}
#endif*/
}

void CLyricShowObj::DarkenLyricsBg(CRawGraph *pGraphics, CRect &rc)
{
	int			nFillTop, nFillBottom;
	int			nLineHeight = GetLineHeight();

	int			nDarkenTop, nDarkenBottom;
	nDarkenTop = GetLineVertAlignPos() - int(nLineHeight * m_nDarkenTopArea);
	nDarkenBottom = GetLineVertAlignPos() + int(nLineHeight * m_nDarkenBottomArea);
	nFillBottom = min(rc.bottom, (long)nDarkenBottom);
	nFillTop = max(rc.top, (long)nDarkenTop);
	if (nFillBottom > nFillTop)
		pGraphics->DrawRect(rc.left, nFillTop, rc.Width(), nFillBottom - nFillTop, m_clrDarken);
}

void CLyricShowObj::SetAntialise(BOOL bAntialis)
{
	OnLyrDrawContextChanged();
}

// COMMENT:
//		取得该行歌词显示的X座标
// INPUT:
//		pGraphics		-	显示歌词行的设备
//		pLyricRow -	歌词行
// OUTPUT:
//		输出此行歌词显示的 X 座标位置
int CLyricShowObj::GetLyricRowAlignPos(CRawGraph	*pGraphics, PML_ROW pLyricRow)
{
	assert(m_pMLData);

	int		x;
	switch (m_nAlignment)
	{
	case LA_LEFT:
		// 居左
		// 计算左边显示的位置
		x = m_rcObj.left + m_nXMargin;
		break;
	case LA_CENTER:
	default:
		// 居中
		{
			int 		nTxtWidth;
			// 取得此行歌词的显示宽度
			nTxtWidth = GetLyricRowTextWidth(pGraphics, pLyricRow);
		
			// 计算左边显示的位置
			x = m_rcObj.left + (m_rcObj.Width() - nTxtWidth) / 2;
		}
		break;
	case LA_RIGHT:
		// 居右
		{
			int 		nTxtWidth;
			// 取得此行歌词的显示宽度
			nTxtWidth = GetLyricRowTextWidth(pGraphics, pLyricRow);
			
			// 计算左边显示的位置
			x = m_rcObj.right - nTxtWidth - m_nXMargin;
		}
		break;
	}
	return x;
}

void CLyricShowObj::OnKeyDown(UINT nChar, UINT nFlags)
{
	assert(m_pMLData);

	switch (nChar)
	{
	case VK_UP:
		m_pMLData->SetOffsetTime(m_pMLData->GetOffsetTime() - SET_SPEED_SPAN);
		g_eventDispather.DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
		break;
	case VK_DOWN:
		m_pMLData->SetOffsetTime(m_pMLData->GetOffsetTime() + SET_SPEED_SPAN);
		g_eventDispather.DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
		break;
	case 'Z':
		g_Player.Prev();
		break;
	case 'X':
		g_Player.Play();
		break;
	case 'C':
		g_Player.PlayPause();
		break;
	case 'V':
		g_Player.Stop();
		break;
	case 'B':
		g_Player.Next();
		break;
	case VK_RIGHT:
		// 前进5秒
		{
			DWORD		dwCurPos;
			DWORD		dwLength;

			dwCurPos = g_Player.GetPlayPos();
			dwCurPos += 5000;
			dwLength = g_Player.GetMediaLength();
			if (dwCurPos > dwLength)
				dwCurPos = dwLength;
			g_Player.SeekTo(dwCurPos);
		}
		break;
	case VK_LEFT:
		// 后退5秒
		{
			int			dwCurPos;

			dwCurPos = g_Player.GetPlayPos();
			dwCurPos -= 5000;
			if (dwCurPos < 0)
				dwCurPos = 0;
			g_Player.SeekTo(dwCurPos);
		}
		break;
	default:
		break;
	}
}

#ifndef _WIN32_WCE
bool CLyricShowObj::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_vads.size())
	{
		V_ADS::iterator	it, itEnd;
		itEnd = m_vads.end();
		for (it = m_vads.begin(); it != itEnd; ++it)
		{
			AdSentence	&ads = (*it);
			if (ads.rcAdWords.PtInRect(point))
			{
				// SetCursor
				MLWidget::SetCursor(m_Cursor);

				return true;
			}
		}
	}

	if (m_yBeginDrag != -1)
	{
		int		nPosStart;
		int		nHeight = m_rcObj.Height();

		if (point.y < m_rcObj.top + nHeight / 4)
			m_vertLineAlign = VLA_TOP;
		else if (point.y > m_rcObj.bottom - nHeight / 4)
			m_vertLineAlign = VLA_BOTTOM;
		else
			m_vertLineAlign = VLA_CENTER;

		if (m_vertLineAlign == VLA_TOP)
			nPosStart = m_rcObj.top;
		else if (m_vertLineAlign == VLA_BOTTOM)
			nPosStart = m_rcObj.bottom;
		else
			nPosStart = m_rcObj.top + nHeight / 2;

		m_nVertLineOffset = point.y - nPosStart;
		// ASSERT(GetLineVertAlignPos() == point.y);		

		MLWidget::SetCursor(m_curAdjustVertAlign);

		Invalidate();
		return true;
	}
	else if (m_bEnableAdjustVertAlign && m_bEnableAdjustVertAlignUser)
	{
		int			nPos;

		nPos = GetLineVertAlignPos();
		if (point.y >= nPos - m_nFontHeight && point.y <= nPos + m_nFontHeight
			 && m_rcObj.Height() >= m_nFontHeight * 3)
		{
			MLWidget::SetCursor(m_curAdjustVertAlign);
			return true;
		}
	}

	return false;
}

bool CLyricShowObj::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_yBeginDrag != -1)
	{
		m_yBeginDrag = -1;
		MLWidget::SetCursor(m_curAdjustVertAlign);
		m_pSkin->ReleaseCaptureMouse(this);

		g_Profile.WriteProfileInt(_T("VertAlign"), m_vertLineAlign);
		g_Profile.WriteProfileInt(_T("VertAlignOffset"), m_nVertLineOffset);
		return true;
	}

	if (m_vads.size())
	{
		V_ADS::iterator	it, itEnd;
		itEnd = m_vads.end();
		for (it = m_vads.begin(); it != itEnd; ++it)
		{
			AdSentence	&ads = (*it);
			if (ads.rcAdWords.PtInRect(point))
			{
				// SetCursor
				MLWidget::SetCursor(m_Cursor);

				return true;
			}
		}
	}

	return false;
}

bool CLyricShowObj::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_vads.size())
	{
		V_ADS::iterator	it, itEnd;
		itEnd = m_vads.end();
		for (it = m_vads.begin(); it != itEnd; ++it)
		{
			AdSentence	&ads = (*it);
			if (ads.rcAdWords.PtInRect(point))
			{
				// open url
				if (_tcsncmp(_T("cmd://rate/"), ads.strLink.c_str(), _tcslen(_T("cmd://rate/"))) == 0)
				{
#ifndef _MPLAYER
					// show rate dialog
					CDlgRateLyrics	dlg;
					dlg.m_strLink = ads.strLink.c_str() + _tcslen(_T("cmd://rate/"));
					dlg.DoModal(m_pSkin);
#endif
				}
				else
				{
					OpenUrl(m_pSkin, ads.strLink.c_str());
				}
				// SetCursor
				MLWidget::SetCursor(m_Cursor);
				return true;
			}
		}
	}

	if (m_bEnableAdjustVertAlign && m_bEnableAdjustVertAlignUser)
	{
		int			nPos;

		nPos = GetLineVertAlignPos();
		if (point.y >= nPos - m_nFontHeight && point.y <= nPos + m_nFontHeight
			&& m_rcObj.Height() >= m_nFontHeight * 3)
		{
			MLWidget::SetCursor(m_curAdjustVertAlign);
			m_pSkin->SetCaptureMouse(this);
			m_yBeginDrag = point.y;
			return true;
		}
	}

	return false;
}
#endif

void CLyricShowObj::OnLyricsChanged()
{
/*
#ifndef _UNICODE
	tstring		str;

	//
	// 检查歌词的字符编码是否为ANSI？
	//

	g_LyricData.ToString(str, FT_LYRICS_TXT, TRUE);

	//
	// 非unicode模式需要根据歌词字符编码创建字体
	//
	if (_tcscmp(m_strEncoding.c_str(), g_LyricData.GetEncoding()) != 0)
	{
		m_strEncoding = g_LyricData.GetEncoding();

		// 编码改变了，重新创建字体
		ENCODING_CODEPAGE		&encdoing = GetCharEncodingByName(g_LyricData.GetEncoding());

// 		if (m_font.GetCharset() != encdoing.nFontCharset)
// 		{
// 			m_font.Destroy();
// 			m_font.Create(m_font.GetName(), font.GetNameOthers(), m_font.GetHeight(), m_font.GetWeight(), m_font.GetItalic(), m_font.GetCharset());
// 		}
	}
#endif*/


	if (m_bUseAlbumArtAsBg || m_bUseBgImg)
	{
		LoadNextBgImage();
	}

	// auto resize || !wraplines
	if ((m_bEnableAutoResize && 
		(m_bAlwaysAutoWidth || g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndWidth"), true)))
		|| !ShouldWrapLines())
		m_pMLData->CopyMLyrLines(m_lyrLines);
	else
		WrapLyricsLines();

	OnLyrDrawContextChanged();

	Invalidate();
}

void CLyricShowObj::OnLyrDrawContextChanged()
{
	m_lyrLines.ClearDrawContextWidth();
}

void CLyricShowObj::OnAdjustHue(float hue, float saturation, float luminance)
{
	if (!m_bUseSkinStyle)
		return;

#define ADJUST_HUE(clrOrg, clr)		\
	if (clrOrg.m_bValid)\
	{\
		clr = clrOrg;\
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(clr);\
	}


	for (int i = TCI_START; i < TCI_COUNT; i++)
	{
		ADJUST_HUE(m_skin_tobLowlight.clr[TOBColorIndex(i)], m_tobLowlight.clr[TOBColorIndex(i)]);
		ADJUST_HUE(m_skin_tobHilight.clr[TOBColorIndex(i)], m_tobHilight.clr[TOBColorIndex(i)]);
	}

	ADJUST_HUE(m_skin_clrBg, m_clrBg);
	ADJUST_HUE(m_skin_clrFg, m_clrFg);
	ADJUST_HUE(m_skin_clrFgLow, m_clrFgLow);
	ADJUST_HUE(m_skin_clrTag, m_clrTag);
	ADJUST_HUE(m_skin_clrFocusLineBg, m_clrFocusLineBg);
}

void CLyricShowObj::WrapLyricsLines()
{
	{
		CRawGraph		*pGraphics;

		pGraphics = m_pSkin->GetMemGraphics();

		pGraphics->SetFont(&m_font);

		g_LyricData.WrapDisplayLyrics(m_lyrLines, pGraphics, m_rcObj.Width() - m_nXMargin * 2);
	}
}

// COMMENT:
// 自动调整迷窗口高度
void CLyricShowObj::AutoHeightSkinAccordLyrics(int nLines)
{
#ifndef _MPLAYER
	if (!m_bEnableAutoResize)
		return;

#ifdef _WIN32
	// auto size feauture is disabled when the skin is embedded into other windw
	if (::GetParent(m_pSkin->GetHandle()) != NULL)
		return;
#endif
	if (m_pSkin->m_wndResizer.IsFixedHeight())
		return;

	if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndHeight"), TRUE))
		return;

	CRect		rc;
	int			nHeightOthers = 0;
	int			nBottomOld;

	m_pSkin->GetSkinRect(rc);
	nHeightOthers = rc.Height() - m_rcObj.Height();
	nBottomOld = rc.bottom;

	rc.bottom = rc.top + nHeightOthers + m_nYMargin * 2 + abs(GetLineHeight() * nLines) + m_nLineSpacing * (nLines - 1);
	if (nBottomOld != rc.bottom)
		m_pSkin->MoveWindow(rc);
#endif
}

// COMMENT:
//		根据歌词的长度，自动调整窗口宽度可以匹配显示歌词
void CLyricShowObj::AutoWidthSkinAccordLyrics(BOOL bSingleRow/* = FALSE*/)
{
#ifndef _WIN32_WCE
	CRect		rc, rcOrg;
	int			nWidthMax = 200;
	int			nWPrev = 0;

	if (!m_pMLData)
		return;

	if (!m_bEnableAutoResize)
		return;

	if (m_pSkin->m_wndResizer.IsFixedWidth())
		return;

	if (!m_bAlwaysAutoWidth && !g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndWidth"), true))
		return;

	if (m_lyrLines.empty())
		return;

#ifdef _WIN32
	// auto size feauture is disabled when the skin is embedded into other windw
	if (::GetParent(m_pSkin->GetHandle()) != NULL)
		return;
#endif

	CRawGraph		*pGraphics;

	pGraphics = m_pSkin->GetMemGraphics();

	pGraphics->SetFont(&m_font);

	//
	// 找到最宽的一行歌词
	m_pSkin->GetWindowRect(&rc);
	rcOrg = rc;
	for (int i = 0; i < (int)m_lyrLines.size(); i++)
	{
		int		w = GetLyricRowTextWidth(pGraphics, m_lyrLines[i]);
		if (bSingleRow)
		{
			if (w + nWPrev > nWidthMax)
				nWidthMax = w + nWPrev;
			nWPrev = w;
		}
		else
		{
			if (w > nWidthMax)
				nWidthMax = w;
		}
	}

	nWidthMax += m_nXMargin * 2 + 8;

	//
	// 调整宽带到可以显示最宽的一行歌词
//	if (rc.right != rc.left + rc.Width() - m_rcObj.Width() + nWidthMax + m_nXMargin * 2 + 40)
	{
		CRect			rcRestrict;
		int				nCxScr;

		nCxScr = rcRestrict.Width();

		GetMonitorRestrictRect(rcOrg, rcRestrict);

		if (m_nLeftAdjust != rc.left)
		{
			m_nLeftOrg = rc.left;
		}
		else
		{
			rc.right -= rc.left - m_nLeftOrg;
			rc.left = m_nLeftOrg;
		}

		if (bSingleRow)
			rc.right = rc.left + rc.Width() - m_rcObj.Width() + nWidthMax;
		else
			rc.right = rc.left + rc.Width() - m_rcObj.Width() + nWidthMax;

		if (rc.Width() < m_pSkin->m_wndResizer.GetMinCx())
			rc.right = rc.left + m_pSkin->m_wndResizer.GetMinCx();

		// Max width is the width of screen
		if (rc.Width() > rcRestrict.Width())
			rc.right = rc.left + rcRestrict.Width();

		// floating lyrics and translucency? Align the skin window to center.
		if (m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS
			&& m_nAlignment == LA_CENTER
			// rcOrg is in center of rcRestrict?
			&& abs((rcRestrict.right + rcRestrict.left) / 2 - (rcOrg.right + rcOrg.left) / 2) < 200)
		{
			int			nOffset;

			nOffset = (rcRestrict.right + rcRestrict.left) / 2 - (rc.right + rc.left) / 2;
			rc.left += nOffset;
			rc.right += nOffset;
		}

		// Out of right border of screen?
		if (rc.right > rcRestrict.right)
		{
			rc.left -= rc.right - rcRestrict.right;
			rc.right -= rc.right - rcRestrict.right;
		}

		// Out of left border of screen?
		if (rc.left < rcRestrict.left)
		{
			rc.right += rcRestrict.left - rc.left;
			rc.left += rcRestrict.left - rc.left;
		}

		m_nLeftAdjust = rc.left;

		// NOT changed
		if (rc.Width() <= rcOrg.Width() + 4 && rc.Width() >= rcOrg.Width() - 50
			&& rcOrg.left >= rcRestrict.left && rcOrg.right <= rcRestrict.right)
			return;

		m_pSkin->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
	}
#endif // #ifndef _WIN32_WCE
}

void CLyricShowObj::OnPlayTimeChangedUpdate()
{
	CRect	rcUpdate;
	CRawGraph		*pmemGraphics;

	if (m_pSkin->IsIconic() || !IsVisible() || !IsParentVisible())
		return;

	pmemGraphics = m_pSkin->GetMemGraphics();

#ifdef _WIN32_DESKTOP
	if (m_pSkin->GetEnableTranslucencyLayered())
	{
		pmemGraphics->SetFont(&m_font);

		m_font.EnableGdiplusAntialias(CMLGraphicsFactory::GetGraphicsType() == MGT_GDIPLUS);

		FastDraw(pmemGraphics, &rcUpdate);

		if (!rcUpdate.IsRectEmpty())
			m_pSkin->UpdateLayeredWindowUsingMemGraph(pmemGraphics);
	}
	else
#endif
	{
		IMLGraphicsBase		*pGraphics;

		pGraphics = m_pSkin->GetGraphics();

		pmemGraphics->SetFont(&m_font);

		FastDraw(pmemGraphics, &rcUpdate);

		pGraphics->DrawGraphics(pmemGraphics, 
			rcUpdate.left, rcUpdate.top, 
			rcUpdate.Width(), rcUpdate.Height(),
			rcUpdate.left, rcUpdate.top);

		m_pSkin->ReleaseGraphics(pGraphics);
	}
}

//
// 取得此行歌词的Fade In color
//
void CLyricShowObj::GetFadeInColor(PML_ROW pRow, MLColor clrIn, MLColor clrOut, MLColor &color)
{
	int				nTime;

	if (!pRow)
		return;

	nTime = pRow->nBegTime - m_pMLData->GetPlayElapsedTime();
	if (nTime <= 1000 && nTime >= -1000)
	{
		int		nPart1;
		if (nTime <= 0)
			nTime = 1000 + nTime;
		else
			nTime = 1000 - nTime;

		nPart1 = 1000 - nTime;
		color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / 1000,
			(clrOut.g() * nPart1 + clrIn.g() * nTime) / 1000,
			(clrOut.b() * nPart1 + clrIn.b() * nTime) / 1000);
	}
	else
		color = clrOut;
}

//
// 取得此行歌词的Fade out color
//
void CLyricShowObj::GetFadeOutColor(PML_ROW pRow, MLColor clrIn, MLColor clrOut, MLColor &color)
{
	int				nTime;

	if (!pRow)
		return;

	nTime = m_pMLData->GetPlayElapsedTime() - pRow->nEndTime;
	if (nTime <= 1000 && nTime >= -1000)
	{
		int		nPart1;
		if (nTime <= 0)
			nTime = 1000 + nTime;
		else
			nTime = 1000 - nTime;

		nPart1 = 1000 - nTime;
		color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / 1000,
			(clrOut.g() * nPart1 + clrIn.g() * nTime) / 1000,
			(clrOut.b() * nPart1 + clrIn.b() * nTime) / 1000);
	}
	else
		color = clrOut;
}

int CLyricShowObj::GetAlpha(PML_ROW pRow)
{
	int				nTime;

	if (pRow->nBegTime > m_pMLData->GetPlayElapsedTime())
	{
		nTime = pRow->nBegTime - m_pMLData->GetPlayElapsedTime();
		if (nTime <= 1000 && nTime >= -1000)
		{
			return (1000 - nTime) * 255 / 1000;
		}
		else
			return 0;
	}
	else
	{
		nTime = m_pMLData->GetPlayElapsedTime() - pRow->nEndTime;
		if (nTime >= 1000)
			return 0;
		else if (nTime > 0)
			return (1000 - nTime) * 255 / 1000;
		else
			return 255;
	}
}

//
// 取得此行歌词的Fade out color
//
void CLyricShowObj::GetFadeOutColorEx(PML_ROW pRow, MLColor clrIn, MLColor clrOut, MLColor &color)
{
	int				nTime;

	if (!pRow)
		return;

	if (pRow->nBegTime > m_pMLData->GetPlayElapsedTime())
	{
		nTime = pRow->nBegTime - m_pMLData->GetPlayElapsedTime();
		if (nTime <= 1000 && nTime >= -1000)
		{
			int		nPart1;
			if (nTime <= 0)
				nTime = 1000 + nTime;
			else
				nTime = 1000 - nTime;

			nPart1 = 1000 - nTime;
			color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / 1000,
				(clrOut.g() * nPart1 + clrIn.g() * nTime) / 1000,
				(clrOut.b() * nPart1 + clrIn.b() * nTime) / 1000);
		}
		else
			color = clrOut;
	}
	else
	{
		nTime = m_pMLData->GetPlayElapsedTime() - pRow->nEndTime;
		if (nTime <= 1000 && nTime >= -1000)
		{
			int		nPart1;
			if (nTime <= 0)
				nTime = 1000 + nTime;
			else
				nTime = 1000 - nTime;

			nPart1 = 1000 - nTime;
			color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / 1000,
				(clrOut.g() * nPart1 + clrIn.g() * nTime) / 1000,
				(clrOut.b() * nPart1 + clrIn.b() * nTime) / 1000);
		}
		else
			color = clrOut;
	}
}

void CLyricShowObj::LoadBgImageFolder()
{
	m_vPicFiles.clear();
	m_nNextPic = 0;
	m_nNextPicInFolder = 0;

	if (m_strBgPicFolder.empty())
		return;

	DirStringFormat(m_strBgPicFolder);

	CMLFileFind		finder;
	static LPCTSTR	szSupportedPicExt[] = { _T(".jpg"), _T(".gif"), _T(".bmp"), _T(".tif"), _T(".png") };

	if (!finder.OpenDir(m_strBgPicFolder.c_str()))
		return;

	while (finder.FindNext())
	{
		for (int i = 0; i < CountOf(szSupportedPicExt); i++)
		{
			if (FileIsExtSame(finder.GetCurName(), szSupportedPicExt[i]))
				m_vPicFiles.push_back(m_strBgPicFolder + finder.GetCurName());
		}
	}
}

void CLyricShowObj::LoadNextBgImage()
{
	PRAW_IMAGE_DATA	hBmp = NULL;

	if (!m_bUseAlbumArtAsBg && !m_bUseBgImg)
	{
		m_curAlbumArt.Close();
		m_img.Destroy();
		return;
	}

	if (!m_curAlbumArt.IsLoaded())
	{
		m_curAlbumArt.Load();
		m_nNextPic = 0;
		hBmp = m_curAlbumArt.LoadAlbumArtByIndex(m_nNextPic);
	}

	if (!hBmp)
	{
		// is m_nNextPic index correct?
		if (m_curAlbumArt.GetPicCount() + m_vPicFiles.size() == 0)
			return;

		if (m_nNextPic >= m_curAlbumArt.GetPicCount() + (int)m_vPicFiles.size())
			m_nNextPic = 0;
	}

	if (!hBmp && m_curAlbumArt.GetPicCount() > 0)
	{
		// load album art
		while (m_nNextPic < m_curAlbumArt.GetPicCount() && !hBmp)
		{
			hBmp = m_curAlbumArt.LoadAlbumArtByIndex(m_nNextPic);
			m_nNextPic++;
		}
	}

	if (!hBmp && m_bUseBgImg && !m_vPicFiles.empty())
	{
		if (m_nNextPic - m_curAlbumArt.GetPicCount() != m_nNextPicInFolder)
		{
			if (m_nNextPicInFolder < 0 || m_nNextPicInFolder >= (int)m_vPicFiles.size())
				m_nNextPicInFolder = 0;
			m_nNextPic = m_nNextPicInFolder + m_curAlbumArt.GetPicCount();
		}

		if (m_nNextPicInFolder >= 0 && m_nNextPicInFolder < (int)m_vPicFiles.size())
		{
			hBmp = LoadRawImageDataFromFile(m_vPicFiles[m_nNextPicInFolder].c_str());
			m_nNextPic++;
			m_nNextPicInFolder++;
		}
	}

	if (m_img.IsValid())
		m_img.Destroy();

	if (hBmp)
	{
// 		if (g_Profile.GetProfileBool(_T("EnableBlendBgPic"), true))
// 		{
// 			CImageData		imgData;
// 			int				nAlpha;
// 
// 			nAlpha = g_Profile.GetProfileInt(_T("BlendBgPicPercent"), 70);
// 			if (nAlpha < 0 || nAlpha > 100)
// 				nAlpha = 128;
// 
// 			if (imgData.CopyFromHBitmap(hBmp))
// 			{
// 				DeleteObject(hBmp);
// 
// 				imgData.ConvertTo24Bpp();
// 
// 				BlendImage(imgData, m_clrBg.get(), nAlpha * 255 / 100);
// 
// 				m_img.Attach(imgData.ToHBitmap(NULL));
// 			}
// 			else
// 				m_img.Attach(hBmp);
// 		}
// 		else
		m_img.Attach(hBmp);

		m_img.GetOrginalSize(m_img.m_cx, m_img.m_cy);
	}

	if (m_img.IsValid())
	{
		//m_bgImgPos = (BgImagePos)g_Profile.GetProfileInt(_T("BgPicPos"), BIP_CENTER);
	}
}


void CLyricShowObj::OnTimer(int nId)
{
	LoadNextBgImage();
	Invalidate();
}

bool CLyricShowObj::ShouldWrapLines()
{
	return m_bCanWrapLines && g_Profile.GetProfileBool(SZ_SECT_LYR_DISPLAY, _T("WrapLines"), true);
}

void CLyricShowObj::AddErrorInfo(LPCTSTR szInfo)
{
	CRawGraph		*pGraphics;

	pGraphics = m_pSkin->GetMemGraphics();

	pGraphics->SetFont(&m_font);

	g_LyricData.AddErrorInfo(pGraphics, m_rcObj.Width() - m_nXMargin * 2, szInfo);
}

void CLyricShowObj::OnSize(int cx, int cy)
{
	if (ShouldWrapLines())
		WrapLyricsLines();
}

#ifndef _WIN32_WCE
void CLyricShowObj::SetAdSentence(int nID, LPCTSTR szLink, int x, int y, int nWidth, int nHeight)
{
	V_ADS::iterator	it, itEnd;
	itEnd = m_vads.end();
	for (it = m_vads.begin(); it != itEnd; ++it)
	{
		AdSentence	&ads = (*it);
		if (ads.nID == nID)
		{
			ads.strLink = szLink;
			ads.rcAdWords.SetRect(x, y, x + nWidth, y + nHeight);
			return;
		}
	}

	AdSentence		ads;
	ads.nID = nID;
	ads.strLink = szLink;
	ads.rcAdWords.SetRect(x, y, x + nWidth, y + nHeight);
	m_vads.push_back(ads);
}

int CLyricShowObj::GetLineVertAlignPos()
{
	int		nPos;

	if (m_vertLineAlign == VLA_TOP)
		nPos = m_rcObj.top;
	else if (m_vertLineAlign == VLA_BOTTOM)
		nPos = m_rcObj.bottom;
	else
		nPos = m_rcObj.top + m_rcObj.Height() / 2;

	nPos += m_nVertLineOffset;

	if (nPos <= m_rcObj.top + m_nYMargin + 5 || nPos >= m_rcObj.bottom - m_nYMargin - GetFontHeight())
		nPos = m_rcObj.top + m_rcObj.Height() / 2;

	return nPos;
}

#endif

BOOL CLyricShowObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("UseSkinStyle")))
	{
		m_bUseSkinStyle = IsTRUE(szValue);
	}
	else if (_tcsicmp(szProperty, _T("LyrAlign")) == 0)
		SetLyrAlign(szValue);
	else if (_tcsicmp(szProperty, _T("LyrDrawOpt")) == 0)
		SetLyrDrawOpt(szValue);
	else if (_tcsicmp(szProperty, _T("BgColor")) == 0)
	{
		SetBgColor(szValue);
		m_skin_clrBg = m_clrBg;
		m_skin_clrBg.m_bValid = true;
	}
	else if (IsPropertyName(szProperty, _T("FgColor")))
	{
		GetColorValue(m_clrFg, szValue);
		m_skin_clrFg = m_clrFg;
		m_skin_clrFg.m_bValid = true;
	}
	else if (IsPropertyName(szProperty, _T("FgLowColor")))
	{
		GetColorValue(m_clrFgLow, szValue);
		m_skin_clrFgLow = m_clrFgLow;
		m_skin_clrFgLow.m_bValid = true;
	}
	else if (IsPropertyName(szProperty, _T("TagColor")))
	{
		GetColorValue(m_clrTag, szValue);
		m_skin_clrTag = m_clrTag;
		m_skin_clrTag.m_bValid = true;
	}
	else if (IsPropertyName(szProperty, _T("FocusLineBgColor")))
	{
		GetColorValue(m_clrFocusLineBg, szValue);
		m_skin_clrFocusLineBg = m_clrFocusLineBg;
		m_skin_clrFocusLineBg.m_bValid = true;
	}
#ifdef _WIN32_DESKTOP
	else if (IsPropertyName(szProperty, _T("ShadowMode")))
	{
		SetLyrShadowMode(szValue);
	}
	else if (IsPropertyName(szProperty, _T("ThickOutline")))
	{
		m_bThickOutline = IsTRUE(szValue);
		m_font.SetThickOutline(m_bThickOutline);
	}
	else if (IsPropertyName(szProperty, _T("FadeOutBorder")))
	{
		m_bFadeOutBorder = IsTRUE(szValue);
	}
#endif
	else if (_tcsicmp(szProperty, _T("UseBgImage")) == 0)
	{
		m_bUseBgImg = IsTRUE(szValue);
		m_pSkin->UnregisterTimerObject(this);
		if (m_bUseBgImg)
		{
			LoadBgImageFolder();
			LoadNextBgImage();
			m_pSkin->RegisterTimerObject(this, g_Profile.GetProfileInt(m_strSectName.c_str(), _T("SlideDelayTime"), 10) * 1000);
		}
		else if (!m_bUseAlbumArtAsBg)
		{
			m_img.Destroy();
			m_vPicFiles.clear();
		}
	}
	else if (_tcsicmp(szProperty, _T("BgPicFolder")) == 0)
	{
		if (_tcscmp(szValue, m_strBgPicFolder.c_str()) != 0)
		{
			m_strBgPicFolder = szValue;
			UpdateBgImage();
		}
	}
	else if (IsPropertyName(szProperty, _T("UseAlbumArtAsBg")))
	{
		m_bUseAlbumArtAsBg = IsTRUE(szValue);
		UpdateBgImage();
	}
	else if (IsPropertyName(szProperty, _T("DarkenLyrBg")))
		m_bDarkenLyrBgOnImg = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("BlendingLyrTextOnBgPic")))
		m_bTobEnabledOnBgPic = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("SlideDelayTime")))
	{
		if (m_bUseBgImg)
		{
			m_pSkin->UnregisterTimerObject(this);
			m_pSkin->RegisterTimerObject(this, _ttoi(szValue) * 1000);
		}
	}
	else if (_tcsicmp(szProperty, _T("LineSpacing")) == 0)
		m_nLineSpacing = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("Font")) == 0)
	{
		int			nHeight, nWeight;
		BYTE		byItalic;
		tstring		strFaceName, strFaceNameOthers;

		// 创建English 歌词的字体
		FontPropertyValueFromStr(szValue, nHeight, nWeight, byItalic, strFaceName, strFaceNameOthers);
		m_font.Create(strFaceName.c_str(), strFaceNameOthers.c_str(), nHeight, nWeight, Tobool(byItalic));

		if (IsBlendingLyrText())
		{
			if (m_tobHilight.imgPattern.height() != GetPatternFontHeight())
			{
				LoadLyrOverlaySettings();
			}
		}

		m_nFontHeight = m_font.GetHeight();
		OnLyrDrawContextChanged();
	}
	else if (IsPropertyName(szProperty, _T("BlendingLyrText")))
	{
		m_bTobEnabled = IsTRUE(szValue);
		m_skin_bTobEnabled = m_bTobEnabled;
	}
	else if (IsPropertyName(szProperty, _T("LOBHilight")))
	{
		m_skin_tobHilight.obm = (OverlayBlendingMode)StringToID(g_idsLOB, szValue, OBM_UNKNOWN);
	}
	else if (IsPropertyName(szProperty, _T("LOBLowlight")))
	{
		m_skin_tobLowlight.obm = (OverlayBlendingMode)StringToID(g_idsLOB, szValue, OBM_UNKNOWN);
	}
	else if (IsPropertyName(szProperty, _T("Karaoke")))
		m_bKaraoke = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("EnableAutoResize")) == 0)
		m_bEnableAutoResize = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("AlwaysAutoWidth")) == 0)
		m_bAlwaysAutoWidth = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableAdjustVertAlign")))
		m_bEnableAdjustVertAlign = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("FloatingLyr")))
	{
		if (IsTRUE(szValue))
		{
			m_etDispSettings = ET_LYRICS_FLOATING_SETTINGS;
			m_strSectName = SZ_SECT_FLOATING_LYR;
		}
	}
	else if (IsPropertyName(szProperty, _T("XMargin")))
		m_nXMargin = _ttoi(szValue);
	else
	{
		TOBColorIndex	i;

		for (i = TCI_START; i < TCI_COUNT; i = (TOBColorIndex)(i + 1))
		{
			if (IsPropertyName(szProperty, _LyrShowTOBFieldNameHilight[i]))
			{
				GetColorValue(m_tobHilight.clr[i], szValue);
				m_skin_tobHilight.clr[i] = m_tobHilight.clr[i];
				m_skin_tobHilight.clr[i].m_bValid = true;
				break;
			}
			else if (IsPropertyName(szProperty, _LyrShowTOBFieldNameLowlight[i]))
			{
				GetColorValue(m_tobLowlight.clr[i], szValue);
				m_skin_tobLowlight.clr[i] = m_tobLowlight.clr[i];
				m_skin_tobLowlight.clr[i].m_bValid = true;
				break;
			}
		}

		// Not processed?
		if (i == TCI_COUNT)
			return FALSE;
	}

	return TRUE;
}

bool CLyricShowObj::OnLyrDisplaySettings(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (m_bUseSkinStyle)
		return true;

	if (IsPropertyName(szProperty, _T("Hue")))
	{
		AdjustHue(float(_ttoi(szValue)));
		m_pSkin->InvalidateRect();
		return true;
	}
	else if (IsPropertyName(szProperty, _T("BgColor")))
	{
		SetBgColor(szValue);
		m_pSkin->InvalidateRect();
		return true;
	}
	else if (IsPropertyName(szProperty, _T("FgColor")))
	{
		GetColorValue(m_clrFg, szValue);
	}
	else if (IsPropertyName(szProperty, _T("FgLowColor")))
	{
		GetColorValue(m_clrFgLow, szValue);
	}
	else if (IsPropertyName(szProperty, _T("TagColor")))
	{
		GetColorValue(m_clrTag, szValue);
	}
	else if (IsPropertyName(szProperty, _T("FocusLineBgColor")))
	{
		GetColorValue(m_clrFocusLineBg, szValue);
	}
	else if (IsPropertyName(szProperty, _T("BlendingLyrText")))
	{
		LoadLyrOverlaySettings();
	}
	//
	// High light of lyrics overlay blending settings
	//
	else if (IsPropertyName(szProperty, _T("LOBHilight")))
	{
		LoadLOBHilightSettings();
	}
	else if (IsPropertyName(szProperty, _T("HilightPattern")))
	{
		if (IsBlendingLyrText())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataFromFile(szValue);
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				m_tobHilight.imgPattern.Attach(pImage);
		}
	}
	//
	// Low light of lyrics overlay blending settings
	//
	else if (IsPropertyName(szProperty, _T("LOBLowlight")))
	{
		LoadLOBLowlightSettings();
	}
	else if (IsPropertyName(szProperty, _T("LowlightPattern")))
	{
		if (IsBlendingLyrText())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataFromFile(szValue);
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				m_tobLowlight.imgPattern.Attach(pImage);
		}
	}
	else if (IsPropertyName(szProperty, _T("ReloadSkinDefClrTheme")))
	{
		ReloadThemeOfSkin(true);
		m_pSkin->InvalidateRect();
		return true;
	}
	else if (IsPropertyName(szProperty, _T("EnableAdjustVertAlign")))
	{
		m_bEnableAdjustVertAlignUser = IsTRUE(szValue);
	}
	else
	{
		TOBColorIndex	i;
		bool			bCreatePatternImg = false;

		//
		// Highlight color settings.
		//
		for (i = TCI_START; i < TCI_COUNT; i = (TOBColorIndex)(i + 1))
		{
			if (IsPropertyName(szProperty, _LyrShowTOBFieldNameHilight[i]))
			{
				GetColorValue(m_tobHilight.clr[i], szValue);
				break;
			}
		}
		if (IsBlendingLyrText() &&
			(i == TCI_AUTO_GRADIENT || i == TCI_GRADIENT1 || i == TCI_GRADIENT2 || i == TCI_GRADIENT3))
			CreateLyrOverlayBlendingPattern(m_tobHilight, GetPatternFontHeight());
		else if (i == TCI_COUNT)
		{
			//
			// Lowlight color settings.
			//
			for (i = TCI_START; i < TCI_COUNT; i = (TOBColorIndex)(i + 1))
			{
				if (IsPropertyName(szProperty, _LyrShowTOBFieldNameLowlight[i]))
				{
					GetColorValue(m_tobLowlight.clr[i], szValue);
					break;
				}
			}
			if (IsBlendingLyrText() &&
				(i == TCI_AUTO_GRADIENT || i == TCI_GRADIENT1 || i == TCI_GRADIENT2 || i == TCI_GRADIENT3))
				CreateLyrOverlayBlendingPattern(m_tobLowlight, GetPatternFontHeight());
			else if (i == TCI_COUNT)
			{
				//
				// Got default processing
				//
				if (!SetProperty(szProperty, szValue))
					return false;
			}
		}
	}

	Invalidate();

	return true;
}

void CLyricShowObj::UpdateBgImage()
{
	if (m_img.IsValid())
		m_img.Destroy();

	if (m_curAlbumArt.IsLoaded())
		m_curAlbumArt.Close();

	if (m_bUseBgImg || m_bUseAlbumArtAsBg)
		LoadNextBgImage();
}

void CLyricShowObj::ReloadThemeOfSkin(bool bSavetoProfile)
{
	tstring			strColor;

	if (bSavetoProfile && m_skin_bTobEnabled)
	{
		ProfileWriteTOBToOld(m_bTobEnabled, m_tobHilight, m_tobLowlight);
		g_Profile.WriteProfileInt(SZ_SECT_LYR_DISPLAY, _T("ReloadTOBFromOld"), true);
	}

#define		SET_THEME_COLOR(szKeyName, skin_clr, clr)	\
			if (skin_clr.m_bValid)\
			{\
				clr = skin_clr;\
				if (bSavetoProfile)\
				{\
					ColorToStr(skin_clr, strColor);\
					g_Profile.WriteProfileString(m_strSectName.c_str(), szKeyName, strColor.c_str());\
				}\
			}

	if (m_skin_clrBg.m_bValid)
	{
		ColorToStr(m_skin_clrBg, strColor);
		SetBgColor(strColor.c_str());
		if (bSavetoProfile)
			g_Profile.WriteProfileString(m_strSectName.c_str(), _T("BgColor"), strColor.c_str());
	}

	SET_THEME_COLOR(_T("FgColor"), m_skin_clrFg, m_clrFg);
	SET_THEME_COLOR(_T("FgLowColor"), m_skin_clrFgLow, m_clrFgLow);
	SET_THEME_COLOR(_T("TagColor"), m_skin_clrTag, m_clrTag);
	SET_THEME_COLOR(_T("FocusLineBgColor"), m_skin_clrFocusLineBg, m_clrFocusLineBg);
	if (m_skin_bTobEnabled)
	{
		m_bTobEnabled = m_skin_bTobEnabled;

		g_Profile.WriteProfileInt(m_strSectName.c_str(), _T("BlendingLyrText"), m_bTobEnabled);

		TOBColorIndex	i;

		for (i = TCI_START; i < TCI_COUNT; i = (TOBColorIndex)(i + 1))
		{
			SET_THEME_COLOR(_LyrShowTOBFieldNameHilight[i], m_skin_tobHilight.clr[i], m_tobHilight.clr[i]);
			SET_THEME_COLOR(_LyrShowTOBFieldNameLowlight[i], m_skin_tobLowlight.clr[i], m_tobLowlight.clr[i]);
		}

		g_Profile.WriteProfileInt(m_strSectName.c_str(), _T("LOBHilight"), m_skin_tobHilight.obm);
		m_tobHilight.obm = m_skin_tobHilight.obm;

		g_Profile.WriteProfileInt(m_strSectName.c_str(), _T("LOBLowlight"), m_skin_tobLowlight.obm);
		m_tobLowlight.obm = m_skin_tobLowlight.obm;

		CreateLyrOverlayBlendingPattern(m_tobHilight, GetPatternFontHeight());
		CreateLyrOverlayBlendingPattern(m_tobLowlight, GetPatternFontHeight());
	}

	if (bSavetoProfile)
	{
//		else
//			g_Profile.WriteProfileInt(SZ_SECT_LYR_DISPLAY, _T("BlendingLyrText"), false);
	}
}

void CLyricShowObj::SetLyrShadowMode(LPCTSTR szValue)
{
	static ID_TO_STRING	_id2str[] = {
		{ CRawBmpFont::SM_OUTLINE, _T("outline") },
		{ CRawBmpFont::SM_SHADOW, _T("shadow") },
		{ 0, NULL },
	};
	m_lyrShadowMode = (CRawBmpFont::ShadowMode)StringToID(_id2str, szValue, m_lyrShadowMode);
	m_font.SetShadowMode(m_lyrShadowMode);
}

void CLyricShowObj::SetLyrAlign(LPCTSTR szValue)
{
	static ID_TO_STRING	_id2strLyrAlign[] = {
		{ LA_LEFT, _T("left") },
		{ LA_CENTER, _T("center") },
		{ LA_RIGHT, _T("right") },
		{ 0, NULL },
	};
	m_nAlignment = (LyrAlignment)StringToID(_id2strLyrAlign, szValue, LA_CENTER);
}

void CLyricShowObj::SetLyrDrawOpt(LPCTSTR szValue)
{
	static ID_TO_STRING	_id2strLyrAlign[] = {
		{ DO_NORMAL, _T("normal") },
		{ DO_FADEOUT_LOWCOLOR, _T("fadein") },
		{ DO_FADEOUT_BG, _T("fadeout") },
		{ 0, NULL },
	};
	m_LyricsDisplayOpt = (DISPLAY_OPTIONS)StringToID(_id2strLyrAlign, szValue, DO_FADEOUT_LOWCOLOR);
}

void CLyricShowObj::SetBgColor(LPCTSTR szValue)
{
	if (IsEmptyString(szValue))
		return;

	GetColorValue(m_clrBg, szValue);

	if (m_brBg.IsValid())
		m_brBg.Destroy();
	m_brBg.CreateSolidBrush(m_clrBg);

	if (m_pSkin)
		m_pSkin->SetProperty(_T("BgColor"), szValue);
}

void CLyricShowObj::LoadLyrOverlaySettings()
{
	m_bTobEnabled = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("BlendingLyrText"), m_bTobEnabled);
	if (IsBlendingLyrText())
	{
		SetLyrShadowMode(g_Profile.GetProfileString(m_strSectName.c_str(), _T("ShadowMode"), _T("")));
		m_bThickOutline = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("ThickOutline"), m_bThickOutline);
		m_font.SetThickOutline(m_bThickOutline);

		ProfileGetColorValue(m_tobHilight.clr[TCI_BORDER], m_strSectName.c_str(), _T("LOBHilightBorder"));
		ProfileGetColorValue(m_tobLowlight.clr[TCI_BORDER], m_strSectName.c_str(), _T("LOBLowlightBorder"));

		LoadLOBHilightSettings();
		LoadLOBLowlightSettings();
	}
	else
	{
		m_tobHilight.obm = OBM_COLOR;
		m_tobLowlight.obm = OBM_COLOR;
	}
}

void CLyricShowObj::LoadLOBHilightSettings()
{
	if (!IsBlendingLyrText())
		return;

	LoadLyrOverlayBlendingSettings(m_strSectName.c_str(), m_tobHilight, GetPatternFontHeight(), true);
}

void CLyricShowObj::LoadLOBLowlightSettings()
{
	if (!IsBlendingLyrText())
		return;

	LoadLyrOverlayBlendingSettings(m_strSectName.c_str(), m_tobLowlight, GetPatternFontHeight(), false);
}

void CLyricShowObj::LoadAllSettings()
{
	if (m_bUseSkinStyle)
		return;

	{
		int			nHeight, nWeight;
		BYTE		byItalic;
		tstring		strFaceName, strFaceNameOthers;

		ProfileGetLyricsFont(m_strSectName.c_str(), nHeight, nWeight, byItalic, strFaceName, strFaceNameOthers);

		m_font.Create(strFaceName.c_str(), strFaceNameOthers.c_str(), nHeight, nWeight, byItalic);
		m_nFontHeight = m_font.GetHeight();
	}

	ProfileGetColorValue(m_clrFg, m_strSectName.c_str(), _T("FgColor"));
	ProfileGetColorValue(m_clrFgLow, m_strSectName.c_str(), _T("FgLowColor"));

	// Floating lyrics won't use Bgcolor and TagColor
	if (m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS)
	{
		SetBgColor(g_Profile.GetProfileString(m_strSectName.c_str(), _T("BgColor"), _T("")));
		ProfileGetColorValue(m_clrTag, m_strSectName.c_str(), _T("TagColor"));
		ProfileGetColorValue(m_clrFocusLineBg, m_strSectName.c_str(), _T("FocusLineBgColor"));
	}

	LoadLyrOverlaySettings();

	// adjust hue?
	float	hue = (float)(g_Profile.GetProfileInt(m_strSectName.c_str(), _T("Hue"), 0));
	if (hue != 0)
		AdjustHue(hue);

	SetLyrAlign(g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrAlign"), _T("")));

	SetLyrDrawOpt(g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrDrawOpt"), _T("")));

	m_nLineSpacing = CMPlayerSettings::GetSettings(m_strSectName.c_str(), _T("LineSpacing"), 2);
	m_bKaraoke = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("Karaoke"), m_bKaraoke);

#ifndef _WIN32_WCE
	m_vertLineAlign = (VertLineAlign)g_Profile.GetProfileInt(m_strSectName.c_str(), _T("VertAlign"), VLA_CENTER);
	if (m_vertLineAlign > VLA_BOTTOM || m_vertLineAlign < VLA_TOP)
		m_vertLineAlign = VLA_CENTER;
	m_nVertLineOffset = g_Profile.GetProfileInt(m_strSectName.c_str(), _T("VertAlignOffset"), 0);
#endif

	//
	// Load Background image settings.
	//		Floating lyrics don't use background....
	if (m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS)
	{
		m_bUseAlbumArtAsBg = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("UseAlbumArtAsBg"), false);
		m_bUseBgImg = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("UseBgImage"), false);
		m_bTobEnabledOnBgPic = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("BlendingLyrTextOnBgPic"), true);
		m_strBgPicFolder = CMLProfile::GetDir(m_strSectName.c_str(), _T("BgPicFolder"), _T(""));

		m_bDarkenLyrBgOnImg = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("DarkenLyrBg"), true);
		m_nDarkenTopArea = float(1.25);
		m_nDarkenBottomArea = float(1.25);

		if (m_bUseBgImg)
		{
			LoadBgImageFolder();
			m_nNextPicInFolder = g_Profile.GetProfileInt(m_strSectName.c_str(), _T("NextPicInFolder"), 0);
		}

		if (m_bUseBgImg || m_bUseAlbumArtAsBg)
			LoadNextBgImage();

		if (!m_bTobEnabled && m_bTobEnabledOnBgPic)
		{
			LoadLyrOverlaySettings();
		}
	}
}

void AdjustColorHue(MLColor &clr, float hueOffset)
{
	COLORREF	c = clr.get();
	AdjustColorHue(c, hueOffset);
	clr.set(c);
}

void CLyricShowObj::AdjustHue(float hue)
{
	if (m_hue != 0)
	{
		// Load orginal settings.
		ProfileGetColorValue(m_clrFg, m_strSectName.c_str(), _T("FgColor"));
		ProfileGetColorValue(m_clrFgLow, m_strSectName.c_str(), _T("FgLowColor"));
		if (m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS)
		{
			SetBgColor(g_Profile.GetProfileString(m_strSectName.c_str(), _T("BgColor"), _T("")));
			ProfileGetColorValue(m_clrTag, m_strSectName.c_str(), _T("TagColor"));
			ProfileGetColorValue(m_clrFocusLineBg, m_strSectName.c_str(), _T("FocusLineBgColor"));
		}

		LoadLyrOverlaySettings();
	}

	m_hue = hue;

	if (hue == 0)
		return;

	// Adjust hue
	AdjustColorHue(m_clrFg, hue);
	AdjustColorHue(m_clrFgLow, hue);
	AdjustColorHue(m_clrTag, hue);
	AdjustColorHue(m_clrFocusLineBg, hue);
	AdjustColorHue(m_clrBg, hue);
	if (m_brBg.IsValid())
		m_brBg.Destroy();
	m_brBg.CreateSolidBrush(m_clrBg);

	tstring		strColor;
	ColorToStr(m_clrBg, strColor);
	if (m_pSkin)
		m_pSkin->SetProperty(_T("BgColor"), strColor.c_str());

	for (TOBColorIndex i = TCI_START; i < TCI_COUNT; i = (TOBColorIndex)(i + 1))
	{
		AdjustColorHue(m_tobHilight.clr[i], hue);
		AdjustColorHue(m_tobLowlight.clr[i], hue);
	}

	if (m_tobHilight.imgPattern.IsValid())
		AdjustImageHue(m_tobHilight.imgPattern.GetHandle(), hue);

	if (m_tobLowlight.imgPattern.IsValid())
		AdjustImageHue(m_tobLowlight.imgPattern.GetHandle(), hue);
}
