#pragma once

class CSkinResizableLinearContainer : public CSkinLinearContainer  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CSkinResizableLinearContainer();
	virtual ~CSkinResizableLinearContainer();

	virtual bool OnMouseMove(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnLButtonUp(UINT nFlags, CPoint point);

	void OnSize();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	// Return -1, if NOT in resizer area
	int HitTestResizer(CPoint pt);

	void SetResizerCursor();

	void DragResizer(int &offset);

	static void ZoomFromRight(VecItems &vItems, int nSize);
	static void ZoomFromLeft(VecItems &vItems, int nSize);
	static void ZoomAll(VecItems &vItems, int nSize);

protected:
	CCursor					m_cursorWE, m_cursorNS;

	bool					m_bDragWnd;
	CPoint					m_ptDragOld;
	int						m_nCurrentResizerArea;	// Current drag resizer area, start from 0.

	int						m_nThicknessOfResizer;

};
