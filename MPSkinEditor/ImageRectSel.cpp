// ImageRectSel.cpp : implementation file
//

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "ImageRectSel.h"
#include "../gfx/RawImageUtils.h"

CImageSelRectEx::CImageSelRectEx()
{
	m_nScale = 2;
	m_bLBtDown = false;
	m_bDragAll = false;
	m_nSelVertex = -1;
}

CImageSelRectEx::~CImageSelRectEx()
{
	Close();
}

void CImageSelRectEx::Close()
{
	m_nSelVertex = -1;
	for (int i = 0; i < m_vVertexOBX.size(); i++)
	{
		Vertex	*pV = m_vVertexOBX[i];
		delete pV;
	}
	m_vVertexOBX.clear();
	m_vLine.clear();
	m_vGroup.clear();
}

void CImageSelRectEx::AddVertexOrderByX(Vertex *pVer)
{
	m_vVertexOBX.push_back(pVer);
}

void CImageSelRectEx::AddVertexOrderByY(Vertex *pVer)
{
	m_vVertexOBY.push_back(pVer);
}

void CImageSelRectEx::AddLine(Line &line)
{
	m_vLine.push_back(line);
}

void CImageSelRectEx::AddLine(Vertex *pV1, Vertex *pV2)
{
	Line	line;
	line.pV1 = pV1;
	line.pV2 = pV2;

	m_vLine.push_back(line);
}

void CImageSelRectEx::AddGroup(Group &group)
{
	m_vGroup.push_back(group);
}

void CImageSelRectEx::SetSelAllRect(Vertex *pV1, Vertex *pV2)
{
	m_SelAllRect.vVertex.push_back(pV1);
	m_SelAllRect.vVertex.push_back(pV2);
}

int GetPtDistance(int x1, int y1, int x2, int y2)
{
	return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

void CImageSelRectEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect	rc;
	int		nNearGroupDistance = 1024;
	int		nSelVertext = -1;
	Vertex	*pVerSel = NULL;

	for (int i = 0; i < m_vVertexOBX.size(); i++)
	{
		Vertex	*pV = m_vVertexOBX[i];

		rc.SetRect(pV->x - 3, pV->y - 3, pV->x + 3, pV->y + 3);
		if (rc.PtInRect(point))
		{
			int		n = GetPtDistance(pV->x, pV->y, point.x, point.y);
			if (n < nNearGroupDistance)
			{
				nNearGroupDistance = n;
				pVerSel = pV;
				nSelVertext = i;
			}
		}
	}

	if (nSelVertext != m_nSelVertex)
	{
		HDC hdc = ::GetDC(m_hWnd);
		DrawXor(hdc);
		m_nSelVertex = nSelVertext;
		DrawXor(hdc);
		::ReleaseDC(m_hWnd, hdc);
	}

	if (m_nSelVertex == -1 && m_SelAllRect.vVertex.size() == 2)
	{
		// Is Select all?
		CRect		rc;
		rc.SetRect(m_SelAllRect.vVertex[0]->x, m_SelAllRect.vVertex[0]->y,
			m_SelAllRect.vVertex[1]->x, m_SelAllRect.vVertex[1]->y);
		if (rc.PtInRect(point))
			m_bDragAll = true;
	}

	m_bLBtDown = TRUE;
	m_ptDragOld = point;
}

void CImageSelRectEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtDown = FALSE;
	m_bDragAll = false;
}

CImageSelRectEx::Group *CImageSelRectEx::FindGroup(Vertex *pV, bool bVert)
{
	for (int i = 0; i < m_vGroup.size(); i++)
	{
		Group	&gr = m_vGroup[i];
		if (gr.bVert != bVert)
			continue;

		for (int k = 0; k < gr.vVertex.size(); k++)
		{
			Vertex	*pT = gr.vVertex[k];
			if (pT == pV)
			{
				// find the group
				return &gr;
			}
		}
	}

	return NULL;
}

