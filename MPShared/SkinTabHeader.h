// SkinTabHeader.h: interface for the CSkinTabHeader class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SKIN_TAB_HEADER_INC_
#define _SKIN_TAB_HEADER_INC_

#pragma once


//
//    TabHeader image must be in following alignment
//           // [ 0 [ 1 [ 2 ] 3 ] 4 ]
//		2 is active button 
//
class CSkinTabHeader : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinTabHeader();
	virtual ~CSkinTabHeader();

	struct Button
	{
		int			nID;
		int			nIDAssociatedObj;
		int			nAutoUniID;
		tstring		strText;
		tstring		strTooltip;
		bool		bTempToolTip;
		tstring		strIDAssociatedObj;

		int			nWidth;
		int			nImageWidth;
		int			nImageStartPos;

		Button();

		void FromXML(CSkinTabHeader *pTabHeader, SXNode *pNode);
#ifdef _SKIN_EDITOR_
		void ToXML(CSkinTabHeader *pTabHeader, CXMLWriter &xml);
#endif // _SKIN_EDITOR_

	};

	friend struct Button;

	typedef vector<Button>		vec_buttons;

public:
	void OnCreate();

	bool OnMouseMove(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);

	void Draw(CRawGraph *pGraph);

	BOOL ReCalculatePos(FORMULA_VAR vars[], CSkinAnimateMotion *pMotion = NULL);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	virtual int FromXML(SXNode *pXmlNode);

#ifdef _SKIN_EDITOR_
	// Write Button in XML
	virtual bool HasXMLChild() { return true; }
	virtual void OnToXMLChild(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

protected:
	int ButtonHitTest(POINT pt);

	void InvalidateButton(int nButton);

	void DrawTabButton(CRawGraph *pGraph, Button &bt, CRect &rcButton, bool bActiveTabButton);

protected:
	int					m_nButtonFaceSize;
	int					m_nButtonBorderSize;
	int					m_nTabHeight;

	int					m_nActiveButtonIncSize;

	//
	// Image order:  Inactive, active, active-mask
	//
	enum  TabImagePos
	{
		TIP_INACTIVE,
		TIP_ACTIVE,
		TIP_ACTIVE_MASK,
	};
	tstring				m_strImageFile;
	CSFImage			m_img;

	bool				m_bEnableHover;
	bool				m_bLBtDown;
	bool				m_bHover;

	vec_buttons			m_vButtons;
	int					m_nActiveButton;
	int					m_nHoverButton;

	CSkinFontProperty	m_font;

};

#endif // !_SKIN_TAB_HEADER_INC_
