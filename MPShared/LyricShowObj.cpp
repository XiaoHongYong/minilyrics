/********************************************************************
	Created  :	2002年1月3日 16:21:56
	FileName :	LyricShowObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowObj.h"
#include "LyricShowAgentObj.h"

#ifndef _WIN32
bool IsCharAlphaNumeric(int c) {
	return IsAlpha(c) || IsDigit(c);
}
#endif


PRAW_IMAGE_DATA AutoFitImageToRect(PRAW_IMAGE_DATA pImgSrc, int wDst, int hDst, int nBitCount)
{
	ASSERT(pImgSrc);
	CRect	rc;
	int		x = 0, y = 0;
	int		wSrcToBlt, hSrcToBlt;

	if (!pImgSrc || pImgSrc->height == 0 || hDst == 0 || wDst == 0)
		return NULL;

	if (float(pImgSrc->width) / pImgSrc->height >= float(wDst) / hDst)
	{
		wSrcToBlt = (int)(float(wDst) * pImgSrc->height / hDst + 0.5);
		hSrcToBlt = pImgSrc->height;
		x = (pImgSrc->width - wSrcToBlt) / 2;
		y = 0;
	}
	else
	{
		wSrcToBlt = pImgSrc->width;
		hSrcToBlt = (int)(float(hDst) * pImgSrc->width / wDst + 0.5);
		x = 0;
		y = (pImgSrc->height - hSrcToBlt) / 2;
	}

	CRawImage	imgSrc;
	CRawImage	imgDst;
	PRAW_IMAGE_DATA		pImgDst;

	pImgDst = new RAW_IMAGE_DATA;
	if (!pImgDst->Create(wDst, hDst, pImgSrc->bitCount))
		return NULL;

	imgSrc.Attach(pImgSrc);
	imgDst.Attach(pImgDst);

	imgSrc.StretchBlt(&imgDst, 0, 0, wDst, hDst, x, y, wSrcToBlt, hSrcToBlt, BPM_COPY | BPM_BILINEAR);

	if (pImgSrc->bitCount != nBitCount)
	{
		PRAW_IMAGE_DATA		pImgDstNew;
		pImgDstNew = new RAW_IMAGE_DATA;
		if (pImgDstNew->Create(wDst, hDst, nBitCount))
		{
			imgSrc.Detach();
			imgSrc.Attach(pImgDst);
			imgDst.Detach();
			imgDst.Attach(pImgDstNew);

			imgSrc.Blt(&imgDst, 0, 0, BPM_COPY);

			FreeRawImage(pImgDst);
			pImgDst = pImgDstNew;
		}
	}

	imgSrc.Detach();
	imgDst.Detach();

	return pImgDst;
}

int GetLyricFragDrawWidth(LyricsPiece *pFrag, CRawGraph* pGraphics)
{
	ASSERT(pFrag);

	if (pFrag->nDrawWidth == 0)
	{
		SIZE			size;
		pGraphics->GetTextExtentPoint32(pFrag->szLyric, pFrag->nLen, &size);
		pFrag->nDrawWidth = (int16)size.cx;
	}

	return pFrag->nDrawWidth;
}

int GetMLRowDrawWidth(LyricsLine *pRow, CRawGraph *pGraph)
{
	VecLyricsPiece::iterator	it, itEnd;
	int					w = 0;
	SIZE				size;

	itEnd = pRow->vFrags.end();
	for (it = pRow->vFrags.begin(); it != itEnd; it++)
	{
		LyricsPiece *pFrag = *it;

		if (pGraph->GetTextExtentPoint32(pFrag->szLyric, pFrag->nLen, &size))
			w += size.cx;
	}

	return w;
}


void WrapWords(CRawGraph *pGraph, int nWidthMax, LPCTSTR szText, vector<tstring> &vStr)
{
	if (_tcslen(szText) <= 10)
	{
		vStr.push_back(szText);
		return;
	}

	tstringex	str;

	StringIterator	strIterator(szText);
	while (!strIterator.IsEOS())
	{
		int nWidth = 0;
		int nLineBeg = strIterator.GetPos();
		int nLastBreak = -1;
		for (; !strIterator.IsEOS(); ++strIterator)
		{
			SIZE size;
			if (pGraph->GetTextExtentPoint32(strIterator.CurChar(), strIterator.CurChar().size(), &size))
				nWidth += size.cx;
			if (nWidth > nWidthMax && strIterator.GetPos() - nLineBeg >= 10)
			{
				if (nLastBreak > 0)
				{
					assert(nLastBreak <= strIterator.GetPos());
					strIterator.SetPos(nLastBreak);
					++strIterator;
				}
				break;
			}

			unsigned int c = strIterator.CurChar()[0];
			if (c >= 127 || !(IsCharAlphaNumeric(c)
				|| c == '\'' || c == '\"' || c == '_' || c == '-'))
				nLastBreak = strIterator.GetPos();
		}
		if (strIterator.GetPos() == nLineBeg)
			++strIterator;
		str.clear();
		str.append(szText + nLineBeg, strIterator.GetPos() - nLineBeg);
		if (str.size())
		{
			if (str.data()[str.size() - 1] == '\r')
				str.resize(str.size() - 1);
		}
		vStr.push_back(str.c_str());

		if (strIterator.CurChar()[0] == '\n')
			++strIterator;
	}
}

bool WrapDisplayLyrics(CLyricsLines &lyrLinesSrc, CLyricsLines &lyrLinesOut, CRawGraph *pGraph, int nWidthMax)
{
	int		i;

	lyrLinesOut.clear();

	LyricsLine *pRow;
	vector<tstring>	vStr;
	for (i = 0; i < (int)lyrLinesSrc.size(); i++)
	{
		LyricsLine *pRowSrc = lyrLinesSrc[i];

		if (pRowSrc->vFrags.size() > 1 || pRowSrc->vFrags.size() == 0
			|| GetMLRowDrawWidth(pRowSrc, pGraph) <= nWidthMax)
		{
			// Not support vFrags > 1.
			pRow = g_LyricData.CopyMLRow(pRowSrc);
			lyrLinesOut.push_back(pRow);
		}
		else
		{
			// Wrap
			int			nLen, nTimeSpan, nBegTime, nEndTime;

			LyricsPiece *pFragSrc = pRowSrc->vFrags[0];

			vStr.clear();
			WrapWords(pGraph, nWidthMax, pFragSrc->szLyric, vStr);

			nLen = pFragSrc->nLen;
			nTimeSpan = pRowSrc->nEndTime - pRowSrc->nBegTime;

			// ASSERT(vStr.size() > 1);
			nBegTime = pRowSrc->nBegTime;
			for (int k = 0; k < (int)vStr.size(); k++)
			{
				if (nLen > 0)
					nEndTime = nBegTime + (vStr[k].size() * nTimeSpan) / nLen;
				else
					nEndTime = nBegTime + nTimeSpan;
				pRow = NewLyricsLine(nBegTime,  nEndTime, pRowSrc->szContent, -1, pRowSrc->bLyricsLine);

				pRow->AppendPiece(nBegTime, nEndTime, vStr[k].c_str(), vStr[k].size(), false, false);

				lyrLinesOut.push_back(pRow);

				nBegTime = nEndTime;
			}
		}
	}

	return true;
}

// 每次调整歌词的速度的基本单位
const int SET_SPEED_SPAN		= 500;


void AlphaBlendColor(MLColor &clr1, MLColor &clr2, int nAlpha, MLColor &clrOut)
{
	ASSERT(nAlpha >= 0 && nAlpha <= 255);
	int		nPart1 = 255 - nAlpha;

	clrOut.set((clr1.r() * nPart1 + clr2.r() * nAlpha) / 255,
		(clr1.g() * nPart1 + clr2.g() * nAlpha) / 255,
		(clr1.b() * nPart1 + clr2.b() * nAlpha) / 255);
}

void GradientFillImageVert(PRAW_IMAGE_DATA pImage, CRect &rc, MLColor clrGradient[], int nClrCount)
{
	ASSERT(pImage->bitCount == 24 || pImage->bitCount == 32);
	ASSERT(nClrCount >= 2);

	int		nPixCount = 3;
	if (pImage->bitCount == 24)
		nPixCount = 3;
	else if (pImage->bitCount == 32)
		nPixCount = 4;

	if (rc.right > pImage->width)
		rc.right = pImage->width;
	if (rc.bottom > pImage->height)
		rc.bottom = pImage->height;

	float block = (float)rc.Height() / (nClrCount - 1);
	int y = rc.top;
	for (int i = 0; i + 2 <= nClrCount; i++)
	{
		int		r0 = clrGradient[i].r(), g0 = clrGradient[i].g(), b0 = clrGradient[i].b();
		int		r1 = clrGradient[i + 1].r(), g1 = clrGradient[i + 1].g(), b1 = clrGradient[i + 1].b();

		float	dr, dg, db;
		dr = float(r1 - r0) / block;
		dg = float(g1 - g0) / block;
		db = float(b1 - b0) / block;

		float	rN, gN, bN;
		rN = (float)(r0 + 0.5); gN = (float)(g0 + 0.5); bN = (float)(b0 + 0.5);
		uint8	*row = pImage->PixPtr(rc.left, y);
		int yEnd = rc.top + rc.Height() * (i + 1) / (nClrCount - 1);
		for (; y < yEnd; y++)
		{
			uint8	*pixl = row;
			for (int x = rc.left; x < rc.right; x++)
			{
				pixl[0] = (uint8)bN;
				pixl[1] = (uint8)gN;
				pixl[2] = (uint8)rN;
				pixl += nPixCount;
			}

			bN += db; gN += dg; rN += dr;
			row += pImage->stride;
		}
	}
}

void GradientFillImageHorz(PRAW_IMAGE_DATA pImage, CRect &rc, MLColor clrGradient[], int nClrCount)
{
	ASSERT(pImage->bitCount == 24 || pImage->bitCount == 32);
	ASSERT(nClrCount >= 1);

	if (nClrCount == 1)
	{
		FillRect(pImage, rc.left, rc.top, rc.right, rc.bottom, clrGradient[0], BPM_COPY, 255);
		return;
	}

	int		nPixCount = 3;
	if (pImage->bitCount == 24)
		nPixCount = 3;
	else if (pImage->bitCount == 32)
		nPixCount = 4;

	if (rc.right > pImage->width)
		rc.right = pImage->width;
	if (rc.bottom > pImage->height)
		rc.bottom = pImage->height;

	float block = (float)rc.Width() / (nClrCount - 1);
	int x = rc.left;
	for (int i = 0; i + 2 <= nClrCount; i++)
	{
		int		r0 = clrGradient[i].r(), g0 = clrGradient[i].g(), b0 = clrGradient[i].b();
		int		r1 = clrGradient[i + 1].r(), g1 = clrGradient[i + 1].g(), b1 = clrGradient[i + 1].b();

		float	dr, dg, db;
		dr = float(r1 - r0) / block;
		dg = float(g1 - g0) / block;
		db = float(b1 - b0) / block;

		float	rN, gN, bN;
		rN = (float)(r0 + 0.5); gN = (float)(g0 + 0.5); bN = (float)(b0 + 0.5);
		uint8	*col = pImage->PixPtr(x, rc.top);
		int xEnd = rc.left + rc.Width() * (i + 1) / (nClrCount - 1);
		for (; x < xEnd; x++)
		{
			uint8	*pixl = col;
			for (int y = rc.top; y < rc.bottom; y++)
			{
				pixl[0] = (uint8)bN;
				pixl[1] = (uint8)gN;
				pixl[2] = (uint8)rN;
				pixl += pImage->stride;
			}

			bN += db; gN += dg; rN += dr;
			col += nPixCount;
		}
	}
}

void CreateGradientFillImage(CRawImage &image, int nHeight, MLColor clrGradient[3])
{
	// Raw font will add the Margin, we need to add it too.
	nHeight += GfxRaw::MARGIN_BITMAP * 2;

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
		CRect	rc(0, 0, pImage->width, pImage->height);
		GradientFillImageVert(pImage, rc, clrGradient, 3);
		image.Attach(pImage);
	}
}

ID_TO_STRING g_idsLyrDisplayOpt[] = 
{
	{ DO_NORMAL,			_T("normal") },
	{ DO_FADEOUT_LOWCOLOR,	_T("fadein") },
	{ DO_FADEOUT_BG,		_T("fadeout") },
	{ DO_AUTO,				_T("auto") },
};

LPCTSTR DisplayOptToStr(DISPLAY_OPTIONS displayOpt)
{
	return IDToString(g_idsLyrDisplayOpt, displayOpt, g_idsLyrDisplayOpt[DO_AUTO].szId);
}

DISPLAY_OPTIONS DisplayOptFromStr(LPCTSTR szDisplayOpt)
{
	return (DISPLAY_OPTIONS)StringToID(g_idsLyrDisplayOpt, szDisplayOpt, DO_AUTO);
}

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
	_T("FgColor"),
	_T("HilightGradient1"), _T("HilightGradient2"), _T("HilightGradient3"),
	_T("HilightBorderColor"),
	_T("HilightOBM"),
	_T("HilightPattern"),
};

LPCTSTR		_LyrShowTOBFieldNameLowlight[] = 
{
	_T("FgLowColor"),
	_T("LowlightGradient1"), _T("LowlightGradient2"), _T("LowlightGradient3"),
	_T("LowlightBorderColor"),
	_T("LowlightOBM"),
	_T("LowlightPattern"),
};

LPCTSTR GetLyrTOBSettingName(bool bHighlight, CLyricShowObj::TOBColorIndex nameIndex)
{
	ASSERT(CountOf(_LyrShowTOBFieldNameLowlight) == CountOf(_LyrShowTOBFieldNameHilight));
	ASSERT(nameIndex >= 0 && nameIndex < CountOf(_LyrShowTOBFieldNameLowlight));
	if (bHighlight)
		return _LyrShowTOBFieldNameHilight[nameIndex];
	else
		return _LyrShowTOBFieldNameLowlight[nameIndex];
}

tstring GetPatternDir();

PRAW_IMAGE_DATA LoadRawImageDataPatternFile(LPCTSTR szFile)
{
	tstring		strFile = GetPatternDir();
	strFile += szFile;
	if (IsFileExist(strFile.c_str()))
		return LoadRawImageDataFromFile(strFile.c_str());
	else
		return LoadRawImageDataFromFile(szFile);
}

void LoadLyrOverlayBlendingSettings(LPCTSTR szSectName, CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight, bool bHilight)
{
	ASSERT(CountOf(_LyrShowTOBFieldNameLowlight) == CountOf(_LyrShowTOBFieldNameHilight));

	LPCTSTR		*pFieledName;

	if (bHilight)
		pFieledName = _LyrShowTOBFieldNameHilight;
	else
		pFieledName = _LyrShowTOBFieldNameLowlight;

	ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_BORDER], szSectName, pFieledName[CLyricShowObj::TCI_BORDER]);
	ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_FILL], szSectName, pFieledName[CLyricShowObj::TCI_FILL]);

	tob.obm = (OverlayBlendingMode)g_Profile.GetProfileInt(szSectName, GetLyrTOBSettingName(bHilight, CLyricShowObj::TCI_OBM),  OBM_COLOR);
	if (tob.obm == OBM_PATTERN)
	{
		tob.imgPattern.Destroy();

		tob.strPatternFile = g_Profile.GetProfileString(szSectName, bHilight ? _T("HilightPattern") : _T("LowlightPattern"), _T(""));
		if (!tob.strPatternFile.empty())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataPatternFile(tob.strPatternFile.c_str());
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				tob.imgPattern.Attach(pImage);
		}
	}
	else if (tob.obm == OBM_GRADIENT_COLOR)
	{
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT1], szSectName, pFieledName[CLyricShowObj::TCI_GRADIENT1]);
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT2], szSectName, pFieledName[CLyricShowObj::TCI_GRADIENT2]);
		ProfileGetColorValue(tob.clr[CLyricShowObj::TCI_GRADIENT3], szSectName, pFieledName[CLyricShowObj::TCI_GRADIENT3]);

		CreateGradientFillImage(tob.imgPattern, nGradientPatternHeight, tob.clr + CLyricShowObj::TCI_GRADIENT1);
	}
	else
	{
		tob.imgPattern.Destroy();
		tob.obm = OBM_COLOR;
	}

	if (!tob.imgPattern.IsValid())
		tob.obm = OBM_COLOR;
}

void CreateLyrOverlayBlendingPattern(CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight)
{
	ASSERT(nGradientPatternHeight > 0 && nGradientPatternHeight < 300);

	if (tob.obm == OBM_PATTERN)
	{
		tob.imgPattern.Destroy();

		if (!tob.strPatternFile.empty())
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataPatternFile(tob.strPatternFile.c_str());
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				tob.imgPattern.Attach(pImage);
		}
	}
	else if (tob.obm == OBM_GRADIENT_COLOR)
	{
		CreateGradientFillImage(tob.imgPattern, nGradientPatternHeight, tob.clr + CLyricShowObj::TCI_GRADIENT1);
	}
	else
		tob.imgPattern.Destroy();

	if (!tob.imgPattern.IsValid())
		tob.obm = OBM_COLOR;
}

bool ProfileGetLyricsFont(LPCTSTR szSectName, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers)
{
	tstring		strFont;

	strFont = g_Profile.GetProfileString(szSectName, _T("ThemeFont"), _T(""));
	if (strFont.size())
	{
		// Apply ThemeFont to current font setting, then remove it.
		FontPropertyValueFromStr(strFont.c_str(), nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers);

		g_Profile.WriteProfileString(szSectName, _T("ThemeFont"), _T(""));

		ProfileWriteLyricsFont(ET_NULL, szSectName, nHeight, nWeight, byItalic, strFaceNameLatin9.c_str(), strFaceNameOthers.c_str());
	}

#ifdef _WIN32
	strFont = g_Profile.GetProfileString(szSectName, _T("Font"), _T("Verdana, 13, bold, 0, 0, Tahoma"));
#else
	strFont = g_Profile.GetProfileString(szSectName, _T("Font"), _T(", 13, bold, 0, 0, "));
#endif

	return FontPropertyValueFromStr(strFont.c_str(), nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers);
}

void ProfileWriteLyricsFont(EventType etColorTheme, LPCTSTR szSectName, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers)
{
	tstring		strFont;
	tstring		strLatin9, strOthers;

	if (szFaceNameLatin9 == NULL || szFaceNameOthers == NULL || IsEmptyString(szFaceNameLatin9) || IsEmptyString(szFaceNameOthers))
	{
		if ((szFaceNameLatin9 == NULL || IsEmptyString(szFaceNameLatin9)) && (szFaceNameOthers == NULL || IsEmptyString(szFaceNameOthers))
			&& nHeight == -1 && nWeight == -1)
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
	}

	FontPropertyValueToStr(strFont, nHeight, nWeight, byItalic, szFaceNameLatin9, szFaceNameOthers);

	CMPlayerSettings::SetSettings(etColorTheme, szSectName, _T("Font"), strFont.c_str());
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

	m_bCanWrapLines = true;

	m_bEnableAutoResize = true;

	m_bUseSkinStyle = false;

	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

	m_etDispSettings = ET_LYRICS_DISPLAY_SETTINGS;
	m_strSectName = SZ_SECT_LYR_DISPLAY;

	m_font.Create(_T(""), _T(""), 13, FW_NORMAL, 0, false);

	m_nFontHeight = m_font.GetHeight();


	m_nAlignment = LA_CENTER;

	m_LyricsDisplayOpt = DO_AUTO;

	m_hue = 0.0;

	m_clrBg.set(RGB(79, 88, 110));
	m_bSetSkinBg = true;

	m_tobHilight.obm = OBM_COLOR;
	m_tobHilight.clr[TCI_FILL].set(RGB(255, 255, 255));
	m_tobHilight.clr[TCI_BORDER].set(RGB(255, 0, 0));

	m_tobLowlight.obm = OBM_COLOR;
	m_tobLowlight.clr[TCI_FILL].set(RGB(255, 255, 255));
	m_tobLowlight.clr[TCI_BORDER].set(RGB(0, 0, 255));

	m_nLineSpacing = 2;

	m_bOutlineLyrics = false;

	m_bKaraoke = false;

	m_bUseAlbumArtAsBg = false;
	m_bUseBgImg = false;

	m_bDarkenLyrBgOnImg = true;
	m_nDarkenTopArea = float(1.25);
	m_nDarkenBottomArea = float(1.25);
	m_clrDarken.set(RGB(0x18, 0x3C, 0x7B));
	m_clrDarken.setAlpha(128);

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

	m_nLeftAdjust = 0;
	m_nLeftOrg = 0;
}

CLyricShowObj::~CLyricShowObj()
{
	g_Profile.WriteProfileInt(_T("NextPicInFolder"), m_nNextPicInFolder);
	m_pSkin->UnregisterTimerObject(this);
}

void CLyricShowObj::OnCreate()
{
	m_pMLData = &g_LyricData;

	{
		// Is floating lyrics?
		bool		bFloatingLyr = false;
		tstring		strLyrStylePropName;
		CLyricShowAgentObj::GetLyrDispStylePropName(m_pSkin, bFloatingLyr, strLyrStylePropName);
		CMPlayerApp::GetInstance()->GetCurLyrDisplaySettingName(bFloatingLyr, m_strSectName, m_etDispSettings);
	}

	LoadAllSettings();

	RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), m_etDispSettings, ET_LYRICS_CHANGED, ET_LYRICS_DRAW_UPDATE, ET_PLAYER_CUR_MEDIA_CHANGED);
	if (m_bUseBgImg)
		m_pSkin->RegisterTimerObject(this, g_Profile.GetProfileInt(_T("SlideDelayTime"), 10) * 1000);

	// !auto resize || !wrap lines
	if (!m_bEnableAutoResize || !g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndWidth"), false)
		|| !m_bCanWrapLines)
	{
		m_pMLData->CopyMLyrLines(m_lyrLines);
		ReverseLyricsForRightToLeftLanguage();
	}
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

void CLyricShowObj::FastDrawMediaInfo(CRawGraph *pGraph, CRect *prcUpdate)
{
	if (prcUpdate)
	{
		prcUpdate->left = prcUpdate->top = prcUpdate->right = prcUpdate->bottom = 0;
		return;
	}

	UpdateLyricDrawBufferBackground(pGraph, m_rcObj);

	pGraph->SetFont(&m_font);

	int		nLineHeight = (m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS ? m_nFontHeight : GetLineHeight());
	int		y = (m_rcObj.top + m_rcObj.bottom) / 2 - m_lyrLines.size() * nLineHeight / 2;
	int		x;

	if (m_lyrLines.size() > 1)
		x = m_rcObj.left + m_nXMargin;
	else
		x = AUTO_CAL_X;

	if (y < m_rcObj.top + m_nYMargin)
		y = m_rcObj.top + m_nYMargin;

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
		m_rcObj.top + m_nYMargin,
		m_rcObj.right - m_nXMargin,
		m_rcObj.bottom - m_nYMargin);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(rcClip);

	for (size_t i = 0; i < m_lyrLines.size(); i++)
	{
		DrawRow(pGraph, m_lyrLines[i], x, y, LP_CUR_LINE);
		y += nLineHeight;
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

bool CLyricShowObj::DrawRow(CRawGraph *pGraphics, LyricsLine *pLyricRow, int x, int y, LinePos linePos)
{
	int			nSize;
	MLColor		clrTxt, clrTxtBorder;
	int			nAlpha = 255, alphaOld;
	bool		bSetOpacityPainting = false;
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
	else if (m_LyricsDisplayOpt == DO_FADEOUT_BG || m_LyricsDisplayOpt == DO_AUTO)
	{
		if (linePos == LP_ABOVE_CUR_LINE)
		{
			lct = LCT_HIGH_TO_BG;
			nAlpha = GetAlpha(pLyricRow);
			if (nAlpha == 0)
				return false;
			if (m_pSkin->GetEnableTranslucencyLayered() || m_img.IsValid()
					|| m_tobHilight.obm != OBM_COLOR)
			{
				bSetOpacityPainting = true;
				lct = LCT_HIGH;
			}
			else
				nAlpha = 255 - nAlpha;
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
				bSetOpacityPainting = true;
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
		LyricsPiece *pFrag;

		pFrag = pLyricRow->vFrags[i];

/*#ifndef _WIN32_WCE
		if (pLyricRow->IsAdvertise())
		{
			CFontInfo		*pOldFont;

			pOldFont = pGraphics->GetFont();
			ASSERT(pOldFont);
			if (pOldFont)
			{
				CFontInfo		font;
				font.Create(m_font.GetName(), font.GetNameOthers(), m_font.GetHeight(), m_font.GetWeight(), m_font.GetItalic(), true);

				pGraphics->SetFont(&font);

				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, GetLyricFragDrawWidth(pFrag, pGraphics), GetFontHeight());

				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
				x += GetLyricFragDrawWidth(pFrag, pGraphics);

				pGraphics->SetFont(pOldFont);
			}
		}
		else
#endif*/
		{
			if (bSetOpacityPainting)
			{
				alphaOld = pGraphics->GetOpacityPainting();
				pGraphics->SetOpacityPainting(nAlpha);
			}

			if (IsOutlineLyrics())
			{
				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y + GfxRaw::MARGIN_BITMAP, pFrag->szLyric, pFrag->nLen, clrTxt, clrTxtBorder);
			}
			else
			{
				pGraphics->SetTextColor(clrTxt);

				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}

			if (bSetOpacityPainting)
				pGraphics->SetOpacityPainting(alphaOld);

