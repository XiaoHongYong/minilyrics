/********************************************************************
	Created  :	2002/01/04	21:43
	FileName :	WndResizer.h
	Author   :	xhy
	
	Purpose  :	调整窗口大小的类
*********************************************************************/

#if !defined(AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_

class CWndResizer
{
public:
	enum ResizeDirection
	{
		RD_TOP				= 0x1,
		RD_LEFT				= 0x1 << 1,
		RD_RIGHT			= 0x1 << 2,
		RD_BOTTOM			= 0x1 << 3,
	};

	struct ResizeArea
	{
		int			nID;
		uint32		resizeDirection;
		CRect		rcResizeArea;
	};

	typedef list<ResizeArea>		ListResizeArea;

public:
	CWndResizer();
	virtual ~CWndResizer();

	void Init(CBaseWnd *pWnd);

	void OnMouseMessage(UINT fwKeys, POINT pt);
	void SetSizeCursor(UINT fwKeys, POINT pt);

	bool IsSizing();

	void FixedHeight(bool bFixedHeight) { m_bFixedHeight = bFixedHeight; }
	void FixedWidth(bool bFixedWidth) { m_bFixedWidth = bFixedWidth; }

	bool IsFixedWidth() const { return m_bFixedWidth; }
	bool IsFixedHeight() const { return m_bFixedHeight; }

	bool IsResizeAutoCloseTo() const { return m_bResizingAutoCloseto; }
	void SetResizeAutoCloseTo(bool bResizeAutoCloseTo) { m_bResizingAutoCloseto = bResizeAutoCloseTo; }

	int GetMinCx() { return m_nMincx; }
	int GetMinCy() { return m_nMincy; }

	void SetMinCx(int nMincx) { m_nMincx = nMincx; }
	void SetMinCy(int nMincy) { m_nMincy = nMincy; }

	// If nID == -1, just add it.
	void SetResizeArea(int nIDArea, uint32 resizeDirection, RECT &rcResizeArea);

	void RemoveResizeArea(int nIDArea);

	void ClearResizeArea() { m_listResizeArea.clear(); }

protected:
	void EndSizing(UINT fwKeys, POINT &pt);
	void OnSizing(UINT fwKeys, POINT &pt);
	void BeginSize(UINT fwKeys, POINT &pt);

	virtual void AutoCloseToWindows(int &nOffx, int &nOffy);

	bool HitTestResizeArea(POINT &pt, ResizeArea &ra);

protected:
	ListResizeArea	m_listResizeArea;

	bool			m_bFixedWidth, m_bFixedHeight;

	bool			m_bResizing;		// 是否在拖动窗口

	bool			m_bAtLeft, m_bAtTop, m_bAtRight, m_bAtBottom;	// 是否在调整左/上/右/下边

	CRect			m_rcResizing;					// 调整中的矩形的大小

	POINT			m_ptOld;					// 鼠标上一次的位置

    bool            m_bResizingAutoCloseto;		// 是否在调整大小时, 自动靠近指定窗口

    CBaseWnd		*m_pWnd;				// 窗口

	int				m_nMincx;		// 窗口最小宽度
	int				m_nMincy;		// 窗口最小高度
	CCursor			m_cursorArrow, m_cursorNWSE, m_cursorNESW, m_cursorWE, m_cursorNS;

};

#endif // !defined(AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