void CImageSelRectEx::OffsetVertexInGroup(Group *gr, bool bVert, int nOffset)
{
	ASSERT(gr->bVert == bVert);

	if (bVert)
	{
		for (int k = 0; k < gr->vVertex.size(); k++)
		{
			Vertex	*pT = gr->vVertex[k];
			pT->y += nOffset;
		}
	}
	else
	{
		// horz
		for (int k = 0; k < gr->vVertex.size(); k++)
		{
			Vertex	*pT = gr->vVertex[k];
			pT->x += nOffset;
		}
	}
}

void CImageSelRectEx::OffsetVertex(int nXOffset, int nYOffset)
{
	if (m_nSelVertex != -1)
	{
		ASSERT(m_nSelVertex >= 0 && m_nSelVertex < m_vVertexOBX.size());

		Vertex	*pV = m_vVertexOBX[m_nSelVertex];

		if (nXOffset != 0 && pV->bCanMoveHorz)
		{
			// Move all vertex in group
			Group	*gr = FindGroup(pV, false);
			if (gr)
				OffsetVertexInGroup(gr, false, nXOffset);
			else
				pV->x += nXOffset;	// The vertex doesn't belong to any group
		}

		if (nYOffset != 0 && pV->bCanMoveVert)
		{
			// Move all vertex in group
			Group	*gr = FindGroup(pV, true);
			if (gr)
				OffsetVertexInGroup(gr, true, nYOffset);
			else
				pV->y += nYOffset;	// The vertex doesn't belong to any group
		}
	}
	else if (m_bDragAll)
	{
		for (int i = 0; i < m_vVertexOBX.size(); i++)
		{
			Vertex	*pV = m_vVertexOBX[i];

			pV->x += nXOffset;

			pV->y += nYOffset;
		}
	}
}

void CImageSelRectEx::OffsetAllVertex(int nXOffset, int nYOffset)
{
	for (int i = 0; i < m_vVertexOBX.size(); i++)
	{
		Vertex	*pV = m_vVertexOBX[i];

		pV->x += nXOffset;

		pV->y += nYOffset;
	}
}

void CImageSelRectEx::SetVertex(Vertex *pV, int x, int y)
{
	if (x != 0)
	{
		// modify x in same group
		Group *gr = FindGroup(pV, false);
		for (int i = 0; i < gr->vVertex.size(); i++)
		{
			Vertex	*v = gr->vVertex[i];
			v->x = x;
		}
	}

	if (y != 0)
	{
		// modify y in same group
		Group *gr = FindGroup(pV, true);
		for (int i = 0; i < gr->vVertex.size(); i++)
		{
			Vertex	*v = gr->vVertex[i];
			v->y = y;
		}
	}
}

void CImageSelRectEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLBtDown && (m_nSelVertex != -1 || m_bDragAll))
	{
		HDC hdc = ::GetDC(m_hWnd);
		DrawXor(hdc);

		OffsetVertex(point.x - m_ptDragOld.x, point.y - m_ptDragOld.y);

		DrawXor(hdc);
		::ReleaseDC(m_hWnd, hdc);

		::SendMessage(::GetParent(m_hWnd), WM_COMMAND, IDC_UPDATE_RC, 0);

		m_ptDragOld = point;
	}
}

void CImageSelRectEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bLBtDown)
		return;

	if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)
	{
		int		nXOffset = 0, nYOffset = 0;

		if (nChar == VK_LEFT || nChar == VK_RIGHT)
		{
			if (nChar == VK_LEFT)
				nXOffset = -1;
			else
				nXOffset = 1;
		}
		else if (nChar == VK_UP || nChar == VK_DOWN)
		{
			if (nChar == VK_UP)
				nYOffset = -1;
			else
				nYOffset = 1;
		}

		HDC hdc = ::GetDC(m_hWnd);
		DrawXor(hdc);

		if (m_nSelVertex == -1)
			m_bDragAll = true;
		OffsetVertex(nXOffset, nYOffset);
		m_bDragAll = false;

		DrawXor(hdc);
		::ReleaseDC(m_hWnd, hdc);

		::SendMessage(::GetParent(m_hWnd), WM_COMMAND, IDC_UPDATE_RC, 0);
	}
}