#ifndef _WIN32_WCE
			if (pLyricRow->IsAdvertise())
			{
				if (pLyricRow->szContent)
					SetAdSentence((int)(long)pLyricRow, pLyricRow->szContent, x, y, GetLyricFragDrawWidth(pFrag, pGraphics), GetFontHeight());
			}
#endif

			x += GetLyricFragDrawWidth(pFrag, pGraphics);
		}
	}

	return true;
}

void CLyricShowObj::DrawRow(CRawGraph *pGraphics, LyricsLine *pLyricRow, int x, int y, MLColor &clrTxt, MLColor &clrTxtBorder)
{
	int			nSize;

	if (x == AUTO_CAL_X)
		x = GetLyricRowAlignPos(pGraphics, pLyricRow);

	// 画出此行歌词
	nSize = pLyricRow->vFrags.size();
	for (int i = 0; i < nSize; i++)
	{
		LyricsPiece *pFrag;

		pFrag = pLyricRow->vFrags[i];

/*#ifndef _WIN32_WCE
		if (pLyricRow->IsAdvertise())
		{
			CFontInfo		*pOldFont;

			pOldFont = pGraphics->GetFont();
			ASSERT(pOldFont);
			if (pOldFont)
			{
				CFontInfo		font;
				font.Create(m_font.GetName(), font.GetNameOthers(), m_font.GetHeight(), m_font.GetWeight(), m_font.GetItalic(), true);

				pGraphics->SetFont(&font);

				if (pLyricRow->szContent)
					SetAdSentence(int(pLyricRow), pLyricRow->szContent, x, y, GetLyricFragDrawWidth(pFrag, pGraphics), GetFontHeight());

				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
				x += GetLyricFragDrawWidth(pFrag, pGraphics);

				pGraphics->SetFont(pOldFont);
			}
		}
		else
#endif*/
		{
			if (IsOutlineLyrics())
			{
				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y + GfxRaw::MARGIN_BITMAP, pFrag->szLyric, pFrag->nLen, clrTxt, clrTxtBorder);
			}
			else
			{
				pGraphics->SetTextColor(clrTxt);
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}
#ifndef _WIN32_WCE
			if (pLyricRow->IsAdvertise())
			{
				if (pLyricRow->szContent)
					SetAdSentence((int)(long)pLyricRow, pLyricRow->szContent, x, y, GetLyricFragDrawWidth(pFrag, pGraphics), GetFontHeight());
			}
#endif
			x += GetLyricFragDrawWidth(pFrag, pGraphics);
		}
	}
}

