// SESkinContainer.cpp: implementation of the CSESkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SESkinWnd.h"
#include "SESkinContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
/*

class CSESkinContainer : public CSkinContainer  
{
public:
	CSESkinContainer();
	virtual ~CSESkinContainer();

	void OnCreate();

	void OnKillFocus();
	void OnSetFocus();

	void Draw(IMLGraphicsBase *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void SetNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

	//
	// CSkinContainer API
	//
	virtual CUIObject *GetFocusChild();
	virtual void SetFocusChild(CUIObject *pObjChild);

	// if no ctrl can be set focused or reached to the end of ctrl, returns false.
	virtual bool FocusNextChild();
	virtual bool FocusPrevChild();

protected:
	void SwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew);


	ISESkinNotify	*m_pNotify;

};


#define INDEX_FOCUS_SELF		-2
#define SIZE_MARGIN				0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinContainer::CSESkinContainer()
{
	m_pNotify = NULL;
}

CSESkinContainer::~CSESkinContainer()
{

}

void CSESkinContainer::OnCreate()
{
	CSkinContainer::OnCreate();
}

void CSESkinContainer::OnKillFocus()
{
	m_pSkin->InvalidateRect(NULL, TRUE);
}

void CSESkinContainer::OnSetFocus()
{
	Invalidate();
}

void CSESkinContainer::Draw(IMLGraphicsBase *pGraph)
{
	HDC		hdc = pGraph->GetHandle();

	CRect	rc;
	rc = m_rcObj;
	rc.left += SIZE_MARGIN;
	rc.top += SIZE_MARGIN;
	rc.right -= SIZE_MARGIN;
	rc.bottom -= SIZE_MARGIN;

	CSkinContainer::Draw(pGraph);

	if (IsOnFocus() && m_nFocusChild == INDEX_FOCUS_SELF)
	{
		HPEN	hPen, hPenOld;
		COLORREF	clrOld;
		int			nBkModeOld;

		hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
		hPenOld = (HPEN)SelectObject(hdc, hPen);
		clrOld = SetBkColor(hdc, RGB(255, 255, 255));
		nBkModeOld = SetBkMode(hdc, OPAQUE);

		// »­±ßÔµ
		// for (int i = 0; i < SIZE_MARGIN; i+=2)
		int	i = 0;
		{
			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.top + i);

			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);

			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1);

			MoveToEx(hdc, m_rcObj.right - i - 1, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);
		}

		SetBkMode(hdc, nBkModeOld);
		SetBkColor(hdc, clrOld);
		SelectObject(hdc, hPenOld);

		DeleteObject(hPen);
	}
}

CUIObject *CSESkinContainer::GetFocusChild()
{
	if (m_nFocusChild == INDEX_FOCUS_SELF)
		return this;
	else
		return CSkinContainer::GetFocusChild();
}

void CSESkinContainer::SetFocusChild(CUIObject *pObjChild)
{
	/// no equal..........???????
	if (pObjChild == this)
	{
		m_nFocusChild = INDEX_FOCUS_SELF;
		m_pSkin->InvalidateRect();
	}
	else
		CSkinContainer::SetFocusChild(pObjChild);
}

// if no ctrl can be set focused or reached to the end of ctrl, returns false.
bool CSESkinContainer::FocusNextChild()
{
	if (m_nFocusChild == INDEX_FOCUS_SELF)
	{
		m_nFocusChild = -1;
		return CSkinContainer::FocusNextChild();
	}
	else if (m_nFocusChild == -1)
	{
		// set focus to self
		CUIObject	*pObjFocusOld = m_pSkin->GetFocusUIObj();

		m_nFocusChild = INDEX_FOCUS_SELF;
		SwitchChildFocus(pObjFocusOld, this);

		m_pSkin->InvalidateRect();
		return true;
	}
	else
		return CSkinContainer::FocusNextChild();
}

bool CSESkinContainer::FocusPrevChild()
{
	if (m_nFocusChild == INDEX_FOCUS_SELF)
	{
		m_nFocusChild = -1;
		Invalidate();
		return false;
	}
	else
	{
		if (CSkinContainer::FocusPrevChild())
			return true;
		else
		{
			CUIObject	*pObjFocusOld = m_pSkin->GetFocusUIObj();

			m_nFocusChild = INDEX_FOCUS_SELF;
			SwitchChildFocus(pObjFocusOld, this);
			m_pSkin->InvalidateRect();
			return true;
		}
	}
}

void CSESkinContainer::SwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew)
{
	if (pUIObjFocusOld != pUIObjFocusNew)
	{
		if (pUIObjFocusOld)
			pUIObjFocusOld->OnKillFocus();

		if (pUIObjFocusNew)
			pUIObjFocusNew->OnSetFocus();
	}
}

BOOL CSESkinContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	BOOL	bRet = CSkinContainer::SetProperty(szProperty, szValue);

	if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
	{
		if (m_pNotify)
			m_pNotify->OnUIObjNameChanged(this);

		return true;
	}

	return bRet;
}
*/