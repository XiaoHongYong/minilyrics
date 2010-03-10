/********************************************************************
	Created  :	2002/01/04	21:40
	FileName :	LyricShowObj.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_LYRICSHOWOBJ_H__9DAA20E0_7727_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSHOWOBJ_H__9DAA20E0_7727_11D5_9E04_02608CAD9330__INCLUDED_

#pragma once


enum LyrAlignment
{
	LA_LEFT			= 0,
	LA_CENTER		= 1,
	LA_RIGHT		= 2,
	LA_MAX			= 3,
};

enum DISPLAY_OPTIONS
{
	DO_NORMAL = 0,				// 普通显示选项
	DO_FADEOUT_LOWCOLOR,		// 淡出为低调色显示
	DO_FADEOUT_BG,				// 淡出为背景色显示
	DO_MAX
};

enum OverlayBlendingMode
{
	OBM_UNKNOWN,
	OBM_COLOR,				// 1 pure color
	OBM_GRADIENT_COLOR,		// 3 colors
	OBM_PATTERN,			// picture pattern
};

extern ID_TO_STRING g_idsLyrDisplayOpt[];

LPCTSTR DisplayOptToStr(DISPLAY_OPTIONS displayOpt);

DISPLAY_OPTIONS DisplayOptFromStr(LPCTSTR szDisplayOpt);

#define AUTO_CAL_X				0xFFFFFF

bool ProfileGetLyricsFont(LPCTSTR szSectName, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers);

void ProfileWriteLyricsFont(EventType etColorTheme, LPCTSTR szSectName, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers);


#define LS_AD_DRAW_INIT			CLyricShowObj::Draw(pGraph)


class CLyricShowObj : public CUIObject, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	enum LinePos
	{
		LP_ABOVE_CUR_LINE,
		LP_CUR_LINE,
		LP_BELOW_CUR_LINE,
	};

	enum VertLineAlign
	{
		VLA_TOP,
		VLA_CENTER,
		VLA_BOTTOM,
	};

public:
	CLyricShowObj();
	virtual ~CLyricShowObj();

	class CDispOptRecover
	{
	public:
		CDispOptRecover(CLyricShowObj *pObj) { m_pObj = pObj; m_dispOpt = pObj->m_LyricsDisplayOpt; }
		~CDispOptRecover() { m_pObj->m_LyricsDisplayOpt = m_dispOpt; }

		DISPLAY_OPTIONS			m_dispOpt;
		CLyricShowObj			*m_pObj;

	};
	friend class CDispOptRecover;

	class MLColorEx : public MLColor
	{
	public:
		MLColorEx() : m_bValid(false)
		{
		}
		MLColorEx(MLColor &clr) : MLColor(clr), m_bValid(true)
		{
		}
		bool		m_bValid;

	};

	// TOB = Text Overlay Blending
	// TCI = Text Color Index
	enum TOBColorIndex
	{
		TCI_START					= 0,
		TCI_FILL					= TCI_START,
		TCI_GRADIENT1,
		TCI_GRADIENT2,
		TCI_GRADIENT3,
		TCI_BORDER,
		TCI_COUNT,
	};

	struct TextOverlayBlending
	{
		TextOverlayBlending() : obm(OBM_UNKNOWN) { }

		OverlayBlendingMode		obm;				// Overlay blending mode
		MLColorEx				clr[TCI_COUNT];		// Colors, Accessed by TOBColorIndex
		CRawImage				imgPattern;
		tstring					strPatternFile;
	};

public:
	virtual void OnCreate();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	virtual void OnKeyDown(UINT nChar, UINT nFlags);

	void Draw(CRawGraph *pGraph);

	// 快速绘画
	// OUTPUT:
	//		rcUpdate	-	更新的矩形区域
	virtual void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);

#ifndef _WIN32_WCE
	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
#endif

	virtual void OnPlayTimeChangedUpdate();

	virtual void OnLyricsChanged();

	virtual void OnSaveLyrics() { };

	virtual void SetAntialise(BOOL bAntialis);

	int GetAlpha(PML_ROW pRow);

	void SetLyricData(CMLData *pMLData);

	bool IsKaraoke() { return m_bKaraoke || m_pMLData->IsKaraokeShowStyle(); }

	//
	// Set setting values
	//
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	virtual bool OnLyrDisplaySettings(LPCTSTR szProperty, LPCTSTR szValue);

	void UpdateBgImage();

	void OnSize(int cx, int cy);

	virtual void OnLyrDrawContextChanged();
	virtual int GetAutoHeightLines() { return 0; }

	void OnAdjustHue(float hue, float saturation, float luminance);

	void WrapLyricsLines();

	void AutoHeightSkinAccordLyrics(int nLines = 1);
	bool AutoWidthSkinAccordLyrics();
	void UpdateLyricDrawBufferBackground(CRawGraph *pGraphics, CRect &rc);

	void DarkenLyricsBg(CRawGraph *pGraphics, CRect &rc);

	int GetLyricRowAlignPos(CRawGraph	*pGraphics, PML_ROW pLyricRow);
	int GetLyricRowTextWidth(CRawGraph	*pGraphics, PML_ROW pLyricRow);

	bool DrawRow(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y, LinePos linePos = LP_ABOVE_CUR_LINE);
	void DrawRow(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y, MLColor &clrTxt, MLColor &clrTxtBorder);

	void DrawRowKaraoke(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y);

	void DrawCurrentRow(CRawGraph *pGraphics, PML_ROW pLyricRow, int x, int y);

	void LoadBgImageFolder();

	void LoadNextBgImage();

	void OnTimer(int nId);

	int GetFontHeight() const { return m_nFontHeight + (m_bOutlineLyrics ? 2 : 0); }

	int GetPatternFontHeight() const { return m_nFontHeight + GfxRaw::MARGIN_BITMAP; }

	int GetLineHeight() const { return m_nFontHeight + (m_bOutlineLyrics ? 2 : 0) + m_nLineSpacing; }

	inline MLColor &GetHighlightColor() { return m_tobHilight.clr[TCI_FILL]; }
	inline MLColor &GetLowlightColor() { return m_tobLowlight.clr[TCI_FILL]; }

	virtual void ReloadThemeOfSkin(bool bSavetoProfile);

	void SetLyrAlign(LPCTSTR szValue);
	void SetBgColor(LPCTSTR szValue);

	void LoadBgImageSettings();

	void LoadLyrOverlaySettings();

	void LoadLOBHilightSettings();
	void LoadLOBLowlightSettings();

	void LoadAllSettings();

	void ReverseLyricsForRightToLeftLanguage();

protected:
	// 边界
	int						m_nXMargin, m_nYMargin;

	//
	// 歌词字体
	//
	CRawBmpFont				m_font;
	int						m_nFontHeight;

	// hue of lyrics
	float					m_hue;

	bool					m_bOutlineLyrics;

	// basic colors
	MLColor					m_clrBg;
	MLColor					m_clrTag, m_clrFocusLineBg;

	//
	// blending lyrics options:
	//
	TextOverlayBlending		m_tobHilight;
	TextOverlayBlending		m_tobLowlight;

	//
	// Original Properties of skin theme
	//

	// The following theme available?
	MLColorEx				m_skin_clrFg, m_skin_clrFgLow, m_skin_clrBg;
	MLColorEx				m_skin_clrTag, m_skin_clrFocusLineBg;
	bool					m_skin_bOutlineLyrics;
	TextOverlayBlending		m_skin_tobHilight;
	TextOverlayBlending		m_skin_tobLowlight;
	int						m_skin_fontHeight, m_skin_fontWeight;
	DISPLAY_OPTIONS			m_skin_do;


	// TRUE: NEVER load customized settings.
	bool					m_bUseSkinStyle;


	//
	// 显示选项: KARAOKE, 淡出为背景、淡出为低调色
	//
	DISPLAY_OPTIONS			m_LyricsDisplayOpt;

	bool					m_bKaraoke;

	//
	// Use back ground image?
	//
	bool					m_bUseAlbumArtAsBg;
	bool					m_bUseBgImg;

	CCurMediaAlbumArt		m_curAlbumArt;
	tstring					m_strBgPicFolder;
	V_Str					m_vPicFiles;
	int						m_nNextPic;
	int						m_nNextPicInFolder;
	CSFImage				m_img;
	bool					m_bDarkenLyrBgOnImg;
	float					m_nDarkenTopArea, m_nDarkenBottomArea;
	MLColor					m_clrDarken;

	// Horizontal lyrics alignment
	LyrAlignment			m_nAlignment;

	// 行间距			
	int						m_nLineSpacing;

	bool					m_bEnableAutoResize;

#ifndef _WIN32_WCE
	//
	// Advertisement
	//
	struct AdSentence
	{
		int			nID;
		CRect		rcAdWords;
		tstring		strLink;
	};
	typedef vector<AdSentence>		V_ADS;

	void SetAdSentence(int nID, LPCTSTR szLink, int x, int y, int nWidth, int nHeight);

	V_ADS				m_vads;
	CCursor				m_Cursor;

	//
	// Lyrics vertical align position
	//
	CCursor					m_curAdjustVertAlign;
	bool					m_bEnableAdjustVertAlign, m_bEnableAdjustVertAlignUser;

	// Vertical lyrics line alignment
	VertLineAlign			m_vertLineAlign;
	int						m_nVertLineOffset;
	int						m_yBeginDrag;

	int GetLineVertAlignPos();
public:
	void EnableAdjustVertAlign(bool bEnableAdjustVertAlign) { m_bEnableAdjustVertAlign = bEnableAdjustVertAlign; }
#else // _WIN32_WCE
	int GetLineVertAlignPos() { return m_rcObj.top + m_rcObj.Height() / 2; }
#endif // _WIN32_WCE

protected:
	// Used to load and save current settings in profile
	tstring				m_strSectName;

	// Event type to send when settings was changed.
	EventType			m_etDispSettings;

	// 歌词数据
	CMLData				*m_pMLData;
	CMLyrLines			m_lyrLines;

	bool				m_bCanWrapLines;

	// used for auto adjust width with lyrics
	int					m_nLeftOrg, m_nLeftAdjust;

};

void CreateGradientFillImage24bpp(CRawImage &image, int nHeight, CLyricShowObj::MLColorEx clrGradient[3]);

void ProfileWriteTOBToOld(bool bOutlineLyrText, 
						  const CLyricShowObj::TextOverlayBlending &tobHilight, 
						  const CLyricShowObj::TextOverlayBlending &tobLowlight);

void ProfileWriteTOBCustomizedToCurrent();

#endif // !defined(AFX_LYRICSHOWOBJ_H__9DAA20E0_7727_11D5_9E04_02608CAD9330__INCLUDED_)