// COMMENT:
//		逐字显示歌词, like Karaoke
void CLyricShowObj::DrawRowKaraoke(CRawGraph	*pGraphics, LyricsLine *pLyricRow, int x, int y)
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
	LyricsPiece *pFrag;
	int			nSize;
	int			nCurPos;

	if (pLyricRow->IsAdvertise())
	{
		DrawRow(pGraphics, pLyricRow, x, y, LP_CUR_LINE);
		return;
	}

	if (pLyricRow->nBegTime >= m_pMLData->GetPlayElapsedTime())
	{
		DrawRow(pGraphics, pLyricRow, x, y, LP_BELOW_CUR_LINE);
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

			if (m_tobHilight.obm == OBM_COLOR)
				m_font.UseColorOverlay();
			else
				m_font.SetOverlayPattern(&m_tobHilight.imgPattern);

			if (IsOutlineLyrics())
			{
				// Draw lyrics text outlined
				pGraphics->TextOutOutlined(x, y + GfxRaw::MARGIN_BITMAP, pFrag->szLyric, pFrag->nLen, m_tobHilight.clr[TCI_FILL], m_tobHilight.clr[TCI_BORDER]);
			}
			else
			{
				pGraphics->SetTextColor(GetHighlightColor());
				pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
			}
			x += GetLyricFragDrawWidth(pFrag, pGraphics);;
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
		nFragDrawWidth = GetLyricFragDrawWidth(pFrag, pGraphics);

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

			if (m_tobLowlight.obm == OBM_COLOR)
				m_font.UseColorOverlay();
			else
				m_font.SetOverlayPattern(&m_tobLowlight.imgPattern);

			if (IsOutlineLyrics())
			{
				pGraphics->DrawTextClipOutlined(pFrag->szLyric, pFrag->nLen, &rcText, m_tobLowlight.clr[TCI_FILL], m_tobLowlight.clr[TCI_BORDER], nKaraokePos);
			}
			else
			{
				pGraphics->SetTextColor(GetLowlightColor());
				pGraphics->DrawTextClip(pFrag->szLyric, pFrag->nLen, &rcText, nKaraokePos);
			}
		}

		// Draw hilight part lyrics
		rcText.SetRect(x, y, x + nKaraokePos, y + GetFontHeight());

		if (m_tobHilight.obm == OBM_COLOR)
			m_font.UseColorOverlay();
		else
			m_font.SetOverlayPattern(&m_tobHilight.imgPattern);

		if (IsOutlineLyrics())
		{
			// Draw lyrics text outlined
			pGraphics->DrawTextClipOutlined(pFrag->szLyric, pFrag->nLen, &rcText, m_tobHilight.clr[TCI_FILL], m_tobHilight.clr[TCI_BORDER]);
		}
		else
		{
			pGraphics->SetTextColor(GetHighlightColor());
			pGraphics->DrawTextClip(pFrag->szLyric, pFrag->nLen, &rcText);
		}

		// 增加 x 的偏移位置
		x += nFragDrawWidth;
		nCurPos++;
	}

	if (m_tobLowlight.obm == OBM_COLOR)
		m_font.UseColorOverlay();
	else
		m_font.SetOverlayPattern(&m_tobLowlight.imgPattern);

	// 显示时间未到的歌词段
	for (; nCurPos < nSize; nCurPos++)
	{
		pFrag = pLyricRow->vFrags[nCurPos];

		// draw text border
		if (IsOutlineLyrics())
		{
			// Draw lyrics text outlined
			pGraphics->TextOutOutlined(x, y + GfxRaw::MARGIN_BITMAP, pFrag->szLyric, pFrag->nLen, m_tobLowlight.clr[TCI_FILL], m_tobLowlight.clr[TCI_BORDER]);
		}
		else
		{
			pGraphics->SetTextColor(GetLowlightColor());
			pGraphics->TextOut(x, y, pFrag->szLyric, pFrag->nLen);
		}
		x += GetLyricFragDrawWidth(pFrag, pGraphics);
	}
	// 5. ok
}