void DrawVertex(HDC hdc, CImageSelRectEx::Vertex *pV, int nSize)
{
	MoveToEx(hdc, pV->x - nSize, pV->y - nSize, NULL);
	LineTo(hdc, pV->x + nSize, pV->y - nSize);

	LineTo(hdc, pV->x + nSize, pV->y + nSize);

	LineTo(hdc, pV->x - nSize, pV->y + nSize);

	LineTo(hdc, pV->x - nSize, pV->y - nSize);
}

void CImageSelRectEx::DrawXor(HDC hdc)
{
	HPEN		hPen, hPenOld;
	COLORREF	clrOld;
	int			nBkModeOld;
	int			nRop2Old;
	CRect		rc;

	hPen = CreatePen(PS_DOT, 1, RGB(0x7F, 0x7F, 0x7F));
	hPenOld = (HPEN)SelectObject(hdc, hPen);
	clrOld = SetBkColor(hdc, RGB(255, 255, 255));
	nBkModeOld = SetBkMode(hdc, OPAQUE);
	nRop2Old = SetROP2(hdc, R2_NOTXORPEN);

	Draw(hdc, m_nScale);

	SetBkMode(hdc, nBkModeOld);
	SetBkColor(hdc, clrOld);
	SelectObject(hdc, hPenOld);
	SetROP2(hdc, nRop2Old);

	DeleteObject(hPen);
}

void CImageSelRectEx::Draw(HDC hdc, int nScale)
{
	UINT		i;
	Vertex		v;

	for (i = 0; i < m_vLine.size(); i++)
	{
		MoveToEx(hdc, m_vLine[i].pV1->x * nScale, m_vLine[i].pV1->y * nScale, NULL);
		LineTo(hdc, m_vLine[i].pV2->x * nScale, m_vLine[i].pV2->y * nScale);
	}

	HPEN		hPen, hPenOld;

	hPen = CreatePen(PS_SOLID, 1, RGB(0x7F, 0x7F, 0x7F));
	hPenOld = (HPEN)SelectObject(hdc, hPen);

	for (i = 0; i < m_vVertexOBX.size(); i++)
	{
		v.x = m_vVertexOBX[i]->x * nScale;
		v.y = m_vVertexOBX[i]->y * nScale;
		DrawVertex(hdc, &v, 2);
	}

	// draw focus vertex
	if (m_nSelVertex != -1)
	{
		v.x = m_vVertexOBX[m_nSelVertex]->x * nScale;
		v.y = m_vVertexOBX[m_nSelVertex]->y * nScale;
		DrawVertex(hdc, &v, 3);
	}

	SelectObject(hdc, hPenOld);

	DeleteObject(hPen);
}


/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl

CImageRectSelCtrl::CImageRectSelCtrl()
{
	m_nScale = 3;
	m_imgRectSelType = IRST_NONE;

	m_pV1 = new CImageSelRectEx::Vertex;
	m_pV2 = new CImageSelRectEx::Vertex;
	m_pV3 = new CImageSelRectEx::Vertex;
	m_pV4 = new CImageSelRectEx::Vertex;
	m_pVSS = new CImageSelRectEx::Vertex;
	m_pVSE = new CImageSelRectEx::Vertex;
	m_pVSS2 = new CImageSelRectEx::Vertex;
	m_pVSE2 = new CImageSelRectEx::Vertex;
	m_pVSSb = new CImageSelRectEx::Vertex;
	m_pVSEb = new CImageSelRectEx::Vertex;
	m_pVSS2b = new CImageSelRectEx::Vertex;
	m_pVSE2b = new CImageSelRectEx::Vertex;
}

CImageRectSelCtrl::~CImageRectSelCtrl()
{
}


BEGIN_MESSAGE_MAP(CImageRectSelCtrl, CStatic)
	//{{AFX_MSG_MAP(CImageRectSelCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl message handlers

void CImageRectSelCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (m_image.IsValid())
	{
		CRgn		rgn;
		CRect		rc;

		GetClientRect(&rc);
		rgn.m_hObject = NULL;
		rgn.CreateRectRgnIndirect(&rc);
		dc.SelectClipRgn(&rgn);

		HDC		hdcTemp = CreateCompatibleDC(dc.m_hDC);
		HBITMAP	hBmpTemp = (HBITMAP)SelectObject(hdcTemp, m_image.GetHandle());
		if (m_nScale == 1)
			BitBlt(dc.m_hDC, 0, 0, m_image.m_cx, m_image.m_cy, hdcTemp, 0, 0, SRCCOPY);
		else
			StretchBlt(dc.m_hDC, 0, 0, m_image.m_cx * m_nScale, m_image.m_cy * m_nScale, hdcTemp, 0, 0, m_image.m_cx, m_image.m_cy, SRCCOPY);
		::SelectObject(hdcTemp, hBmpTemp);
		DeleteDC(hdcTemp);

		dc.SelectClipRgn(NULL);
	}

	if (m_imgRectSelType != IRST_NONE)
		m_imageSelGrid.DrawXor(dc.m_hDC);
}

void CImageRectSelCtrl::InitAsRectImg(CRect &rcObj)
{
	m_imgRectSelType = IRST_IMG;

	m_pV4->x = m_pV1->x = rcObj.left;
	m_pV2->y = m_pV1->y = rcObj.top;

	m_pV2->x = m_pV3->x = rcObj.right;
	m_pV3->y = m_pV4->y = rcObj.bottom;

	m_imageSelGrid.AddVertexOrderByX(m_pV1);
	m_imageSelGrid.AddVertexOrderByX(m_pV4);
	m_imageSelGrid.AddVertexOrderByX(m_pV2);
	m_imageSelGrid.AddVertexOrderByX(m_pV3);

	m_imageSelGrid.AddLine(m_pV1, m_pV2);
	m_imageSelGrid.AddLine(m_pV2, m_pV3);
	m_imageSelGrid.AddLine(m_pV3, m_pV4);
	m_imageSelGrid.AddLine(m_pV4, m_pV1);

	m_imageSelGrid.m_hWnd = m_hWnd;
	m_imageSelGrid.m_nScale = m_nScale;

	CImageSelRectEx::Group		gr;
	gr.bVert = true;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV2);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pV3);
	gr.vVertex.push_back(m_pV4);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pV3);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV4);
	m_imageSelGrid.AddGroup(gr);

	m_imageSelGrid.SetSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::InitAsHorzStretchImg(CRect &rcObj, int nStretchStartX, int nStretchEndX)
{
	m_imgRectSelType = IRST_HORZ_STRETCH_IMG;

	m_pV4->x = m_pV1->x = rcObj.left;
	m_pV2->y = m_pV1->y = rcObj.top;

	m_pV2->x = m_pV3->x = rcObj.right;
	m_pV3->y = m_pV4->y = rcObj.bottom;

	m_pVSSb->x = m_pVSS->x = nStretchStartX;
	m_pVSEb->x = m_pVSE->x = nStretchEndX;
	m_pVSE->y = m_pVSS->y = rcObj.top;
	m_pVSSb->y = m_pVSEb->y = rcObj.bottom;
	m_pVSS->bCanMoveVert = m_pVSSb->bCanMoveVert = m_pVSE->bCanMoveVert = m_pVSEb->bCanMoveVert = false;

	m_imageSelGrid.AddVertexOrderByX(m_pV1);
	m_imageSelGrid.AddVertexOrderByX(m_pV4);

	m_imageSelGrid.AddVertexOrderByX(m_pVSS);
	m_imageSelGrid.AddVertexOrderByX(m_pVSSb);

	m_imageSelGrid.AddVertexOrderByX(m_pVSE);
	m_imageSelGrid.AddVertexOrderByX(m_pVSEb);

	m_imageSelGrid.AddVertexOrderByX(m_pV2);
	m_imageSelGrid.AddVertexOrderByX(m_pV3);

	m_imageSelGrid.AddLine(m_pV1, m_pV2);
	m_imageSelGrid.AddLine(m_pV2, m_pV3);
	m_imageSelGrid.AddLine(m_pV3, m_pV4);
	m_imageSelGrid.AddLine(m_pV4, m_pV1);
	m_imageSelGrid.AddLine(m_pVSS, m_pVSSb);
	m_imageSelGrid.AddLine(m_pVSE, m_pVSEb);

	m_imageSelGrid.m_hWnd = m_hWnd;
	m_imageSelGrid.m_nScale = m_nScale;

	CImageSelRectEx::Group		gr;

	// top lines
	gr.bVert = true;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSE);
	m_imageSelGrid.AddGroup(gr);

	// bottom lines
	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pV3);
	gr.vVertex.push_back(m_pV4);
	gr.vVertex.push_back(m_pVSSb);
	gr.vVertex.push_back(m_pVSEb);
	m_imageSelGrid.AddGroup(gr);

	// left line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pV3);
	m_imageSelGrid.AddGroup(gr);

	// right line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV4);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSSb);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSE);
	gr.vVertex.push_back(m_pVSEb);
	m_imageSelGrid.AddGroup(gr);

	m_imageSelGrid.SetSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::InitAsVertStretchImg(CRect &rcObj, int nStretchStartY, int nStretchEndY)
{
	m_imgRectSelType = IRST_VERT_STRETCH_IMG;

	m_pV4->x = m_pV1->x = rcObj.left;
	m_pV2->y = m_pV1->y = rcObj.top;

	m_pV2->x = m_pV3->x = rcObj.right;
	m_pV3->y = m_pV4->y = rcObj.bottom;

	m_pVSSb->y = m_pVSS->y = nStretchStartY;
	m_pVSEb->y = m_pVSE->y = nStretchEndY;
	m_pVSE->x = m_pVSS->x = rcObj.left;
	m_pVSSb->x = m_pVSEb->x = rcObj.right;
	m_pVSS->bCanMoveHorz = m_pVSSb->bCanMoveHorz = m_pVSE->bCanMoveHorz = m_pVSEb->bCanMoveHorz = false;

	m_imageSelGrid.AddVertexOrderByX(m_pV1);
	m_imageSelGrid.AddVertexOrderByX(m_pV4);

	m_imageSelGrid.AddVertexOrderByX(m_pVSSb);

	m_imageSelGrid.AddVertexOrderByX(m_pVSEb);

	m_imageSelGrid.AddVertexOrderByX(m_pVSS);
	m_imageSelGrid.AddVertexOrderByX(m_pVSE);
	m_imageSelGrid.AddVertexOrderByX(m_pV2);
	m_imageSelGrid.AddVertexOrderByX(m_pV3);

	m_imageSelGrid.AddLine(m_pV1, m_pV2);
	m_imageSelGrid.AddLine(m_pV2, m_pV3);
	m_imageSelGrid.AddLine(m_pV3, m_pV4);
	m_imageSelGrid.AddLine(m_pV4, m_pV1);
	m_imageSelGrid.AddLine(m_pVSS, m_pVSSb);
	m_imageSelGrid.AddLine(m_pVSE, m_pVSEb);

	m_imageSelGrid.m_hWnd = m_hWnd;
	m_imageSelGrid.m_nScale = m_nScale;

	CImageSelRectEx::Group		gr;

	// top lines
	gr.bVert = true;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV2);
	m_imageSelGrid.AddGroup(gr);

	// bottom lines
	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pV3);
	gr.vVertex.push_back(m_pV4);
	m_imageSelGrid.AddGroup(gr);

	// left line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pV3);
	gr.vVertex.push_back(m_pVSSb);
	gr.vVertex.push_back(m_pVSEb);
	m_imageSelGrid.AddGroup(gr);

	// right line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV4);
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSE);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSSb);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pVSE);
	gr.vVertex.push_back(m_pVSEb);
	m_imageSelGrid.AddGroup(gr);

	m_imageSelGrid.SetSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::InitAsCaptionImg(CRect &rcObj, int nStretchStartX, int nStretchEndX, int nStretchStartX2, int nStretchEndX2)
{
	m_imgRectSelType = IRST_CAPTION_IMG;

	m_pV4->x = m_pV1->x = rcObj.left;
	m_pV2->y = m_pV1->y = rcObj.top;

	m_pV2->x = m_pV3->x = rcObj.right;
	m_pV3->y = m_pV4->y = rcObj.bottom;

	m_pVSSb->x = m_pVSS->x = nStretchStartX;
	m_pVSEb->x = m_pVSE->x = nStretchEndX;

	m_pVSS2b->x = m_pVSS2->x = nStretchStartX2;
	m_pVSE2b->x = m_pVSE2->x = nStretchEndX2;

	m_pVSS2->y = m_pVSE2->y = m_pVSE->y = m_pVSS->y = rcObj.top;
	m_pVSS2b->y = m_pVSE2b->y = m_pVSSb->y = m_pVSEb->y = rcObj.bottom;

	m_pVSS->bCanMoveVert = m_pVSSb->bCanMoveVert = m_pVSE->bCanMoveVert = m_pVSEb->bCanMoveVert = 
		m_pVSS2->bCanMoveVert = m_pVSS2b->bCanMoveVert = m_pVSE2->bCanMoveVert = m_pVSE2b->bCanMoveVert = false;

	m_imageSelGrid.AddVertexOrderByX(m_pV1);
	m_imageSelGrid.AddVertexOrderByX(m_pV4);

	m_imageSelGrid.AddVertexOrderByX(m_pVSS);
	m_imageSelGrid.AddVertexOrderByX(m_pVSSb);

	m_imageSelGrid.AddVertexOrderByX(m_pVSE);
	m_imageSelGrid.AddVertexOrderByX(m_pVSEb);

	m_imageSelGrid.AddVertexOrderByX(m_pVSS2);
	m_imageSelGrid.AddVertexOrderByX(m_pVSS2b);

	m_imageSelGrid.AddVertexOrderByX(m_pVSE2);
	m_imageSelGrid.AddVertexOrderByX(m_pVSE2b);

	m_imageSelGrid.AddVertexOrderByX(m_pV2);
	m_imageSelGrid.AddVertexOrderByX(m_pV3);

	m_imageSelGrid.AddLine(m_pV1, m_pV2);
	m_imageSelGrid.AddLine(m_pV2, m_pV3);
	m_imageSelGrid.AddLine(m_pV3, m_pV4);
	m_imageSelGrid.AddLine(m_pV4, m_pV1);
	m_imageSelGrid.AddLine(m_pVSS, m_pVSSb);
	m_imageSelGrid.AddLine(m_pVSE, m_pVSEb);
	m_imageSelGrid.AddLine(m_pVSS2, m_pVSS2b);
	m_imageSelGrid.AddLine(m_pVSE2, m_pVSE2b);

	m_imageSelGrid.m_hWnd = m_hWnd;
	m_imageSelGrid.m_nScale = m_nScale;

	CImageSelRectEx::Group		gr;

	// top lines
	gr.bVert = true;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSE);
	gr.vVertex.push_back(m_pVSS2);
	gr.vVertex.push_back(m_pVSE2);
	m_imageSelGrid.AddGroup(gr);

	// bottom lines
	gr.vVertex.clear();
	gr.bVert = true;
	gr.vVertex.push_back(m_pV3);
	gr.vVertex.push_back(m_pV4);
	gr.vVertex.push_back(m_pVSSb);
	gr.vVertex.push_back(m_pVSEb);
	gr.vVertex.push_back(m_pVSS2b);
	gr.vVertex.push_back(m_pVSE2b);
	m_imageSelGrid.AddGroup(gr);

	// left line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV2);
	gr.vVertex.push_back(m_pV3);
	m_imageSelGrid.AddGroup(gr);

	// right line
	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pV1);
	gr.vVertex.push_back(m_pV4);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSS);
	gr.vVertex.push_back(m_pVSSb);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSE);
	gr.vVertex.push_back(m_pVSEb);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSS2);
	gr.vVertex.push_back(m_pVSS2b);
	m_imageSelGrid.AddGroup(gr);

	gr.vVertex.clear();
	gr.bVert = false;
	gr.vVertex.push_back(m_pVSE2);
	gr.vVertex.push_back(m_pVSE2b);
	m_imageSelGrid.AddGroup(gr);

	m_imageSelGrid.SetSelAllRect(m_pV1, m_pV3);
}


