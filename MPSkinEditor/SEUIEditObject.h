// UIEditObject.h: interface for the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
#define AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _OUR_MFC_LIB

// typedef vector<string>	CProperties;

class CUIObject;
interface ISESkinNotify;

template<class _BaseUIObjClass>
class CSEUIEditObject : public _BaseUIObjClass
{
public:
	CSEUIEditObject(ISESkinNotify *pNotify)
	{
		m_bLBtDown = FALSE;
		static BYTE	r = 0, g = 0, b = 0;
		r += 75;
		g += 50;
		b += 25;
		m_brBk.CreateSolidBrush(MLColor(RGB(r, g, b)));
		m_nMsgWant = UO_MSG_WANT_ALL;
		m_pNotify = pNotify;
	}
	virtual ~CSEUIEditObject()
	{
	}

	virtual bool OnLButtonDown(UINT nFlags, CPoint point)
	{
		m_bLBtDown = TRUE;
		m_bCanBeMoved = false;
		m_ptDragOld = point;

		// 捕捉鼠标输入
		m_pSkin->SetCaptureMouse(this);

		return true;
	}
	virtual bool OnLButtonUp(UINT nFlags, CPoint point)
	{
		m_bLBtDown = FALSE;
		m_bCanBeMoved = false;

		// 释放鼠标输入
		m_pSkin->ReleaseCaptureMouse(this);
		return true;
	}
	virtual bool OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_bLBtDown)
		{
			//
			// 移动此控件
			CString		str;
			// int			nNewPos;
			int			nOffset;

			if (!m_bCanBeMoved)
			{
				if (abs(point.x - m_ptDragOld.x) < 4 && abs(point.y - m_ptDragOld.y) < 4)
					return true;
				else
					m_bCanBeMoved = true;
			}

			nOffset = point.x - m_ptDragOld.x;
			// nNewPos = m_rcObj.left + point.x - m_ptDragOld.x + SIZE_MARGIN;
			if (nOffset != 0)
			{
				if (nOffset > 0)
					str.Format(_T("%s + %d"), m_formLeft.GetFormula(), nOffset);
				else
					str.Format(_T("%s%d"), m_formLeft.GetFormula(), nOffset);
				SetProperty(_T("left"), str);

				if (m_pNotify)
					m_pNotify->OnFocusUIObjPropertyChanged(_T("left"), m_formLeft.GetFormula());
			}

			nOffset = point.y - m_ptDragOld.y;
			// nNewPos = m_rcObj.top + point.y - m_ptDragOld.y + SIZE_MARGIN;
			if (nOffset != 0)
			{
				if (nOffset > 0)
					str.Format(_T("%s + %d"), m_formTop.GetFormula(), nOffset);
				else
					str.Format(_T("%s%d"), m_formTop.GetFormula(), nOffset);
				SetProperty(_T("top"), str);
				if (m_pNotify)
					m_pNotify->OnFocusUIObjPropertyChanged(_T("top"), m_formTop.GetFormula());
			}

			m_pSkin->RecalculateSizePos();

			m_pSkin->InvalidateRect(NULL, TRUE);

			m_ptDragOld = point;
		}
		return true;
	}

	virtual void Draw(CRawGraph *pGraph)
	{
		pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), &m_brBk);

		_BaseUIObjClass::Draw(pGraph);

		if (IsOnFocus())
		{
		//	BitBlt(hdc, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), NULL, 0, 0, WHITENESS);
			HDC		hdc = pGraph->GetHandle();

			HPEN	hPen, hPenOld;
			COLORREF	clrOld;
			int			nBkModeOld;

			hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
			hPenOld = (HPEN)SelectObject(hdc, hPen);
			clrOld = ::SetBkColor(hdc, RGB(255, 255, 255));
			nBkModeOld = SetBkMode(hdc, OPAQUE);

			// 画边缘
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
			::SetBkColor(hdc, clrOld);
			SelectObject(hdc, hPenOld);

			DeleteObject(hPen);
		}
	}

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
	{
		if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
		{
			_BaseUIObjClass::SetProperty(szProperty, szValue);

			if (m_pNotify)
				m_pNotify->OnUIObjNameChanged(this);

			return true;
		}

		return _BaseUIObjClass::SetProperty(szProperty, szValue);
	}

	void SetNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

protected:
	virtual void OnKillFocus()
	{
		_BaseUIObjClass::OnKillFocus();
		m_pSkin->InvalidateRect();
	}
	virtual void OnSetFocus()
	{
		_BaseUIObjClass::OnSetFocus();
		Invalidate();
	}

protected:
	BOOL			m_bLBtDown;
	POINT			m_ptDragOld;			// drag begin pos
	bool			m_bCanBeMoved;

	CMLBrush		m_brBk;

	ISESkinNotify	*m_pNotify;

};

#endif // !defined(AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
