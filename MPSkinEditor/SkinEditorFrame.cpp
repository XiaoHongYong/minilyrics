// SkinEditorFrame.cpp: implementation of the CSkinEditorFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "MainFrm.h"
#include "SkinEditorFrame.h"

#include "PropertyBar.h"
#include "ObjListBar.h"

CSkinEditorFrame		*g_pActiveFrameOld = NULL;

class CUIObjClipboard
{
public:
	CUIObjClipboard()
	{
	}

	void SetClippedObj(CUIObject *pObj)
	{
		m_objProperties.clear();
		if (pObj)
		{
			pObj->EnumProperties(m_objProperties);
			m_strClassName = pObj->GetClassName();
		}
		else
		{
			m_strClassName.resize(0);
		}
	}

	bool IsEmpty() { return m_strClassName.empty(); }

	CUIObject *FromClipboard(CSkin *pSkin)
	{
		CUIObject	*pObj;
		V_Str		vStr;

		pObj = pSkin->GetSkinFactory()->CreateUIObject(pSkin, m_strClassName.c_str());
		if (pObj)
		{
			m_objProperties.ToUIObjProperties(vStr);

			pObj->SetProperties(vStr);
		}
		return pObj;
	}

protected:
	tstring				m_strClassName;
	CUIObjProperties	m_objProperties;

};

CUIObjClipboard		g_skinObjClipboard;

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame

IMPLEMENT_DYNCREATE(CSkinEditorFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CSkinEditorFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CSkinEditorFrame)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_INSERT_BTN, OnInsertBtn)
	ON_COMMAND(IDC_PROPERTY, OnProperty)
	ON_COMMAND(IDC_DELETE, OnDelete)
	ON_COMMAND(IDC_INSERT_FOCUS_BTN, OnInsertFocusBtn)
	ON_COMMAND(IDC_INSERT_IMG, OnInsertImg)
	ON_COMMAND(IDC_INSERT_FOCUS_IMG, OnInsertFocusImg)
	ON_COMMAND(IDC_INSERT_XSCALE_IMG, OnInsertXscaleImg)
	ON_COMMAND(IDC_INSERT_YSCALE_IMG, OnInsertYscaleImg)
	ON_COMMAND(IDC_INSERT_VSCROLLBAR, OnInsertVscrollbar)
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDC_PROPERTY_SAVE, OnPropertySave)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(IDC_ROOM_IN, OnRoomIn)
	ON_COMMAND(IDC_ROOM_OUT, OnRoomOut)
	ON_WM_KILLFOCUS()
	ON_COMMAND(IDC_UPDATE_SKIN_OBJ_LIST, OnUpdateSkinObjList)
	ON_COMMAND(IDC_SKIN_OBJ_LIST_SEL_CHANGED, OnSkinObjListSelChanged)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(IDC_REDO, OnRedo)
	ON_WM_SYSCOMMAND()
	ON_WM_CHILDACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame construction/destruction

CSkinEditorFrame::CSkinEditorFrame()
{
	// TODO: add member initialization code here
	
}

CSkinEditorFrame::~CSkinEditorFrame()
{
}


void CSkinEditorFrame::OnSkinWndNameChanged()
{
	SetTitle(m_wndSkin.GetSkinWndName());
	AfxSetWindowText(m_hWnd, m_wndSkin.GetSkinWndName());

	CMainFrame	*pMainFrm = (CMainFrame *)AfxGetMainWnd();

	pMainFrm->GetMDITab()->Update();
}

HTREEITEM FindItemInSkinUIObjTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, DWORD dwItemData)
{
	HTREEITEM	hChild;

	hChild = pTree->GetChildItem(hParent);
	while (hChild)
	{
		if (pTree->GetItemData(hChild) == dwItemData)
			return hChild;
		if (pTree->GetChildItem(hChild))
		{
			HTREEITEM	hTemp;
			hTemp = FindItemInSkinUIObjTreeCtrl(pTree, hChild, dwItemData);
			if (hTemp)
				return hTemp;
		}

		hChild = pTree->GetNextSiblingItem(hChild);
	}

	return NULL;
}