void CImageRectSelCtrl::GetRect(CRect &rcObj)
{
	rcObj.left = m_pV1->x;
	rcObj.top = m_pV1->y;
	rcObj.right = m_pV3->x;
	rcObj.bottom = m_pV3->y;
}

int CImageRectSelCtrl::GetStretchStart()
{
	if (m_imgRectSelType == IRST_VERT_STRETCH_IMG)
		return m_pVSS->y;
	else
		return m_pVSS->x;
}

int CImageRectSelCtrl::GetStretchEnd()
{
	if (m_imgRectSelType == IRST_VERT_STRETCH_IMG)
		return m_pVSE->y;
	else
		return m_pVSE->x;
}

int CImageRectSelCtrl::GetStretchStart2()
{
	return m_pVSS2->x;
}

int CImageRectSelCtrl::GetStretchEnd2()
{
	return m_pVSE2->x;
}

void CImageRectSelCtrl::SetRectX(int x)
{
	int		nOffset = x - m_pV1->x;
	m_imageSelGrid.OffsetAllVertex(nOffset, 0);
}

void CImageRectSelCtrl::SetRectY(int y)
{
	int		nOffset = y - m_pV1->y;
	m_imageSelGrid.OffsetAllVertex(0, nOffset);
}

void CImageRectSelCtrl::SetRectCx(int cx)
{
	int		x = m_pV1->x + cx;
	m_imageSelGrid.SetVertex(m_pV2, x, 0);
}

