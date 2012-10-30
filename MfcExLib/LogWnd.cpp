// LogWnd.cpp : implementation file
//

#include "stdafx.h"
#include "../base/base.h"
#include "../Utils/Utils.h"
#include "LogWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	COL_ROW			0
#define	COL_TIME		1
#define	COL_SRC_FILE	2
#define	COL_SRC_LINE	3
#define	COL_LOG_LVL		4
#define	COL_LOG_MSG		5
#define	COL_ERR_MOD		6

#define MAX_LOG_COUNT		5000

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

#define IDC_VIEW_TIME	0
#define IDC_VIEW_SRC_FILE	0
#define IDC_VIEW_SRC_LINE	0
#define IDC_VIEW_LOG_LVL	0

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
		200,
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

/////////////////////////////////////////////////////////////////////////////
// CLogWnd

CLogWnd::CLogWnd()
{
	m_bTopmost = FALSE;
	m_bScroll = TRUE;
	m_PauseState = PS_NORMAL;
	m_nShowLogLvl = LOG_LVL_DEBUG;

	m_bMaxLogLimit = TRUE;
}

CLogWnd::~CLogWnd()
{
}


BEGIN_MESSAGE_MAP(CLogWnd, CListCtrl)
	//{{AFX_MSG_MAP(CLogWnd)
	ON_WM_CREATE()
	ON_MESSAGE(MYWM_ADD_MSG, OnLogBuffer)
	ON_WM_COPYDATA()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogWnd message handlers

BOOL CLogWnd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
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
	return CListCtrl::OnCopyData(pWnd, pCopyDataStruct);
}

//
// 将缓冲区中的日志都添加到窗口中
//
LRESULT CLogWnd::OnLogBuffer(WPARAM wParam, LPARAM lParam)
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
void CLogWnd::OnOpen() 
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

	this->DeleteAllItems();

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

//
// 将日志添加到窗口中
//
void CLogWnd::InsertLog(LPCTSTR szTime, LPCTSTR szSrcFile, int nLine, int nLevel, LPCTSTR szMsg)
{
	int			nIndex = -1;
	char		szText[256];

	if (m_bMaxLogLimit && GetItemCount() >= MAX_LOG_COUNT)
	{
		// 删除一半
		for (int i = 0; i < MAX_LOG_COUNT / 2; i++)
			DeleteItem(0);
	}

	if (_COL_LIST_CTRL[COL_ROW].bShow)
	{
		sprintf(szText, "%d", this->GetItemCount() + 1);
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), szText);
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_ROW].nIndex, szText);
	}

	if (_COL_LIST_CTRL[COL_TIME].bShow)
	{
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), szTime);
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_TIME].nIndex, szTime);
	}

	if (_COL_LIST_CTRL[COL_SRC_FILE].bShow)
	{
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), szSrcFile);
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_SRC_FILE].nIndex, szSrcFile);
	}

	if (_COL_LIST_CTRL[COL_SRC_LINE].bShow)
	{
		sprintf(szText, "%d", nLine);
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), szText);
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_SRC_LINE].nIndex, szText);
	}

	if (_COL_LIST_CTRL[COL_LOG_LVL].bShow)
	{
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), LogLvlToStr(nLevel));
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_LOG_LVL].nIndex, LogLvlToStr(nLevel));
	}

	if (_COL_LIST_CTRL[COL_LOG_MSG].bShow)
	{
		if (nIndex == -1)
			nIndex = this->InsertItem(this->GetItemCount(), szMsg);
		else
			this->SetItemText(nIndex, _COL_LIST_CTRL[COL_LOG_MSG].nIndex, szMsg);
	}

	if (m_bScroll)
		this->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

//
// 分析一行日志，并且添加到窗口中
//
void CLogWnd::AnanysisLogMsg(LPTSTR szBuffer)
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
		while (*szPtr != '\0' && *szPtr != '|')
			szPtr++;
	}
	if (*(szPtr - 1) == '\n')
	{
		szPtr--;
		if (*(szPtr - 1) == '\0')
			szPtr--;
		*szPtr = '\0';
	}

	if (atoi(szErrLevel) <= m_nShowLogLvl)
		InsertLog(szTime, szSrcFile, atoi(szLine), atoi(szErrLevel), szMsg);
}

