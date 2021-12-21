// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DebugTracer.h"
#include "log.h"
#include "Profile.h"

#include "MainFrm.h"
#include "afxpriv.h"

#define	COL_ROW			0
#define	COL_TIME		1
#define	COL_SRC_FILE	2
#define	COL_SRC_LINE	3
#define	COL_LOG_LVL		4
#define	COL_LOG_MSG		5
#define	COL_ERR_MOD		6

#define		_MAX_LOG_LVL		7
LPCTSTR		_SZ_LOG_LVL[] =
{
	"UNDEFINE",
	"ERROR_FATAL",		// LOG_LVL_ERROR_FATAL			= 1,
	"ERROR",			// LOG_LVL_ERROR				= 2,
	"ERROR_MINOR",		// LOG_LVL_ERROR_MINOR			= 3,
	"WARNING",			// LOG_LVL_WARNING				= 4,
	"EVENT_IMP",		// LOG_LVL_EVENT_IMP			= 5,
	"EVENT_NORMAL",		// LOG_LVL_EVENT_NORMAL			= 6,
	"DEBUG",			// LOG_LVL_DEBUG				= 7,
};

LPCTSTR	LogLvlToStr(int nLvl)
{
	// 日志的等级定义
	if (nLvl <= 0 || nLvl > _MAX_LOG_LVL)
		return _SZ_LOG_LVL[0];

	return _SZ_LOG_LVL[nLvl];
}
/*
LPCTSTR		_SZ_COL_NAME[] = 
{
	"源文件",
	"代码行",
	"日志级别",
	"消息",
	NULL
};*/

typedef struct LIST_CTRL_COLOUM
{
	LPCTSTR		szName;
	int			nWidth;
	BOOL		bShow;
	int			nIndex;
	UINT		uMenuId;	// 菜单ID
}LIST_CTRL_COLOUM;

LIST_CTRL_COLOUM	_COL_LIST_CTRL[] =
{
	{
		"序号",
		40,
		TRUE,
		0,
		0
	},
	{
		"时间",
		90,
		TRUE,
		1,
		IDC_VIEW_TIME
	},
	{
		"源文件",
		120,
		TRUE,
		2,
		IDC_VIEW_SRC_FILE
	},
	{
		"代码行",
		50,
		TRUE,
		3,
		IDC_VIEW_SRC_LINE
	},
	{
		"日志级别",
		70,
		TRUE,
		4,
		IDC_VIEW_LOG_LVL
	},
	{
		"消息",
		430,
		TRUE,
		5,
		0
	},
	{
		NULL,
		0,
		TRUE,
		6,
		0
	}
};

// 安全的字符串拷贝，如果超出长度范围，则进行截断
// 并且将结尾设置为以'\0'结束
int	strncpysz_safe(char *strDestination, int nLenMax, const char *strSource, int nToCopy)
{
	if (nToCopy >= nLenMax)
		nToCopy = nLenMax - 1;

	strncpy(strDestination, strSource, nToCopy);
	strDestination[nToCopy] = '\0';

    return nToCopy;
}

BOOL CopyTextToClipboard(LPCTSTR szText, HWND hWnd)
{
	int		nLen;
    HGLOBAL hglbCopy;
	LPTSTR	lptstrCopy;
	
    if (!OpenClipboard(hWnd))
        return FALSE;
    EmptyClipboard();
	
	nLen = _tcslen(szText);
	
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * sizeof(TCHAR));
	
	if (hglbCopy == NULL) 
    {
        CloseClipboard();
        return FALSE;
    }
	
	lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, szText,
		nLen * sizeof(TCHAR));
	lptstrCopy[nLen] = (TCHAR) 0;    // null character
	GlobalUnlock(hglbCopy);
	
	// Place the handle on the clipboard. 
	
#ifdef _UNICODE
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
	SetClipboardData(CF_TEXT, hglbCopy);