void CImageRectSelCtrl::SetRectCy(int cy)
{
	int		y = m_pV1->y + cy;
	m_imageSelGrid.SetVertex(m_pV3, 0, y);
}

void CImageRectSelCtrl::SetStretchStart(int nStretchStart)
{
	m_pVSS->x = nStretchStart;
	m_pVSSb->x = nStretchStart;
}

void CImageRectSelCtrl::SetStretchEnd(int nStretchEnd)
{
	m_pVSE->x = nStretchEnd;
	m_pVSEb->x = nStretchEnd;
}

void CImageRectSelCtrl::SetStretchStart2(int nStretchStart2)
{
	m_pVSS2->x = nStretchStart2;
	m_pVSS2b->x = nStretchStart2;
}

void CImageRectSelCtrl::SetStretchEnd2(int nStretchEnd2)
{
	m_pVSE2->x = nStretchEnd2;
	m_pVSE2b->x = nStretchEnd2;
}

int CImageRectSelCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CImageRectSelCtrl::OnDestroy() 
{
	CStatic::OnDestroy();
	
	m_image.Destroy();
	m_imageSelGrid.Close();
}

void CImageRectSelCtrl::SetScale(int nScale)
{
	m_imageSelGrid.m_nScale = nScale;
	m_nScale = nScale;
}