// when the UIObject name is changed, update the tree control.
void CSkinEditorFrame::OnUIObjNameChanged(CUIObject *pObj)
{
	CTreeCtrl		*pTree = GetSkinUIObjTreeCtrl();
	if (!pTree)
		return;

	HTREEITEM	hItem = FindItemInSkinUIObjTreeCtrl(pTree, TVI_ROOT, (DWORD)pObj);
	if (hItem)
	{
		pTree->SetItemText(hItem, CStrPrintf(_T("%s (%s)"), pObj->m_strName.c_str(), pObj->GetClassName()).c_str());
	}
}

void CSkinEditorFrame::OnUIObjectListChanged()
{
	UpdateSkinObjList();
}

void CSkinEditorFrame::OnUIObjFocusChanged()
{
	UpdateToPropertyListCtrl();
}

void CSkinEditorFrame::OnFocusUIObjPropertyChanged(LPCTSTR szPropertyName, LPCTSTR szNewValue)
{
	if (!CPropertyBar::ms_pwndProperty)
		return;

	CUIObject				*pObjFocus;
	pObjFocus = m_wndSkin.GetFocusUIObj();

	if (!g_seSkinPropertyAdapter.IsSrcChanged(&m_wndSkin, pObjFocus))
	{
		CPropertyBar::ms_pwndProperty->SetProperty(szPropertyName, szNewValue);
	}
}

void CSkinEditorFrame::TrackPopupMenu(HMENU hMenu, int x, int y)
{
	::TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, m_hWnd, NULL);
}

BOOL CSkinEditorFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame diagnostics

#ifdef _DEBUG
void CSkinEditorFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CSkinEditorFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame message handlers
void CSkinEditorFrame::OnFileClose() 
{
	// To close the frame, just send a WM_CLOSE, which is the equivalent
	// choosing close from the system menu.

	SendMessage(WM_CLOSE);
}

int CSkinEditorFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

//	SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)), true);
//	SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)), false);
	SetClassLong(m_hWnd, GCL_HICONSM, (LONG)LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));
	SetClassLong(m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));

	m_wndSkin.SetNotify(this);

	m_wndSkin.Create(_T("MPSkin"), _T("MPSkin"), &g_skinFactory, _T(""), CBaseWnd::FromHandle(m_hWnd));

	m_wndSkin.MoveWindow(5, 5, 640, 480);

	g_skinPrj.OnCreateSkinDataItem(this);

	return 0;
}

void CSkinEditorFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect		rc;

	GetClientRect(&rc);

	dc.FillSolidRect(&rc, RGB(255, 255, 255));
}

void CSkinEditorFrame::OnDestroy() 
{
	g_skinPrj.OnDestroySkinDataItem(this);

	m_wndSkin.Destroy();

	CMDIChildWnd::OnDestroy();
}

void CSkinEditorFrame::UpdatePropertyItem(LPCTSTR szPropName, LPCTSTR szValue)
{
	if (!CPropertyBar::ms_pwndProperty)
		return;

	CPropertyBar::ms_pwndProperty->SetProperty(szPropName, szValue);
}

bool SelectItemInPropertyTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, CUIObject *pObjFocus)
{
	HTREEITEM	hChild = NULL;

	hChild = pTree->GetChildItem(hParent);
	while (hChild)
	{
		if (pTree->GetItemData(hChild) == (DWORD)pObjFocus)
		{
			pTree->SelectItem(hChild);

			return true;
		}
		if (SelectItemInPropertyTreeCtrl(pTree, hChild, pObjFocus))
			return true;

		hChild = pTree->GetNextSiblingItem(hChild);
	}

	return false;
}

void CSkinEditorFrame::UpdateToPropertyListCtrl()
{
	if (!CPropertyBar::ms_pwndProperty)
		return;

	CUIObject				*pObjFocus;
	CUIObjProperties		properties;
	CTreeCtrl				*pTree = GetSkinUIObjTreeCtrl();

	pObjFocus = m_wndSkin.GetFocusUIObj();

	if (pTree)
	{
		// Select focus UIObj in Skin Object tree ctrl
		HTREEITEM	hItem = pTree->GetSelectedItem();
		if (hItem == NULL || pTree->GetItemData(hItem) != (DWORD)pObjFocus)
		{
			pTree->Select(hItem, 0);

			SelectItemInPropertyTreeCtrl(pTree, TVI_ROOT, m_wndSkin.GetFocusUIObj());
		}
	}

	g_seSkinPropertyAdapter.SetSrc(&m_wndSkin, pObjFocus, m_hWnd);

	if (pObjFocus)
	{
		pObjFocus->EnumProperties(properties);
	}
	else
	{
		m_wndSkin.EnumProperties(properties);
	}

	CPropertyBar::ms_pwndProperty->ResetContent();

	///////////////////////////////////////
	// Add the Properties
	CPropertyBar::ms_pwndProperty->AddProperties(properties);
}