void CLyricShowObj::DrawCurrentRow(CRawGraph	*pGraphics, LyricsLine *pLyricRow, int x, int y)
{
	assert(m_pMLData);
	assert(pGraphics);

	if (IsKaraoke())
		DrawRowKaraoke(pGraphics, pLyricRow, x, y);
	else
		DrawRow(pGraphics, pLyricRow, x, y, LP_CUR_LINE);
}

void CLyricShowObj::FadeOutVertBorder(CRawGraph *pGraphics, int yDrawLyrStartPos, int yDrawLyrEndPos)
{
#ifdef _WIN32_DESKTOP
	if (m_pSkin->m_nCurTranslucencyAlpha > 0)
	{
		if (m_bSetSkinBg && m_pContainer->IsDrawBgImage())
			return;

		if (m_img.IsValid())
			return;
	}

	MLColor clrBg;
	if (m_bSetSkinBg)
		clrBg = m_pContainer->GetBgColor();
	else
		clrBg = m_clrBg;

	CRect	rc;
	rc.SetRect(m_rcObj.left, m_rcObj.top + m_nYMargin, m_rcObj.right, m_rcObj.top + m_nYMargin + m_nFontHeight);
	if (rc.bottom > yDrawLyrStartPos)
	{
		if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nCurTranslucencyAlpha <= 0)
			pGraphics->VertAlphaFadeOut(&rc, true);
		else
			pGraphics->VertFadeOut(&rc, clrBg, true);
	}

	rc.bottom = m_rcObj.bottom - m_nYMargin;
	rc.top = m_rcObj.bottom - m_nYMargin - m_nFontHeight;
	if (rc.top < yDrawLyrEndPos)
	{
		if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nCurTranslucencyAlpha <= 0)
			pGraphics->VertAlphaFadeOut(&rc, false);
		else
			pGraphics->VertFadeOut(&rc, clrBg, false);
	}