bool CImageRectSelCtrl::SetImageFile(LPCTSTR szImage)
{
	CSFImage		image;

	image.LoadFromSRM(&g_skinFactory, szImage);

	m_image.Destroy();
	m_image.Attach(RawImageToHBitmap(image));

	if (m_hWnd)
		Invalidate();

	return true;
}

void CImageRectSelCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	if (m_imgRectSelType == IRST_NONE)
		return;

	point.x /= m_nScale;
	point.y /= m_nScale;

	m_imageSelGrid.OnLButtonDown(nFlags, point);

	SetCapture();

	CStatic::OnLButtonDown(nFlags, point);
}

void CImageRectSelCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	point.x /= m_nScale;
	point.y /= m_nScale;

	m_imageSelGrid.OnLButtonUp(nFlags, point);

	ReleaseCapture();
}

void CImageRectSelCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	point.x /= m_nScale;
	point.y /= m_nScale;

	CString		str;
	str.Format(_T("X: %4d, Y: %4d"), point.x, point.y);
	::SetDlgItemText(::GetParent(m_hWnd), IDC_S_CURSOR, str);

	m_imageSelGrid.OnMouseMove(nFlags, point);
}

LRESULT CImageRectSelCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_GETDLGCODE)
	{
		return DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_DEFPUSHBUTTON;
	}
	
	return CStatic::WindowProc(message, wParam, lParam);
}

void CImageRectSelCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_imgRectSelType != IRST_NONE)
		m_imageSelGrid.OnKeyDown(nChar, nRepCnt, nFlags);

	CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CImageRectSelCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect	rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(&rc, RGB(255, 255, 255));

	return true;
}
