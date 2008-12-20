#if !defined(AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_)
#define AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageRectSel.h : header file
//

enum ImageRectSelType
{
	IRST_NONE,
	IRST_IMG,					// SkinImage
	IRST_HORZ_STRETCH_IMG,		// HStretchImage
	IRST_VERT_STRETCH_IMG,		// VStretchImage
	IRST_CAPTION_IMG,			// CaptionImage
};

class CImageSelRectEx
{
public:
	struct Vertex
	{
		Vertex()
		{
			bCanMoveHorz = true;
			bCanMoveVert = true;
			x = y = 0;
		}
		bool	bCanMoveHorz;
		bool	bCanMoveVert;
		int		x, y;
	};
	typedef	vector<Vertex *>	V_VERTEX;

	struct Group
	{
		bool			bVert;		// true = vert, false = horz
		V_VERTEX		vVertex;
	};
	typedef	vector<Group>		V_GROUP;

	struct Line
	{
		Vertex	*pV1, *pV2;
	};
	typedef	vector<Line>		V_LINE;

public:
	CImageSelRectEx();
	virtual ~CImageSelRectEx();

	void Close();

	void AddVertexOrderByX(Vertex *pVer);
	void AddVertexOrderByY(Vertex *pVer);
	void AddLine(Line &line);
	void AddLine(Vertex *pV1, Vertex *pV2);
	void AddGroup(Group &group);

	void SetSelAllRect(Vertex *pV1, Vertex *pV2);

	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	void Draw(HDC hdc, int nScale);

	void DrawXor(HDC hdc);

	void OffsetAllVertex(int nXOffset, int nYOffset);

	void SetVertex(Vertex *pV, int x, int y);

protected:
	Group *FindGroup(Vertex *pV, bool bVert);
	void OffsetVertexInGroup(Group *gr, bool bVert, int nOffset);

	void OffsetVertex(int nXOffset, int nYOffset);

public:
	V_VERTEX		m_vVertexOBX, m_vVertexOBY;	// OBX = order by X
	V_LINE			m_vLine;
	V_GROUP			m_vGroup;

	Group			m_SelAllRect;
	bool			m_bDragAll;

	bool			m_bMoveKeepVertexOrder;			// order by x, y

	BOOL			m_bLBtDown;
	POINT			m_ptDragOld;			// drag begin pos

	// int				m_nSelGroup;
	int				m_nSelVertex;
	int				m_nScale;

public:
	HWND			m_hWnd;

};

/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl window

/*

  // SS = StretchStart
  // SE = StretchEnd
  v1 ---vSS----vSE-----vSS2----vSE2-- v2
  |                                   |
  |                                   |
  |                                   |
  |                                   |
  v4 ---vSSb---vSEb----vSS2b---vSE2b- v3

*/

class CImageRectSelCtrl : public CStatic
{
// Construction
public:
	CImageRectSelCtrl();

// Attributes
protected:
	// CSFImage		m_image;
	CMLImage		m_image;
	CImageSelRectEx	m_imageSelGrid;

	ImageRectSelType	m_imgRectSelType;

	int				m_nScale;

	CImageSelRectEx::Vertex		*m_pV1;
	CImageSelRectEx::Vertex		*m_pV2;
	CImageSelRectEx::Vertex		*m_pV3;
	CImageSelRectEx::Vertex		*m_pV4;
	CImageSelRectEx::Vertex		*m_pVSS;
	CImageSelRectEx::Vertex		*m_pVSE;
	CImageSelRectEx::Vertex		*m_pVSS2;
	CImageSelRectEx::Vertex		*m_pVSE2;
	CImageSelRectEx::Vertex		*m_pVSSb;
	CImageSelRectEx::Vertex		*m_pVSEb;
	CImageSelRectEx::Vertex		*m_pVSS2b;
	CImageSelRectEx::Vertex		*m_pVSE2b;


// Operations
public:
	int GetScale() { return m_nScale; }
	void SetScale(int nScale);
	bool SetImageFile(LPCTSTR szImage);
	void InitAsRectImg(CRect &rcObj);
	void InitAsHorzStretchImg(CRect &rcObj, int nStretchStartX, int nStretchEndX);
	void InitAsVertStretchImg(CRect &rcObj, int nStretchStartY, int nStretchEndY);
	void InitAsCaptionImg(CRect &rcObj, int nStretchStartX, int nStretchEndX, int nStretchStartX2, int nStretchEndX2);

	void GetRect(CRect &rcObj);
	int GetStretchStart();
	int GetStretchEnd();
	int GetStretchStart2();
	int GetStretchEnd2();

	int GetImageWidth() { return m_image.m_cx; }
	int GetImageHeight() { return m_image.m_cy; }

	void SetRectX(int x);
	void SetRectY(int y);
	void SetRectCx(int cx);
	void SetRectCy(int cy);
	void SetStretchStart(int nStretchStart);
	void SetStretchEnd(int nStretchEnd);
	void SetStretchStart2(int nStretchStart2);
	void SetStretchEnd2(int nStretchEnd2);

	void GetLatestCursorPos(CPoint &pt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageRectSelCtrl)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageRectSelCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImageRectSelCtrl)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_)