#endif

	CloseClipboard();

	return TRUE;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_COPYDATA()
	ON_COMMAND(IDC_VIEW_LOG_LVL, OnViewLogLvl)
	ON_COMMAND(IDC_VIEW_SRC_FILE, OnViewSrcFile)
	ON_COMMAND(IDC_VIEW_SRC_LINE, OnViewSrcLine)
	ON_COMMAND(IDC_VIEW_TIME, OnViewTime)
	ON_COMMAND(IDC_OPEN, OnOpen)
	ON_COMMAND(IDC_ALWAYS_ONTOP, OnAlwaysOntop)
	ON_COMMAND(IDC_PAUSE, OnPause)
	ON_COMMAND(IDC_PAUSE_DROP_LOG, OnPauseDropLog)
	ON_COMMAND(IDC_CLEAR_LOG, OnClearLog)
	ON_COMMAND(IDC_SCROLL_LOG, OnScrollLog)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(IDC_VIEW_FILENAME_ONLY, OnViewFilenameOnly)
	//}}AFX_MSG_MAP
	ON_MESSAGE(MYWM_ADD_MSG, OnLogBuffer)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	RegisterWindowClass();

	m_bTopmost = FALSE;
	m_bScroll = TRUE;
	m_PauseState = PS_NORMAL;
	m_bShowOnlyFileName = true;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
//	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
//		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	if (!m_wndView.Create(LVS_REPORT | AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndView.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	for (int i = 0; _COL_LIST_CTRL[i].szName != NULL; i++)
		m_wndView.InsertColumn(_COL_LIST_CTRL[i].nIndex, _COL_LIST_CTRL[i].szName, LVCFMT_LEFT, _COL_LIST_CTRL[i].nWidth, i);

	SetWindowText(DEBUGWNDNAME);

	PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	
	HICON	hIcon;

	hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = DEBUGCLASSNAME;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