#endif
}

// COMMENT:
//		根据当前的 Graphics 环境，取得歌词pLyricRow行的宽度
int CLyricShowObj::GetLyricRowTextWidth(CRawGraph	*pGraphics, LyricsLine *pLyricRow)
{
	assert(m_pMLData);

	int			nTxtWidth = 0;
	int			nSize;
	LyricsPiece *pFrag;

	nSize = pLyricRow->vFrags.size();
	for (int i = 0; i < nSize; i++)
	{
		pFrag = pLyricRow->vFrags[i];

		nTxtWidth += GetLyricFragDrawWidth(pFrag, pGraphics);
	}
	
	return nTxtWidth;
}

void CLyricShowObj::UpdateLyricDrawBufferBackground(CRawGraph *pGraphics, CRect &rc)
{
	if (m_img.IsValid())
	{
		m_img.TileBltEx(pGraphics, m_rcObj.left, m_rcObj.top, rc.left, rc.top, rc.Width(), rc.Height());
		// m_img.Blt(pGraphics, m_rcObj.left, m_rcObj.top);

		if (m_bDarkenLyrBgOnImg)
			DarkenLyricsBg(pGraphics, rc);
	}
	else if (m_bSetSkinBg)
	{
		ASSERT(rc.bottom <= m_rcObj.bottom);
		m_pContainer->RedrawBackground(pGraphics, rc);
		// pGraphics->FillRect(rc.left, rc.top, rc.Width(), rc.Height(), &m_brBg);
	}
	else
		pGraphics->FillRect(&rc, m_clrBg);

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
	nFillBottom = min((long)rc.bottom, (long)nDarkenBottom);
	nFillTop = max((long)rc.top, (long)nDarkenTop);
	if (nFillBottom > nFillTop)
		pGraphics->FillRect(rc.left, nFillTop, rc.Width(), nFillBottom - nFillTop, m_clrDarken, BPM_BLEND);
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
int CLyricShowObj::GetLyricRowAlignPos(CRawGraph	*pGraphics, LyricsLine *pLyricRow)
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
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
		break;
	case VK_DOWN:
		m_pMLData->SetOffsetTime(m_pMLData->GetOffsetTime() + SET_SPEED_SPAN);
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
		break;
	case 'D':
		m_pSkin->PostShortcutKeyCmd(CMD_OPEN_LRC);
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
bool CLyricShowObj::OnMouseMove(CPoint point)
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
					CMLMenu	menu;
					menu.LoadPopupMenu(IDR_MENU_RATE, 0);

					CPoint	pt;
					MLWidget::GetCursorPos(&pt);
					menu.TrackPopupMenu(pt.x, pt.y, m_pSkin);
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

void CLyricShowObj::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == MK_CONTROL)
	{
		m_pSkin->PostShortcutKeyCmd(nWheelDistance > 0 ? CMD_FONT_SIZE_INC : CMD_FONT_SIZE_DEC);
	}
}

