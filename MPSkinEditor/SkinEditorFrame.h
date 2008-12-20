// SkinEditorFrame.h: interface for the CSkinEditorFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_)
#define AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSkinEditorFrame : public CMDIChildWnd, public ISESkinNotify, public ISESkinDataItem
{
	DECLARE_DYNCREATE(CSkinEditorFrame)
public:
	CSkinEditorFrame();

	virtual void OnSkinWndNameChanged();
	virtual void OnUIObjNameChanged(CUIObject *pObj);
	virtual void OnUIObjectListChanged();
	virtual void OnUIObjFocusChanged();
	virtual void OnFocusUIObjPropertyChanged(LPCTSTR szPropertyName, LPCTSTR szNewValue);
	virtual void TrackPopupMenu(HMENU hMenu, int x, int y);

	virtual void OnSave();

// Attributes
public:

// Operations
public:
	void UpdateToPropertyListCtrl();
	void SetPropertiesToUIObj();

	void UpdatePropertyItem(LPCTSTR szPropName, LPCTSTR szValue);
	void UpdateSkinObjList();

	void InsertUIObj(LPCTSTR szClassName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinEditorFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	// view for the client area of the frame.
	CSESkinWnd		m_wndSkin;
	virtual ~CSkinEditorFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Generated message map functions
protected:
	//{{AFX_MSG(CSkinEditorFrame)
	afx_msg void OnFileClose();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnInsertBtn();
	afx_msg void OnProperty();
	afx_msg void OnDelete();
	afx_msg void OnInsertFocusBtn();
	afx_msg void OnInsertImg();
	afx_msg void OnInsertFocusImg();
	afx_msg void OnInsertXscaleImg();
	afx_msg void OnInsertYscaleImg();
	afx_msg void OnInsertVscrollbar();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPropertySave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRoomIn();
	afx_msg void OnRoomOut();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnUpdateSkinObjList();
	afx_msg void OnSkinObjListSelChanged();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnRedo();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnChildActivate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_)
