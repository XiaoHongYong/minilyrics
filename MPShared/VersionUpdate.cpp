// VersionUpdate.cpp: implementation of the CVersionUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#ifdef _WIN32_DESKTOP
#include <time.h>
#endif

#ifdef _MPLAYER
#define SOFTWARE_INFO		_T("zikiplayerinfo")
#define VER_FILE			_T("zpver.xml")
#else
#define SOFTWARE_INFO		_T("minilyricsinfo")
#define VER_FILE			_T("mlver.xml")
#endif

BOOL CVersion::From(LPCTSTR szVersion)
{
	int		nRet;
	nRet = _stscanf(szVersion, _T("%d.%d.%d"), &nVerMajor, &nVerMinor, &nVerBuild);
	if (nRet == 2)
		nVerBuild = 0;
	else if (nRet <= 1)
		return FALSE;

	return TRUE;
}

CVersionInfo::CVersionInfo()
{
}

//////////////////////////////////////////////////////////////////////////
// 从服务器取得VER_FILE的版本控制信息，根据此文件进行更新
// VER_FILE文件的格式

//<?xml version="1.0" encoding="gb2312"?>
//<SOFTWARE_INFO>
//  <curversion version="1.3" upgradedate="2002-9-12" obsoletever="1.4.3" TrDays="30">
//    <feature>
//    </feature>
//  </curversion>
//</SOFTWARE_INFO>

int CVersionInfo::FromXML(SXNode *pNodeVer)
{
	if (_tcscmp(pNodeVer->strName.c_str(), SOFTWARE_INFO) != 0)
		return ERR_FALSE;

	SXNode		*pNode;

	//
	// curversion
	//
	pNode = pNodeVer->GetChild(_T("curversion"));
	if (!pNode)
		return ERR_FALSE;

	LPCTSTR		szTemp;

	//
	// new version
	szTemp = pNode->GetPropertySafe(_T("version"));
	if (!verNew.From(szTemp))
		return ERR_FALSE;

	szTemp = pNode->GetProperty(_T("upgradedate"));
	if (szTemp)
		strNewVerDate = szTemp;

	//
	// new feature
	SXNode		*pFeatureNode;
	pFeatureNode = pNode->GetChild(_T("feature"));
	assert(pFeatureNode);
	if (pFeatureNode)
		strFeature = pFeatureNode->strContent.c_str();

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////


class CVersionUpdateDlg : public CBaseDialog
{
public:
	const TCHAR *	m_szVersionDesc;
	CCECommandBar	m_cmdBar;

	CVersionUpdateDlg() : CBaseDialog(IDD_NEW_VERSION_PROMPT)
	{
		m_szVersionDesc = NULL;
	};

	~CVersionUpdateDlg()	{};

	void OnCommand(UINT uID, UINT nNotifyCode)
	{
		CBaseDialog::OnCommand(uID, nNotifyCode);

		if (uID == IDC_VISITWEB)
		{
			OpenUrl(this, GetStrName(SN_HTTP_DOMAIN));
			OnOK();
		}
	}

	BOOL OnInitDialog()
	{
		CBaseDialog::OnInitDialog();

		m_LangDlgHelper.RemoveDlgItem(IDC_NEW_VERSION);

		if (m_szVersionDesc)
		{
			CWidgetEditBox	editBox;

			editBox.Attach(this, IDC_NEW_VERSION);

			editBox.SetWindowText(m_szVersionDesc);
		}

		CeSHInitFullScrDlg(this);

		m_cmdBar.Create(this, -1);

		return TRUE;
	}

	void OnDestroy()
	{
		m_cmdBar.Destroy();

		CBaseDialog::OnDestroy();
	}

};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVersionUpdate::CVersionUpdate()
{

}

CVersionUpdate::~CVersionUpdate()
{

}

void CVersionUpdate::OnDownloadOK(CDownloadTask *pTask)
{
	if (pTask->taskType != DTT_CHECK_VERSION)
		return;

	CVersionInfo	versionInfo;
	int				nRet;

	nRet = GetUpdateInfo(pTask, versionInfo);
	if (nRet != ERR_OK)
		return;

	CVersion		verCur(MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
	if (verCur < versionInfo.verNew)
	{
		CVersionUpdateDlg	dlg;

		dlg.m_szVersionDesc = versionInfo.strFeature.c_str();
		dlg.DoModal(g_mpApp.GetMainWnd());
	}
}

int CVersionUpdate::GetUpdateInfo(CDownloadTask *pTask, CVersionInfo &versionInfo)
{
	assert(pTask);

	TCHAR		szCheckVerTime[256];
	CSimpleXML	xml;
	time_t		lTime;
	tm			*curtime;

	time(&lTime);

	curtime = localtime(&lTime);

	//
	// 将检查更新时间写入配置文件中
	_stprintf_s(szCheckVerTime, CountOf(szCheckVerTime), _T("%d-%d-%d"), curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
	g_Profile.EncryptWriteProfileString(_T("CheckVerTime"), szCheckVerTime);

	// 分析VER_FILE
	if (!xml.ParseData(pTask->buffContent.c_str(), pTask->buffContent.size()))
	{
		ERRLOG1(_T("parse xml data FAILED: %s"), pTask->buffContent.c_str());
		return ERR_PARSE_XML;
	}

	if (versionInfo.FromXML(xml.m_pRoot) != ERR_OK)
	{
		ERRLOG1(_T("xml file data error: %s"), pTask->m_strSaveFile.c_str());
		return ERR_PARSE_XML;
	}

	return ERR_OK;
}

void CVersionUpdate::CheckNewVersion(bool bAutoCheck)
{
	if (bAutoCheck)
	{
		TCHAR		szCheckVerTime[256];
		int			wYear, wMonth, wDay;
		time_t		lTime;
		tm			*curtime;

		time(&lTime);

		curtime = localtime(&lTime);

		g_Profile.EncryptGetProfileString(_T("CheckVerTime"), _T(""), szCheckVerTime, CountOf(szCheckVerTime));
		if (_stscanf(szCheckVerTime, _T("%d-%d-%d"), &wYear, &wMonth, &wDay) < 3)
		{
			// no process
		}

		// 如果日期相隔7天，则检查更新
		if (abs(GetCurDaysFrom2000() - GetDaysFrom2000(wMonth, wDay, wYear)) <= 7)
			return;

		_stprintf_s(szCheckVerTime, CountOf(szCheckVerTime), _T("%d-%d-%d"), curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
		g_Profile.EncryptWriteProfileString(_T("CheckVerTime"), szCheckVerTime);
	}

	g_LyricsDownloader.DownloadVersionFile(GetStrName(SN_HTTP_ML_VER), !bAutoCheck);
}