#endif	// _WIN32_WCE

void CLyricShowObj::OnLyricsChanged()
{
	if ((m_bEnableAutoResize && g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndWidth"), false))
		|| !m_bCanWrapLines)
	{
		// auto resize || !wrap lines
		m_pMLData->CopyMLyrLines(m_lyrLines);
		ReverseLyricsForRightToLeftLanguage();
	}
	else
		WrapLyricsLines();

	OnLyrDrawContextChanged();

	Invalidate();
}

void CLyricShowObj::OnLyrDrawContextChanged()
{
	m_lyrLines.ClearDrawContextWidth();

	if (m_bCanWrapLines)
	{
		if (!AutoWidthSkinAccordLyrics())
			WrapLyricsLines();
	}

	// Auto height.
	AutoHeightSkinAccordLyrics(GetAutoHeightLines());
}

void AdjustColorHue(MLColor &clr, float hueOffset)
{
	COLORREF	c = clr.get();
	AdjustColorHue(c, hueOffset);
	clr.set(c);
}

void CLyricShowObj::OnAdjustHue(float hue, float saturation, float luminance)
{
	if (m_hue != 0)
	{
		// Load original settings.
		SetBgColor(g_Profile.GetProfileString(m_strSectName.c_str(), _T("BgColor"), _T("")));

		LoadLyrOverlaySettings();
	}

	m_hue = hue;

	if (hue == 0)
		return;

	// Adjust hue
	AdjustColorHue(m_clrBg, hue);

	tstring		strColor;
	ColorToStr(m_clrBg, strColor);
	if (m_pSkin && m_bSetSkinBg)
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

void CLyricShowObj::WrapLyricsLines()
{
	CRawGraph		*pGraphics;

	pGraphics = m_pContainer->GetMemGraph();

	pGraphics->SetFont(&m_font);

	WrapDisplayLyrics(g_LyricData.GetRawLyrics(), m_lyrLines, pGraphics, m_rcObj.Width() - m_nXMargin * 2);

	ReverseLyricsForRightToLeftLanguage();
}

// COMMENT:
// 自动调整迷窗口高度
void CLyricShowObj::AutoHeightSkinAccordLyrics(int nLines)
{
#ifndef _MPLAYER
	if (!m_bEnableAutoResize || nLines <= 0)
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

	m_pSkin->GetWindowRect(rc);
	nHeightOthers = rc.Height() - m_rcObj.Height();
	nBottomOld = rc.bottom;

	rc.bottom = rc.top + nHeightOthers + m_nYMargin * 2 + abs(GetLineHeight() * nLines) + m_nLineSpacing * (nLines - 1);
	if (nBottomOld != rc.bottom)
		m_pSkin->MoveWindow(rc);
#endif
}

// COMMENT:
//		根据歌词的长度，自动调整窗口宽度可以匹配显示歌词
bool CLyricShowObj::AutoWidthSkinAccordLyrics()
{
#ifdef _WIN32_WCE
	return false;
#else
	CRect		rc, rcOrg;
	int			nWidthMax = 200;
	int			nWPrev = 0;

	if (!m_pMLData || !m_bEnableAutoResize || m_pSkin->m_wndResizer.IsFixedWidth())
		return false;

	if (m_lyrLines.empty())
		return false;

	if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoAdjustWndWidth"), false))
		return false;

#ifdef _WIN32
	// auto size feature is disabled when the skin is embedded into other window
	if (::GetParent(m_pSkin->GetHandle()) != NULL)
		return false;
#endif

	CRawGraph		*pGraphics;

	pGraphics = m_pContainer->GetMemGraph();

	pGraphics->SetFont(&m_font);

	//
	// 找到最宽的一行歌词
	m_pSkin->GetWindowRect(&rc);
	rcOrg = rc;
	for (int i = 0; i < (int)m_lyrLines.size(); i++)
	{
		int		w = GetLyricRowTextWidth(pGraphics, m_lyrLines[i]);
		if (w > nWidthMax)
			nWidthMax = w;
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
			return true;

		m_pSkin->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
	}
	return true;
#endif // #ifndef _WIN32_WCE
}

void CLyricShowObj::OnPlayTimeChangedUpdate()
{
	CRect	rcUpdate;
	CRawGraph		*pmemGraphics;

	if (m_pSkin->IsIconic() || !IsVisible() || !IsParentVisible())
		return;

	pmemGraphics = m_pContainer->GetMemGraph();

	pmemGraphics->SetFont(&m_font);

	FastDraw(pmemGraphics, &rcUpdate);

	if (!rcUpdate.IsRectEmpty())
		m_pContainer->UpdateMemGraphicsToScreen(&rcUpdate, this);
}

int CLyricShowObj::GetAlpha(LyricsLine *pRow)
{
	int				nTime;
	const int		FADE_DURATION = 1500;

	if (pRow->nBegTime > m_pMLData->GetPlayElapsedTime())
	{
		nTime = pRow->nBegTime - m_pMLData->GetPlayElapsedTime();
		if (nTime <= FADE_DURATION && nTime >= -FADE_DURATION)
		{
			return (FADE_DURATION - nTime) * 255 / FADE_DURATION;
		}
		else
			return 0;
	}
	else
	{
		nTime = m_pMLData->GetPlayElapsedTime() - pRow->nEndTime;
		if (nTime >= FADE_DURATION)
			return 0;
		else if (nTime > 0)
			return (FADE_DURATION - nTime) * 255 / FADE_DURATION;
		else
			return 255;
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
		PRAW_IMAGE_DATA	hImg = AutoFitImageToRect(hBmp, m_rcObj.Width(), m_rcObj.Height(), hBmp->bitCount);
		if (hImg)
		{
			m_img.Attach(hImg);
			m_img.GetOrginalSize(m_img.m_cx, m_img.m_cy);
		}
		FreeRawImage(hBmp);
	}
}


void CLyricShowObj::OnTimer(int nId)
{
	LoadNextBgImage();
	Invalidate();
}

void CLyricShowObj::OnSize()
{
	CUIObject::OnSize();

	if (m_bUseBgImg || m_bUseAlbumArtAsBg)
		LoadNextBgImage();

	if (m_bCanWrapLines)
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
	{
		m_LyricsDisplayOpt = DisplayOptFromStr(szValue);
	}
	else if (_tcsicmp(szProperty, _T("BgColor")) == 0)
		SetBgColor(szValue);
	else if (_tcsicmp(szProperty, _T("SetSkinBg")) == 0)
		m_bSetSkinBg = IsTRUE(szValue);
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
		m_nFontHeight = m_font.GetHeight();

		if (m_tobHilight.imgPattern.height() != GetPatternFontHeight())
		{
			if (m_hue == 0)
				LoadLyrOverlaySettings();
			else
				OnAdjustHue(m_hue, 0, 0);
		}

		OnLyrDrawContextChanged();
	}
	else if (IsPropertyName(szProperty, _T("OutlineLyrText")))
	{
		m_bOutlineLyrics = IsTRUE(szValue);
	}
	else if (IsPropertyName(szProperty, _T("Karaoke")))
		m_bKaraoke = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("EnableAutoResize")) == 0)
		m_bEnableAutoResize = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableAdjustVertAlign")))
		m_bEnableAdjustVertAlign = IsTRUE(szValue);
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
				break;
			}
			else if (IsPropertyName(szProperty, _LyrShowTOBFieldNameLowlight[i]))
			{
				GetColorValue(m_tobLowlight.clr[i], szValue);
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

	if (IsPropertyName(szProperty, _T("BgColor")))
	{
		SetBgColor(szValue);
		m_pSkin->InvalidateRect();
		return true;
	}
	else if (IsPropertyName(szProperty, _T("OutlineLyrText")))
	{
		m_bOutlineLyrics = IsTRUE(szValue);
	}
	//
	// High light of lyrics overlay blending settings
	//
	else if (IsPropertyName(szProperty, _T("HilightOBM")))
	{
		LoadLOBHilightSettings();
	}
	else if (IsPropertyName(szProperty, _T("HilightPattern")))
	{
		if (m_tobHilight.obm == OBM_PATTERN)
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataPatternFile(szValue);
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				m_tobHilight.imgPattern.Attach(pImage);
		}
	}
	//
	// Low light of lyrics overlay blending settings
	//
	else if (IsPropertyName(szProperty, _T("LowlightOBM")))
	{
		LoadLOBLowlightSettings();
	}
	else if (IsPropertyName(szProperty, _T("LowlightPattern")))
	{
		if (m_tobLowlight.obm == OBM_PATTERN)
		{
			PRAW_IMAGE_DATA		pImage = LoadRawImageDataPatternFile(szValue);
			if (pImage && pImage->bitCount != 24)
				pImage = ConvertTo24BppRawImage(pImage);

			if (pImage)
				m_tobLowlight.imgPattern.Attach(pImage);
		}
	}
	else if (IsPropertyName(szProperty, _T("EnableAdjustVertAlign")))
	{
		m_bEnableAdjustVertAlignUser = IsTRUE(szValue);
	}
	else
	{
		TOBColorIndex	i;

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
		if ((i == TCI_GRADIENT1 || i == TCI_GRADIENT2 || i == TCI_GRADIENT3))
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
			if ((i == TCI_GRADIENT1 || i == TCI_GRADIENT2 || i == TCI_GRADIENT3))
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

void CLyricShowObj::SetBgColor(LPCTSTR szValue)
{
	if (IsEmptyString(szValue))
		return;

	GetColorValue(m_clrBg, szValue);

	if (m_pSkin && m_bSetSkinBg)
		m_pSkin->SetProperty(_T("BgColor"), szValue);
}

void CLyricShowObj::LoadLyrOverlaySettings()
{
	m_bOutlineLyrics = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("OutlineLyrText"), m_bOutlineLyrics);

	ProfileGetColorValue(m_tobHilight.clr[TCI_BORDER], m_strSectName.c_str(), _T("HilightBorderColor"));
	ProfileGetColorValue(m_tobLowlight.clr[TCI_BORDER], m_strSectName.c_str(), _T("LowlightBorderColor"));

	LoadLOBHilightSettings();
	LoadLOBLowlightSettings();
}