void CLogWnd::OnPause() 
{
//	CMenu	*pMenu;
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

/*	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_PAUSE, dwFlag);
	pMenu->CheckMenuItem(IDC_PAUSE_DROP_LOG, MF_BYCOMMAND | MF_UNCHECKED);*/
}

void CLogWnd::OnPauseDropLog() 
{
//	CMenu	*pMenu;
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

/*
	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_PAUSE_DROP_LOG, dwFlag);
	pMenu->CheckMenuItem(IDC_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
*/
}

void CLogWnd::OnClearLog() 
{
	this->DeleteAllItems();
}

void CLogWnd::OnScrollLog() 
{
//	CMenu	*pMenu;
	DWORD	dwFlag = MF_BYCOMMAND;

	m_bScroll = !m_bScroll;
	if (m_bScroll)
		dwFlag |= MF_CHECKED;
	else
		dwFlag |= MF_UNCHECKED;

/*
	pMenu = GetMenu();
	pMenu = pMenu->GetSubMenu(1);

	pMenu->CheckMenuItem(IDC_SCROLL_LOG, dwFlag);
*/
}

int CLogWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	

	for (int i = 0; _COL_LIST_CTRL[i].szName != NULL; i++)
		this->InsertColumn(_COL_LIST_CTRL[i].nIndex, _COL_LIST_CTRL[i].szName, LVCFMT_LEFT, _COL_LIST_CTRL[i].nWidth, i);
	
	return 0;
}

void CLogWnd::Init()
{
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_nShowLogLvl = g_Profile.GetInt(_T("LogWnd"), _T("ShowLogLevel"), LOG_LVL_DEBUG);
	m_bScroll = g_Profile.GetInt(_T("LogWnd"), _T("AutoScroll"), TRUE);
	m_bMaxLogLimit = g_Profile.GetInt(_T("LogWnd"), _T("MaxLogLimit"), TRUE);

	int		nIndex = 0;
	
	for (int i = 0; _COL_LIST_CTRL[i].szName != NULL; i++)
	{
		_COL_LIST_CTRL[i].bShow = g_Profile.GetInt(_T("LogWnd"), _COL_LIST_CTRL[i].szName, TRUE);
		_COL_LIST_CTRL[i].nIndex = nIndex;
		if (_COL_LIST_CTRL[i].bShow)
		{
			nIndex++;
			this->InsertColumn(_COL_LIST_CTRL[i].nIndex, _COL_LIST_CTRL[i].szName, LVCFMT_LEFT, _COL_LIST_CTRL[i].nWidth, i);
		}
	}
}


void CLogWnd::ViewColumnChanged(int nColumn)
{
	int		nIndex = 0;
/*	CMenu	*pMenu;
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

	pMenu->CheckMenuItem(_COL_LIST_CTRL[nColumn].uMenuId, dwFlag);*/


	//
	// 增加、删除相应的列
	//
	if (_COL_LIST_CTRL[nColumn].bShow)
		InsertColumn(_COL_LIST_CTRL[nColumn].nIndex, _COL_LIST_CTRL[nColumn].szName, LVCFMT_LEFT, _COL_LIST_CTRL[nColumn].nWidth);
	else
		DeleteColumn(_COL_LIST_CTRL[nColumn].nIndex);

	for (int i = 0; _COL_LIST_CTRL[i].szName != NULL; i++)
	{
		_COL_LIST_CTRL[i].nIndex = nIndex;
		if (_COL_LIST_CTRL[i].bShow)
			nIndex++;
	}
}


void CLogWnd::OnViewLogLvl() 
{
	ViewColumnChanged(COL_LOG_LVL);
}

void CLogWnd::OnViewSrcFile() 
{
	ViewColumnChanged(COL_SRC_FILE);
}

void CLogWnd::OnViewSrcLine() 
{
	ViewColumnChanged(COL_SRC_LINE);
}

void CLogWnd::OnViewTime() 
{
	ViewColumnChanged(COL_TIME);
}

void CLogWnd::OnDestroy() 
{
	CListCtrl::OnDestroy();
	
}
