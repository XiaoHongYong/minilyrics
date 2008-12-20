// SESkinContainer.h: interface for the CSESkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_)
#define AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_

#define INDEX_FOCUS_SELF		-2
interface ISESkinNotify;

template<class _BaseSkinContainer>
class CSESkinContainer : public _BaseSkinContainer
{
public:
	CSESkinContainer(ISESkinNotify *pNotify)
	{
		m_pNotify = pNotify;
	}
	virtual ~CSESkinContainer()
	{

	}

	void OnKillFocus()
	{
		m_pSkin->InvalidateRect(NULL, TRUE);
	}
	void OnSetFocus()
	{
		Invalidate();
	}

	void Draw(CRawGraph *pGraph)
	{
		HDC		hdc = pGraph->GetHandle();

		CRect	rc;
		rc = m_rcObj;

		_BaseSkinContainer::Draw(pGraph);

		if (IsOnFocus())
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

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
	{
		BOOL	bRet = _BaseSkinContainer::SetProperty(szProperty, szValue);

		if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
		{
			if (m_pNotify)
				m_pNotify->OnUIObjNameChanged(this);

			return true;
		}

		return bRet;
	}

	void SetNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

protected:
	void SwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew)
	{
		if (pUIObjFocusOld != pUIObjFocusNew)
		{
			if (pUIObjFocusOld)
				pUIObjFocusOld->OnKillFocus();

			if (pUIObjFocusNew)
				pUIObjFocusNew->OnSetFocus();
		}
	}


	ISESkinNotify	*m_pNotify;

};

#endif // !defined(AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_)
