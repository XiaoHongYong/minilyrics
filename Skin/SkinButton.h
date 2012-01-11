/********************************************************************
	Created  :	2002/01/04	21:42
	FileName :	SkinButton.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_

#include "UIObject.h"
#include "SkinNStatusButton.h"

class CSkinButton : public CSkinNStatusButton  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinNStatusButton)
public:
	CSkinButton();
	virtual ~CSkinButton();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	bool IsCheck();
	void SetCheck(bool bCheck);

protected:
	virtual void ButtonUpAction();

	bool			m_bRadioBt;

};

class CSkinActiveButton : public CSkinButton
{
UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
	void Draw(CRawGraph *pGraph);
	
	CSkinActiveButton();
	virtual ~CSkinActiveButton();

};

class CSkinImageButton : public CSkinButton
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
	CSkinImageButton();
	virtual ~CSkinImageButton();

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void SetContentImage(PRAW_IMAGE_DATA pRawImage);

	void SetColor(const MLColor &clr, bool bAutoSelColor);

	MLColor GetColor() const
		{ return m_clrContent; }

	void ButtonUpAction();

protected:
	int					m_nContentMarginX, m_nContentMarginY;
	CRawImage			m_contentImage;
	MLColor				m_clrContent;
	bool				m_bContentImage;
	bool				m_bAutoSelColor;

};

class CSkinHoverActionBtEventNotify : public IUIObjNotify
{
public:
	CSkinHoverActionBtEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_CLICK; }
	enum Command
	{
		C_CLICK,
		C_BEGIN_HOVER,
		C_HOVER_ACTION,
		C_END_HOVER,
	};

	Command				cmd;

};

class CSkinHoverActionButton : public CSkinButton
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
	CSkinHoverActionButton();
	virtual ~CSkinHoverActionButton();

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnMouseMove(CPoint point);

	void ButtonUpAction();

	void OnTimer(int nId);

protected:
	void DispatchEvent(CSkinHoverActionBtEventNotify::Command cmd);

	void EndHover();

protected:
	int					m_nTimerIdHoverAction;
	bool				m_bHoverActionBegin;
	DWORD				m_dwBeginHoverTime;

};

#endif // !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