void CSkinEditorFrame::SetPropertiesToUIObj()
{
	if (!CPropertyBar::ms_pwndProperty)
		return;

	CUIObject		*pObjFocus;
	vector<tstring>	vProperties;

	pObjFocus = m_wndSkin.GetFocusUIObj();

	if (g_seSkinPropertyAdapter.IsSrcChanged(&m_wndSkin, pObjFocus))
		return;

	CPropertyBar::ms_pwndProperty->GetProperties(vProperties);

	if (pObjFocus)
	{
		pObjFocus->SetProperties(vProperties);

		m_wndSkin.RecalculateSizePos();

		m_wndSkin.InvalidateRect(NULL, TRUE);
	}
	else
	{
		// Set skin's properties.
		for (UINT i = 0; i < vProperties.size(); i += 2)
			m_wndSkin.SetProperty(vProperties[i].c_str(), vProperties[i + 1].c_str());
	}
}

void CSkinEditorFrame::OnProperty() 
{
	UpdateToPropertyListCtrl();
}

void CSkinEditorFrame::InsertUIObj(LPCTSTR szClassName)
{
	CUIObject *pObj = g_skinFactory.CreateUIObject(&m_wndSkin, szClassName);
	m_wndSkin.InsertUIObject(pObj);
}

void CSkinEditorFrame::OnInsertBtn() 
{
	InsertUIObj(CSkinButton::ClassName());
}

void CSkinEditorFrame::OnInsertFocusBtn() 
{
	InsertUIObj(CSkinActiveButton::ClassName());
}

void CSkinEditorFrame::OnInsertImg() 
{
	InsertUIObj(CSkinImage::ClassName());
}

void CSkinEditorFrame::OnInsertFocusImg() 
{
	InsertUIObj(CSkinActiveImage::ClassName());
}

void CSkinEditorFrame::OnInsertXscaleImg() 
{
	InsertUIObj(CSkinXScaleImage::ClassName());
}

void CSkinEditorFrame::OnInsertYscaleImg() 
{
	InsertUIObj(CSkinYScaleImage::ClassName());
}

void CSkinEditorFrame::OnInsertVscrollbar() 
{
	InsertUIObj(CSkinVScrollBar::ClassName());
}

void CSkinEditorFrame::OnSetFocus(CWnd* pOldWnd) 
{
	::SetFocus(m_wndSkin.GetHandle());
}


BOOL CSkinEditorFrame::OnEraseBkgnd(CDC* pDC) 
{
	CRect	rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(0, 0, rc.Width(), rc.Height(), GetSysColor(COLOR_3DSHADOW));
	
	return TRUE;
	// return CMDIChildWnd::OnEraseBkgnd(pDC);
}

void CSkinEditorFrame::OnPropertySave() 
{
	SetPropertiesToUIObj();
}

void CSkinEditorFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// SetFocus();
	m_wndSkin.SetNOFocusUIObj();

	OnProperty();
	
	CMDIChildWnd::OnLButtonDown(nFlags, point);
}

LRESULT CSkinEditorFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == SNM_UPDATEPROPERTYITEM)
	{
		UpdatePropertyItem((LPCTSTR)wParam, (LPCTSTR)lParam);
	}
	
	return CMDIChildWnd::WindowProc(message, wParam, lParam);
}

void CSkinEditorFrame::OnRoomIn() 
{
	m_wndSkin.ZoomIn();
}

void CSkinEditorFrame::OnRoomOut() 
{
	m_wndSkin.ZoomOut();
}

void CSkinEditorFrame::OnKillFocus(CWnd* pNewWnd) 
{
	CMDIChildWnd::OnKillFocus(pNewWnd);

/*	CPropertyBar::ms_pObjFocus = NULL;
	g_seSkinPropertyAdapter.m_hWndCommand = NULL;
	CPropertyBar::ms_pwndProperty->ResetContent();*/
}

void AddSkinObjToTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, CSkinContainer *pConainter)
{
	if (!pConainter)
		return;

	HTREEITEM		hChild = NULL;
	int		nCount = pConainter->GetChildrenCount();

	for (int i = 0; i < nCount; i++)
	{
		CUIObject	*pObj = pConainter->GetChildByIndex(i);
		if (pObj)
		{
			hChild = pTree->InsertItem(CStrPrintf(_T("%s (%s)"), pObj->m_strName.c_str(), pObj->GetClassName()).c_str(), 
				hParent, TVI_LAST);
			pTree->SetItemData(hChild, (DWORD)pObj);

			if (pObj->IsContainer())
			{
				AddSkinObjToTreeCtrl(pTree, hChild, pObj->GetContainerIf());
			}
		}
	}
	pTree->Expand(hParent, TVE_EXPAND);
}

void CSkinEditorFrame::UpdateSkinObjList()
{
	CTreeCtrl		*pTree = GetSkinUIObjTreeCtrl();

	if (!pTree)
		return;

	pTree->DeleteAllItems();

	CSkinContainer	*pConainter = m_wndSkin.GetRootContainer();

	AddSkinObjToTreeCtrl(pTree, TVI_ROOT, pConainter);
}

void CSkinEditorFrame::OnUpdateSkinObjList() 
{
	UpdateSkinObjList();
}

void CSkinEditorFrame::OnSkinObjListSelChanged() 
{
	CTreeCtrl		*pTree = GetSkinUIObjTreeCtrl();
	if (!pTree)
		return;

	HTREEITEM		hSel = pTree->GetSelectedItem();
	DWORD			dwSelData = pTree->GetItemData(hSel);
	CUIObject		*pObj = m_wndSkin.GetFocusUIObj();

	if (dwSelData == (DWORD)pObj)
		return;

	// selected UIObject changed, change the focus to it.
	if (dwSelData && m_wndSkin.IsUIObjectExist((CUIObject*)dwSelData))
	{
		m_wndSkin.SetNOFocusUIObj();

		pObj = (CUIObject *)dwSelData;
		pObj->SetFocus();
	}

	OnProperty();
}

void CSkinEditorFrame::OnEditCopy() 
{
	CUIObject		*pObjFocus;
	pObjFocus = m_wndSkin.GetFocusUIObj();
	if (pObjFocus)
		g_skinObjClipboard.SetClippedObj(pObjFocus);
}

void CSkinEditorFrame::OnEditCut() 
{
	OnEditCopy();
	OnDelete();
}

void CSkinEditorFrame::OnDelete() 
{
	m_wndSkin.RemoveFocusUIObject();
}

void CSkinEditorFrame::OnEditPaste() 
{
	CUIObject	*pObj = g_skinObjClipboard.FromClipboard(&m_wndSkin);
	m_wndSkin.InsertUIObject(pObj);
}

void CSkinEditorFrame::OnEditUndo() 
{
	// TODO: Add your command handler code here
	
}

void CSkinEditorFrame::OnRedo() 
{
	// TODO: Add your command handler code here
	
}

void CSkinEditorFrame::OnSave()
{
	CXMLWriter		xmlStream;
	CSimpleXML		xml;

	m_wndSkin.ToXML(xmlStream);
	if (xml.ParseData((void *)xmlStream.GetData(), xmlStream.GetLength()))
	{
		g_skinFactory.GetSkinFile()->SetSkinWndNode(m_wndSkin.GetSkinWndName(), xml.m_pRoot);
		xml.m_pRoot = NULL;
	}
	else
	{
		SaveDataAsFile(_T("C:\\1.xml"), xmlStream.GetData(), xmlStream.GetLength());
		ERRLOG0(_T("Parse Skin Wnd Node xml FAILED."));
	}
}

void CSkinEditorFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		return;
	
	CMDIChildWnd::OnSysCommand(nID, lParam);
}

void CSkinEditorFrame::OnChildActivate() 
{
	CMDIChildWnd::OnChildActivate();

	if (g_seSkinPropertyAdapter.IsSrcChanged(&m_wndSkin, m_wndSkin.GetFocusUIObj()))
		OnProperty();

	if (g_pActiveFrameOld != this)
	{
		UpdateSkinObjList();
	}
}