void CLyricShowObj::LoadLOBHilightSettings()
{
	LoadLyrOverlayBlendingSettings(m_strSectName.c_str(), m_tobHilight, GetPatternFontHeight(), true);
}

void CLyricShowObj::LoadLOBLowlightSettings()
{
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

	SetBgColor(g_Profile.GetProfileString(m_strSectName.c_str(), _T("BgColor"), _T("")));

	LoadLyrOverlaySettings();

	SetLyrAlign(g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrAlign"), _T("")));

	m_LyricsDisplayOpt = DisplayOptFromStr(g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrDrawOpt"), _T("")));

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
		m_strBgPicFolder = CMLProfile::GetDir(m_strSectName.c_str(), _T("BgPicFolder"), _T(""));

		m_bDarkenLyrBgOnImg = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("DarkenLyrBg"), true);

		if (m_bUseBgImg)
		{
			LoadBgImageFolder();
			m_nNextPicInFolder = g_Profile.GetProfileInt(m_strSectName.c_str(), _T("NextPicInFolder"), 0);
		}

		LoadLyrOverlaySettings();
	}
}

bool inline isArabic(WCHAR c)
{
	return (c >= 0x590 && c <= 0x6ff);// || (c >= 0x0E00 && c <= 0x0E7F);
}

bool inline isStrArabic(LPCTSTR str)
{
	while (*str)
	{
		if (isArabic(*str))
			return true;
		str++;
	}

	return false;
}

