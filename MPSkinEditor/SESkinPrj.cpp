// SESkinPrj.cpp: implementation of the CSESkinPrj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SESkinPrj.h"
#include "MPSkinEditor.h"
#include "../MPShared/MLCmd.h"

#include "MainFrm.h"

CSESkinFactory		g_skinFactory;

CSESkinPrj			g_skinPrj;

CSEUIAdapter		g_seUIAdapter;

//////////////////////////////////////////////////////////////////////////
CSEUIAdapter::CSEUIAdapter()
{

}

CSEUIAdapter::~CSEUIAdapter()
{

}

void CSEUIAdapter::SetMousePointerInfo(int x, int y)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CStatusBar	*pStatusBar = pFrame->GetStatusBar();
	int			nIndex = 1;//pStatusBar->CommandToIndex(IDC_MOUSE_POINTER);
	pStatusBar->SetPaneStyle(nIndex, pStatusBar->GetPaneStyle(nIndex) & ~SBPS_DISABLED);
	pStatusBar->SetPaneText(nIndex, CStrPrintf(_T("X: %4d Y: %4d"), x, y).c_str(), TRUE);
//	pStatusBar->SetPaneText(2, CStrPrintf(_T("X: %4d Y: %4d"), x, y).c_str());
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinPrj::CSESkinPrj()
{
	g_skinFactory.SetStringIDMap(g_StringIdMap);
}

CSESkinPrj::~CSESkinPrj()
{

}

void GetBackupFile(LPCTSTR szDir, tstring &strLatestBackup, tstring &strNextBackup)
{
	V_Str	vFiles;
	int		nMax = 0, nMin = 0;
	int		i;
	LPCTSTR SZ_SKIN_FILE_PREFIX	= _T("zikiplayer-");
	int		nPrefixLen = _tcslen(SZ_SKIN_FILE_PREFIX);
	tstring	strDirOK = szDir;

	DirStringFormat(strDirOK);

	// Get Max and Min backup file
	EnumFilesInDir(szDir, _T("*.xml"), vFiles, false);
	for (i = 0; i < vFiles.size(); i++)
	{
		LPCTSTR		szFile = vFiles[i].c_str();
		if (_tcsnicmp(szFile, SZ_SKIN_FILE_PREFIX, nPrefixLen) == 0)
		{
			int		n = _ttoi(szFile + nPrefixLen);
			if (n > nMax)
				nMax = n;
			else if (n < nMin && n > 0)
				nMin = nMin;
		}
	}

	strLatestBackup = strDirOK + CStrPrintf(_T("%s%03d.xml"), SZ_SKIN_FILE_PREFIX, nMax).c_str();
	if (IsFileExist(strLatestBackup.c_str()))
		nMax++;
	strNextBackup = strDirOK + CStrPrintf(_T("%s%03d.xml"), SZ_SKIN_FILE_PREFIX, nMax).c_str();

	// remove too old backup file
	for (i = nMin; i <= nMax - 1000; i++)
	{
		DeleteFile(CStrPrintf(_T("%s%03d.xml"), SZ_SKIN_FILE_PREFIX, i).c_str());
	}
}

bool IsFileContentSame(LPCTSTR szFile1, LPCTSTR szFile2)
{
	CCharBuffer		buff1, buff2;
	if (!OpenFileToString(szFile1, buff1))
		return false;
	if (!OpenFileToString(szFile2, buff2))
		return false;

	if (buff1.size() != buff2.size())
		return false;

	if (memcmp(buff1.c_str(), buff2.c_str(), buff1.size()) == 0)
		return true;

	return false;
}

bool CSESkinPrj::IsModified()
{
	CSkinFileXML	*pSkinFile;
	tstring			strSkinFile, strSkinFileTemp;

	pSkinFile = g_skinFactory.GetSkinFile();
	strSkinFileTemp = GetWorkingFolder();
	strSkinFileTemp += _T("savetempskin.xml");
	strSkinFile = pSkinFile->GetFileName();

	// update skin data to xml data.
	for (int i = 0; i < m_vSkinDataItems.size(); i++)
	{
		m_vSkinDataItems[i]->OnSave();
	}

	// convert dynamic command node to xml data
	CDynamicCmds &dc = g_skinFactory.GetDynamicCmds();
	pSkinFile->SetDynamicCmdsNode(dc.ToXML());

	// convert dynamic control node to xml data
	CDynamicCtrls &dctrl = g_skinFactory.GetDynamicCtrls();
	pSkinFile->SetDynamicCtrlsNode(DupSXNode(dctrl.GetData()));

	if (pSkinFile->Save(strSkinFile.c_str()) != ERR_OK)
		return true;

	// Is current file same as content?
	return !IsFileContentSame(strSkinFileTemp.c_str(), strSkinFile.c_str());
}

int CSESkinPrj::Save()
{
	CSkinFileXML	*pSkinFile;
	TCHAR			szSkinBackupDir[MAX_PATH];
	tstring			strSkinFile;

	pSkinFile = g_skinFactory.GetSkinFile();
	strSkinFile = pSkinFile->GetFileName();

	ASSERT(!strSkinFile.empty());

	//
	// Backup old zikiplayer.xml first.
	//
	tstring			strLatestBackup, strNextBackup;

	FileGetPath(szSkinBackupDir, CountOf(szSkinBackupDir), strSkinFile.c_str());
	_tcscat(szSkinBackupDir, _T("backup\\"));

	GetBackupFile(szSkinBackupDir, strLatestBackup, strNextBackup);
	if (!IsDirExist(szSkinBackupDir))
		CreateDirectory(szSkinBackupDir, NULL);

	// Is current file same as latest backup?
	if (!IsFileContentSame(strLatestBackup.c_str(), strSkinFile.c_str()))
	{
		// backup current
		CopyFile(strSkinFile.c_str(), strNextBackup.c_str(), FALSE);
	}

	// update skin data to xml data.
	for (int i = 0; i < m_vSkinDataItems.size(); i++)
	{
		m_vSkinDataItems[i]->OnSave();
	}

	// convert dynamic command node to xml data
	CDynamicCmds &dc = g_skinFactory.GetDynamicCmds();
	pSkinFile->SetDynamicCmdsNode(dc.ToXML());

	// convert dynamic control node to xml data
	CDynamicCtrls &dctrl = g_skinFactory.GetDynamicCtrls();
	pSkinFile->SetDynamicCtrlsNode(DupSXNode(dctrl.GetData()));

	return pSkinFile->Save(strSkinFile.c_str());
}

void CSESkinPrj::Close()
{
	ASSERT(m_vSkinDataItems.size() == 0);
	m_vSkinDataItems.clear();
	g_skinFactory.Close();
}

void CSESkinPrj::OnCreateSkinDataItem(ISESkinDataItem *pSkinDataItem)
{
	for (int i = 0; i < m_vSkinDataItems.size(); i++)
	{
		if (m_vSkinDataItems[i] == pSkinDataItem)
			return;
	}

	m_vSkinDataItems.push_back(pSkinDataItem);
}

void CSESkinPrj::OnDestroySkinDataItem(ISESkinDataItem *pSkinDataItem)
{
	for (int i = 0; i < m_vSkinDataItems.size(); i++)
	{
		if (m_vSkinDataItems[i] == pSkinDataItem)
		{
			pSkinDataItem->OnSave();

			m_vSkinDataItems.erase(m_vSkinDataItems.begin() + i);
			return;
		}
	}
	ASSERT(0);
}