//	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
//		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL CMainFrame::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();
    //HINSTANCE hInst = AfxGetResourceHandle();

    if (!(::GetClassInfo(hInst, DEBUGCLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
#ifndef _WIN32_WCE_NO_CURSOR
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
#else
        wndcls.hCursor          = 0;
#endif
        wndcls.hbrBackground    = NULL; //(HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = DEBUGCLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CMainFrame::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	COPYDATASTRUCT *pLogData;
	pLogData = (COPYDATASTRUCT *)pCopyDataStruct;
	// m_wndView.OnDebugMsg(pLogData);
	
	if (pLogData->dwData == SENDDEBUGMESSAGE)
	{
		SYSTEMTIME	CurTime;
		LPTSTR		szLog;

		szLog = (LPTSTR)pLogData->lpData;
		szLog[pLogData->cbData - 1] = '\0';

		GetLocalTime(&CurTime);
		
		if (m_PauseState != PS_PAUSE_DROP)
		{
			m_vLogMsg.push_back(szLog);

			//
			// 因为WM_COPYDADA会让发送者等待，所有为了不影响发送者，我们先将此消息存储起来，通过postmessage来处理
			//
			if (m_PauseState == PS_NORMAL)
				PostMessage(MYWM_ADD_MSG, 0, 0);
		}
	}
	return CFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CMainFrame::OnViewLogLvl() 
{
	ViewColumnChanged(COL_LOG_LVL);
}

void CMainFrame::OnViewSrcFile() 
{
	ViewColumnChanged(COL_SRC_FILE);
}

void CMainFrame::OnViewSrcLine() 
{
	ViewColumnChanged(COL_SRC_LINE);
}

void CMainFrame::OnViewTime() 
{
	ViewColumnChanged(COL_TIME);
}

void CMainFrame::OnViewFilenameOnly() 
{
	m_bShowOnlyFileName = !m_bShowOnlyFileName;
}

void CMainFrame::ViewColumnChanged(int nColumn)
{
	int		nIndex = 0;
	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	_COL_LIST_CTRL[nColumn].bShow = !_COL_LIST_CTRL[nColumn].bShow;

	//
	// 设置菜单的Check标志
	//
	if (_COL_LIST_CTRL[nColumn].bShow)
		dwFlag |= MF_CHECKED;
	else
		dwFlag |= MF_UNCHECKED;

	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(2);

	pMenu->CheckMenuItem(_COL_LIST_CTRL[nColumn].uMenuId, dwFlag);


	//
	// 增加、删除相应的列
	//
	if (_COL_LIST_CTRL[nColumn].bShow)
		m_wndView.InsertColumn(_COL_LIST_CTRL[nColumn].nIndex, _COL_LIST_CTRL[nColumn].szName, LVCFMT_LEFT, _COL_LIST_CTRL[nColumn].nWidth);
	else
		m_wndView.DeleteColumn(_COL_LIST_CTRL[nColumn].nIndex);

	for (int i = 0; _COL_LIST_CTRL[i].szName != NULL; i++)
	{
		_COL_LIST_CTRL[i].nIndex = nIndex;
		if (_COL_LIST_CTRL[i].bShow)
			nIndex++;
	}
}

//
// 将缓冲区中的日志都添加到窗口中
//
LRESULT CMainFrame::OnLogBuffer(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < m_vLogMsg.size(); i++)
	{
		AnanysisLogMsg((LPTSTR)m_vLogMsg[i].c_str());
	}

	m_vLogMsg.clear();

	return 0;
}

//
// 打开日志文件
//
void CMainFrame::OnOpen() 
{
	FILE		*fp;
	char		*szBuffer;
	int			nLen;
	char		szFile[MAX_PATH];

	g_Profile.GetString("DebugTracer", "OpenLogFile", "", szFile, MAX_PATH);

	CFileDialog		dlg(TRUE, "Log Files(*.txt)|*.txt|All Files (*.*)|*.*||", szFile);
	if (dlg.DoModal() != IDOK)
		return;

	fp = fopen(dlg.GetPathName(), "r");
	if (fp == NULL)
	{
		AfxMessageBox("打开文件失败！");
		return;
	}

	g_Profile.WriteString("DebugTracer", "OpenLogFile", dlg.GetPathName());

	// fseek(fp, 0, SEEK_END);
	// nLen = ftell(fp);
	// fseek(fp, 0, SEEK_SET);
	nLen = 2048;

	szBuffer = new char[nLen + 1];
	ASSERT(szBuffer);
	if (szBuffer == NULL)
	{
		fclose(fp);
		AfxMessageBox("分配内存出错！");
		return;
	}

	m_wndView.DeleteAllItems();

	while (fgets(szBuffer, nLen, fp))
	{
		// 分析每一行
		AnanysisLogMsg(szBuffer);
	}

	delete szBuffer;
	if (ferror(fp))
	{
		fclose(fp);
		AfxMessageBox("读文件失败！");
		return;
	}

	fclose(fp);
}

void CMainFrame::AddTree(DWORD dwTreeId, DWORD dwParentId, DWORD CurId, LPCTSTR szName)
{
	if (m_dlgTree.m_hWnd)
	{
		if (!::IsWindowVisible(m_dlgTree.m_hWnd))
			m_dlgTree.ShowWindow(SW_SHOW);
	}
	else
		m_dlgTree.Create(IDD_DBG_TREE, this);

	m_dlgTree.AddTree(dwTreeId, dwParentId, CurId, szName);
}

//
// 将日志添加到窗口中
//
void CMainFrame::InsertLog(LPCTSTR szTime, LPCTSTR szSrcFile, int nLine, int nLevel, LPCTSTR szMsg)
{
	int			nIndex = -1;
	char		szText[256];

	if (_COL_LIST_CTRL[COL_ROW].bShow)
	{
		sprintf(szText, "%d", m_wndView.GetItemCount());
		if (nIndex == -1)
			nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), szText);
		else
			m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_ROW].nIndex, szText);
	}

	if (_COL_LIST_CTRL[COL_TIME].bShow)
	{
		if (nIndex == -1)
			nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), szTime);
		else
			m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_TIME].nIndex, szTime);
	}

	if (_COL_LIST_CTRL[COL_SRC_FILE].bShow)
	{
		if (m_bShowOnlyFileName)
		{
			LPCTSTR	szFileName;
			szFileName = strrchr(szSrcFile, '\\');
			if (szFileName)
				szSrcFile = szFileName + 1;
		}

		if (nIndex == -1)
			nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), szSrcFile);
		else
			m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_SRC_FILE].nIndex, szSrcFile);
	}

	if (_COL_LIST_CTRL[COL_SRC_LINE].bShow)
	{
		sprintf(szText, "%d", nLine);
		if (nIndex == -1)
			nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), szText);
		else
			m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_SRC_LINE].nIndex, szText);
	}

	if (_COL_LIST_CTRL[COL_LOG_LVL].bShow)
	{
		if (nIndex == -1)
			nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), LogLvlToStr(nLevel));
		else
			m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_LOG_LVL].nIndex, LogLvlToStr(nLevel));
	}

	if (_COL_LIST_CTRL[COL_LOG_MSG].bShow)
	{
		TCHAR	szTemp[100];
		int		nLen = strlen(szMsg);
		BOOL	bInsert;
		if (nIndex == -1)
			bInsert = TRUE;
		else
			bInsert = FALSE;

		for (int i = 0; i < nLen;)
		{
			int		nTocpy;
			LPCTSTR	szPos;
			int		nReturn = 0;
			szPos = strchr(szMsg + i, '\n');
			if (szPos)
			{
				nTocpy = szPos - (szMsg + i);
				if (nTocpy > 80)
					nTocpy = 80;
				else
					nReturn = 1;
			}
			else
			{
				if (i + 80 > nLen)
					nTocpy = nLen - i;
				else
					nTocpy = 80;
			}
			strncpysz_safe(szTemp, 100, szMsg + i, nTocpy);
			if (bInsert)
			{
				m_wndView.InsertItem(m_wndView.GetItemCount(), szTemp);
			}
			else
			{
				m_wndView.SetItemText(nIndex, _COL_LIST_CTRL[COL_LOG_MSG].nIndex, szTemp);
			}
			if (i + nTocpy < nLen && !bInsert)
			{
				nIndex = m_wndView.InsertItem(m_wndView.GetItemCount(), "");
			}
			i += nTocpy + nReturn;
		}
	}

	if (m_bScroll)
		m_wndView.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