void strreverse(LPTSTR str, int len)
{
	for (int i = len / 2; i >= 0; i--)
	{
		TCHAR c = str[len - 1 - i];
		str[len - 1 - i] = str[i];
		str[i] = c;
	}
}

inline bool IsNoneAlpha(TCHAR c)
{
	return (uint)c < 0x7F && !IsAlpha(c) && !IsDigit(c);
}

void ReverseStringForRightToLeftLanguage(LPTSTR szText)
{
	LPCTSTR		p = szText;
	LPCTSTR		szStart = szText;
	LPCTSTR		szNoneAlpha = NULL;

	tstring		str;

	while (*p)
	{
		szNoneAlpha = NULL;
		while (*p && !isArabic(*p))
		{
			if (IsNoneAlpha(*p))
			{
				if (szNoneAlpha == NULL)
					szNoneAlpha = p;
			}
			else
				szNoneAlpha = NULL;
			p++;
		}

		if (*p && szNoneAlpha != NULL)
		{
			// Reverse none alpha char
			strreverse((LPTSTR)szNoneAlpha, int(p - szNoneAlpha));

			// Insert alpha str
			str.insert(0, szStart, int(szNoneAlpha - szStart));

			// Insert none alpha
			str.insert(0, szNoneAlpha, int(p - szNoneAlpha));
		}
		else
			str.insert(0, szStart, int(p - szStart));
		szStart = p;

		if (!*p)
			break;

		while (isArabic(*p))
			p++;

		str.insert(0, szStart, int(p - szStart));
		szStart = p;

		while (*p && IsNoneAlpha(*p) && !isArabic(*p))
			p++;

		if (p != szStart)
		{
			// Reverse none alpha char
			strreverse((LPTSTR)szStart, int(p - szStart));

			// Insert none alpha
			str.insert(0, szStart, int(p - szStart));
			szStart = p;
		}
	}

	ASSERT(str.size() == _tcslen(szText));
	_tcscpy(szText, str.c_str());
}

void CLyricShowObj::ReverseLyricsForRightToLeftLanguage()
{
	for (int i = 0; i < (int)m_lyrLines.size(); i++)
	{
		LyricsLine *pRow = m_lyrLines[i];
		for (int k = 0; k < (int)pRow->vFrags.size(); k++)
		{
			LyricsPiece *pFrag = pRow->vFrags[k];

			if (isStrArabic(pFrag->szLyric))
				ReverseStringForRightToLeftLanguage(pFrag->szLyric);
		}
	}
}
