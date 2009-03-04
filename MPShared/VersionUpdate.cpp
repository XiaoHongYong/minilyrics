// VersionUpdate.cpp: implementation of the CVersionUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloadMgr.h"
#include "VersionUpdate.h"
#ifdef _WIN32_DESKTOP
#include "win32/OnlineMsgWnd.h"
#include <time.h>
#endif

#ifdef _MPLAYER
#define SOFTWARE_INFO		_T("zikiplayerinfo")
#else
#define SOFTWARE_INFO		_T("minilyricsinfo")
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
	nTrialDays = 0;
	bUseAffiliate = true;
	nPageWidth = 320;
	nPageHeight = 240;
}

//////////////////////////////////////////////////////////////////////////
// 从服务器取得/mlver.xml的版本控制信息，根据此文件进行更新
// mlver.xml文件的格式

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
	// Obsolete Version
	szTemp = pNode->GetProperty(_T("obsoletever"));
	if (szTemp)
	{
		if (!verObs.From(szTemp))
			return ERR_FALSE;
	}

	//
	// new version
	szTemp = pNode->GetProperty(_T("version"));
	if (!szTemp)
		return ERR_FALSE;
	if (!verNew.From(szTemp))
		return ERR_FALSE;

	strNewVerDlPage = pNode->GetPropertySafe(_T("NewVerDlPage"));
	nPageWidth = pNode->GetPropertyInt(_T("DlPageWidth"));
	nPageHeight = pNode->GetPropertyInt(_T("DlPageHeight"));

	szTemp = pNode->GetProperty(_T("upgradedate"));
	if (szTemp)
		strNewVerDate = szTemp;

	szTemp = pNode->GetProperty(_T("TrDaysEx"));
	if (szTemp)
		nTrialDays = _ttoi(szTemp);

	szTemp = pNode->GetProperty(_T("UseAffiliate"));
	if (szTemp)
		bUseAffiliate = Tobool(_ttoi(szTemp));
//
//	if (nTrialDays <= 15 || nTrialDays >= 150)
//		nTrialDays = 150;

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
	tstring			m_strDlLink;
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
			if (m_strDlLink.size())
				OpenUrl(this, m_strDlLink.c_str());
			else
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
	if (pTask->m_dwDownOkID == DOWNOKID_CHECK_VERSION)
		OnVersionInfoDownOK(pTask, false);
	else if (pTask->m_dwDownOkID == DOWNOKID_CHECK_VERSION_MANUAL)
		OnVersionInfoDownOK(pTask, true);
}

//
// mlver.xml 文件的格式参考前面的定义
void CVersionUpdate::OnVersionInfoDownOK(CDownloadTask *pTask, bool bManualCheck)
{
	assert(pTask);

	TCHAR		szCheckVerTime[256];
	CSimpleXML	xml;
	TCHAR		szTemp[MAX_PATH];
	CVersion	verCur(MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
	time_t		lTime;
	tm			*curtime;

	time(&lTime);

	curtime = localtime(&lTime);

	//
	// 将检查更新时间写入配置文件中
	_stprintf_s(szCheckVerTime, CountOf(szCheckVerTime), _T("%d-%d-%d"), curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
	g_Profile.EncryptWriteProfileString(_T("CheckVerTime"), szCheckVerTime);

	// 分析mlver.xml
	if (!xml.ParseFile(pTask->m_strSaveFile.c_str()))
	{
		ERRLOG1(_T("parse xml FILE FAILED: %s"), pTask->m_strSaveFile.c_str());
		return;
	}

	CVersionInfo		verInfo;
	if (verInfo.FromXML(xml.m_pRoot) != ERR_OK)
	{
		ERRLOG1(_T("xml file data error: %s"), pTask->m_strSaveFile.c_str());
		return;
	}

	g_Profile.WriteProfileInt(_T("UseAffiliate"), verInfo.bUseAffiliate);

	// RegWriteProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_CHECK_REG, _T("TrDaysEx"), verInfo.nTrialDays);
	// g_Profile.WriteProfileInt(_T("TrDaysEx"), verInfo.nTrialDays);

	verInfo.verObs.To(szTemp);
	g_Profile.WriteProfileString(_T("ObsoleteVersion"), szTemp);

	if (verCur < verInfo.verNew)
	{
#ifdef _WIN32_DESKTOP
		if (!g_bEngEdition && g_LicenseType != LT_REGISTERED)
		{
 			COnlineMsgWnd		*pWnd = NULL;
 			{
 				pWnd = new COnlineMsgWnd;
 				pWnd->Create(CStrPrintf(_T("%s - %s"), TOLOCAL(SZ_APP_NAME), TOLOCAL(_T("New version notice"))).c_str(),
					verInfo.nPageWidth, verInfo.nPageHeight, verInfo.strNewVerDlPage.c_str());
 				pWnd->ShowWindow(SW_SHOW);
 			}
		}
		else
#endif
		{
			CVersionUpdateDlg	dlg;

			dlg.m_szVersionDesc = verInfo.strFeature.c_str();
			dlg.m_strDlLink = verInfo.strNewVerDlPage;
			dlg.DoModal(g_mpApp.GetMainWnd());
		}
	}
	else if (bManualCheck && g_mpApp.GetMainWnd())
		g_mpApp.GetMainWnd()->MessageOut(TOLOCAL(_T("Your $Product$ is up to date.")));
}

void CVersionUpdate::CheckNewVersion(CLyricsDownloadMgr *pMgr)
{
	CDownloadTask *pOutTask = NULL;
	TCHAR		szCheckVerTime[256];
	int			wYear, wMonth, wDay;
	TCHAR		szVerFile[MAX_PATH];
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
	// g_Profile.WriteProfileString(_T("CheckVerTime"), szCheckVerTime);

	// 首先删除上次下载的文件
	_tcscpy_safe(szVerFile, CountOf(szVerFile), GetAppDataDir());
	_tcscat_safe(szVerFile, CountOf(szVerFile), _T("mlver.xml"));
	if (IsFileExist(szVerFile))
	{
		SetFileNoReadOnly(szVerFile);
		if (!DeleteFile(szVerFile))
		{
			ERRLOG1(_T("Delete File: %s, FAILED!"), szVerFile);
		}
	}

	pMgr->AddTask(GetStrName(SN_HTTP_ML_VER), NULL, szVerFile, DOWNOKID_CHECK_VERSION, NULL, false, true);
}

void CVersionUpdate::CheckNewVersionManual(CLyricsDownloadMgr *pMgr)
{
	CDownloadTask *pOutTask = NULL;
	TCHAR		szVerFile[MAX_PATH];

	// 首先删除上次下载的文件
	_tcscpy_safe(szVerFile, CountOf(szVerFile), GetAppDataDir());
	_tcscat_safe(szVerFile, CountOf(szVerFile), _T("mlver.xml"));
	if (IsFileExist(szVerFile))
	{
		SetFileNoReadOnly(szVerFile);
		if (!DeleteFile(szVerFile))
		{
			ERRLOG1(_T("Delete File: %s, FAILED!"), szVerFile);
		}
	}

	pMgr->AddTask(GetStrName(SN_HTTP_ML_VER), NULL, szVerFile, DOWNOKID_CHECK_VERSION_MANUAL, NULL, false, true);
}