//
// 分析一行日志，并且添加到窗口中
//
void CMainFrame::AnanysisLogMsg(LPTSTR szBuffer)
{
	// 分析每一行
	// |F%s|L%d|E%d|M%s
	LPTSTR		szTime = "", szSrcFile = "", szLine = "", szErrLevel = "", szMsg = "";
	LPTSTR		szPtr;

	szPtr = szBuffer;
	if (szPtr[0] == '|' && szPtr[1] == 'T')
	{
		*szPtr = '\0';
		szPtr += 2;
		szTime = szPtr;
		while (*szPtr != '\0' && *szPtr != '|')
			szPtr++;
	}

	if (szPtr[0] == '|' && szPtr[1] == 'F')
	{
		*szPtr = '\0';
		szPtr += 2;
		szSrcFile = szPtr;
		while (*szPtr != '\0' && *szPtr != '|')
			szPtr++;
	}

	if (szPtr[0] == '|' && szPtr[1] == 'L')
	{
		*szPtr = '\0';
		szPtr += 2;
		szLine = szPtr;
		while (*szPtr != '\0' && *szPtr != '|')
			szPtr++;
	}

	if (szPtr[0] == '|' && szPtr[1] == 'E')
	{
		*szPtr = '\0';
		szPtr += 2;
		szErrLevel = szPtr;
		while (*szPtr != '\0' && *szPtr != '|')
			szPtr++;
	}

	if (szPtr[0] == '|' && szPtr[1] == 'M')
	{
		*szPtr = '\0';
		szPtr += 2;
		szMsg = szPtr;
		while (*szPtr != '\0')
			szPtr++;
	}
	if (*(szPtr - 1) == '\n')
	{
		szPtr--;
		if (*(szPtr - 1) == '\0')
			szPtr--;
		*szPtr = '\0';
	}

	InsertLog(szTime, szSrcFile, atoi(szLine), atoi(szErrLevel), szMsg);


	// tree message
	// |Ttid pid id name
	szPtr = szMsg;
	if (szPtr[0] == '|' && szPtr[1] == 'T')
	{
		szPtr += 2;
		DWORD		dwTreeId, dwParentId, CurId;
		LPCTSTR		szName;
		dwTreeId = atoi(szPtr);
		while (*szPtr && *szPtr != ' ')
			szPtr++;
		if (szPtr)
		{
			szPtr++;
			dwParentId = atoi(szPtr);
			while (*szPtr && *szPtr != ' ')
				szPtr++;
			if (szPtr)
			{
				szPtr++;
				CurId = atoi(szPtr);
				while (*szPtr && *szPtr != ' ')
					szPtr++;
				if (szPtr)
				{
					szPtr++;
					szName = szPtr;
				}
				else
					szName = "";

				AddTree(dwTreeId, dwParentId, CurId, szName);
			}
		}
	}
}

