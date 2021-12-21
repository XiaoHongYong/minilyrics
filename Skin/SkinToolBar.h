// SkinToolBar.h: interface for the CSkinToolbar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINTOOLBAR_H__0DEC93D1_44E6_4ABE_BEF8_BC7EA2CC7576__INCLUDED_)
#define AFX_SKINTOOLBAR_H__0DEC93D1_44E6_4ABE_BEF8_BC7EA2CC7576__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinToolbar : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	struct Button
	{
		Button();

		enum {
			STATUS_NORMAL		= 0,
			STATUS_CHECKED		= 1,
			STATUS_MAX			= 4
		};

		int			nID;
		int			nAutoUniID;
		int16		nWidth;
		int16		nWidthImage;
		bool		bSeperator;

		int8		nGroup;

		tstring		strText, strTextEnglish;

		uint8		nCurStatus;
		uint8		nStatusMax;
		int16		nLeftPositon[STATUS_MAX + 1];

		bool		bEnabled;
		int			nLeftOfDiabled;		// Image position of disabled

		tstring		strTooltip;
		bool		bTempToolTip;
		bool		bContinuousCmd;		// Will the cmd be triggered continuous?

		void FromXML(CSkinToolbar *pToolbar, SXNode *pNode);
#ifdef _SKIN_EDITOR_
		void ToXML(CSkinToolbar *pToolbar, CXMLWriter &xml);
#endif // _SKIN_EDITOR_

	};

	friend struct Button;

	typedef vector<Button>		vec_buttons;

public:
	CSkinToolbar();
	virtual ~CSkinToolbar();

	void OnCreate();

	void OnAdjustHue(float hue, float saturation, float luminance);

	void OnSize();
	void OnTimer(int nId);

	bool OnMouseDrag(CPoint point);
	bool OnMouseMove(CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);

	void OnLanguageChanged();

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	virtual void SetVisible(bool bVisible, bool bRedraw = false);

	virtual int FromXML(SXNode *pXmlNode);

	bool IsCheck(int nCmdID);
	void SetCheck(int nCmdID, bool bCheck, bool bRedraw = true);

	void SetBtStatus(int nCmdID, int nStatus, bool bRedraw);
	int GetBtStatus(int nCmdID);

	bool IsBtEnabled(int nCmdID);
	void EnableBt(int nCmdID, bool bEnable);

	bool IsButtonExist(int nCmdID);

#ifdef _SKIN_EDITOR_
	// 在导出到xml时，写完属性，写内容时调用
	virtual bool HasXMLChild() { return true; }
	virtual void OnToXMLChild(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

protected:
	void InvalidateButton(int nButton);
	int GetCurButton(POINT pt);

	void GroupButtonUncheckOld(int nGroup, int nCurButton);

	void AddTooltip();
	void DelTooltip();

	enum BT_DRAW_STATE
	{
		ROW_NORMAL,
		ROW_DOWN,
		ROW_HOVER,
	};

	void CalculateBtWidth(CRawGraph *pGraph, Button &bt);

	virtual void DrawButton(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft);

	void DrawCurButtonFadein(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft);

protected:
	tstring			m_strImageFile;
	CSFImage		m_image;
	int				m_nImageHeight;

	// Background image of normal state, and background image of checked button.
	tstring			m_strImageBgFile, m_strImageBgCheckedFile;
	CSFImage		m_imageBg, m_imageBgChecked;

	// The actual position or size of button will be multipled by m_nUnitsOfX.
	int				m_nUnitsOfX;

	bool			m_bEnableHover;
	bool			m_bHover;
	bool			m_bLBtDown;

	// blank images position
	int				m_nBlankX;
	int				m_nBlankCX;

	// seperator images position
	int				m_nSeperatorX;
	int				m_nSeperatorCX;

	// The spaces between two buttons.
	int				m_nBtSpacesCX;

	int				m_nMarginX, m_nMarginY;

	vec_buttons		m_vButtons;

	int				m_nCurButton;

	CSkinFontProperty	m_font;
	bool			m_bDrawBtText;

	// Has full status image (normal, down, hover)
	bool			m_bFullStatusImage;

	// Sending continuous command message, if the button is down.
	bool			m_bContinuousBegin;

	bool			m_bFadein;
	DWORD			m_dwBeginFadeinTime;
	int				m_nLastImageLeft;
	BT_DRAW_STATE	m_nLastImageDrawState;

};

#endif // !defined(AFX_SKINTOOLBAR_H__0DEC93D1_44E6_4ABE_BEF8_BC7EA2CC7576__INCLUDED_)