void TopmostWindow(HWND hwnd, BOOL bTopmost)
{
	DWORD	dwStyleEx;
	if (bTopmost)
	{
		dwStyleEx = (DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE);
		dwStyleEx |= WS_EX_TOPMOST;
		::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		dwStyleEx = (DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE);
		dwStyleEx &= ~WS_EX_TOPMOST;
		::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

void CMainFrame::OnAlwaysOntop() 
{
	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	m_bTopmost = !m_bTopmost;
	if (m_bTopmost)
		dwFlag |= MF_CHECKED;
	else
		dwFlag |= MF_UNCHECKED;

	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(2);

	pMenu->CheckMenuItem(IDC_ALWAYS_ONTOP, dwFlag);

	TopmostWindow(m_hWnd, m_bTopmost);
}

void CMainFrame::OnPause() 
{
	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	if (m_PauseState == PS_PAUSE)
	{
		m_PauseState = PS_NORMAL;
		PostMessage(MYWM_ADD_MSG, 0, 0);
		dwFlag |= MF_UNCHECKED;
	}
	else
	{
		m_PauseState = PS_PAUSE;
		dwFlag |= MF_CHECKED;
	}

	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_PAUSE, dwFlag);
	pMenu->CheckMenuItem(IDC_PAUSE_DROP_LOG, MF_BYCOMMAND | MF_UNCHECKED);
}

void CMainFrame::OnPauseDropLog() 
{
	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	if (m_PauseState == PS_PAUSE_DROP)
	{
		m_PauseState = PS_NORMAL;
		PostMessage(MYWM_ADD_MSG, 0, 0);
		dwFlag |= MF_UNCHECKED;
	}
	else
	{
		m_PauseState = PS_PAUSE_DROP;
		dwFlag |= MF_CHECKED;
	}

	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_PAUSE_DROP_LOG, dwFlag);
	pMenu->CheckMenuItem(IDC_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
}

void CMainFrame::OnClearLog() 
{
	m_wndView.DeleteAllItems();
}

void CMainFrame::OnScrollLog() 
{
	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	m_bScroll = !m_bScroll;
	if (m_bScroll)
		dwFlag |= MF_CHECKED;
	else
		dwFlag |= MF_UNCHECKED;

	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_SCROLL_LOG, dwFlag);
}

void CMainFrame::OnEditCopy() 
{
	//
	POSITION pos = m_wndView.GetFirstSelectedItemPosition();
	TCHAR		szBuff[512];
	CString		str;

	while (pos)
	{
		int			nItem;
		nItem = m_wndView.GetNextSelectedItem(pos);
		for (int i = 0; i < _MAX_LOG_LVL; i++)
		{
			if (!m_wndView.GetItemText(nItem, i, szBuff, 512))
				break;
			str += szBuff;
			str += "	";
		}
		str += "\r\n";
	}

	CopyTextToClipboard(str, m_hWnd);
}
