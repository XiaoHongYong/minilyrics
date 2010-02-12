#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloadMgr.h"
#include "VersionUpdate.h"
#include "DlgAbout.h"

class CPageAbout : public CSkinPageHandler, public IEventHandler
{
public:
	CPageAbout() : CSkinPageHandler(_T("CID_C_ABOUT"), 
		_T("CID_AM_SHOW_ABOUT_F_LEFT"), _T("CID_AM_SHOW_ABOUT_F_RIGHT"),
		_T("CID_AM_HIDE_ABOUT_2_LEFT"), NULL)
	{
		g_eventDispather.RegisterHanlder(ET_DOWNLOAD_END, this);
	}

	~CPageAbout()
	{
		g_eventDispather.UnregisterHanlder(ET_DOWNLOAD_END, this);
	}

	void UpdateLicenseInfo()
	{
		TCHAR			szName[256];
		bool			bHasLicense = false;

		//
		// 注册信息
		if (g_LicenseType == LT_REGISTERED)
		{
			g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, sizeof(szName));
			m_pSkinWnd->SetUIObjectProperty(_T("CID_LICENSE_TO"), SZ_PN_TEXT, szName);
			bHasLicense = true;
		}

		m_pSkinWnd->SetUIObjectVisible(_T("CID_LICENSE"), bHasLicense, false);
		m_pSkinWnd->SetUIObjectVisible(_T("CID_LICENSE_TO"), bHasLicense, false);
		m_pSkinWnd->SetUIObjectVisible(_T("CID_UNREGISTERED"), !bHasLicense, false);

		m_pSkinWnd->InvalidateRect();
	}

	void OnCreate()
	{
		m_pSkinWnd->SetUIObjectProperty(_T("CID_VERSION"), SZ_PN_TEXT, SZ_ML_VERSION);

		m_pSkinWnd->SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_LINK, GetStrName(SN_HTTP_DOMAIN));

		m_pSkinWnd->SetUIObjectProperty(_T("CID_EMAIL"), SZ_PN_LINK, GetStrName(SN_EMAIL));

		m_pLocalizeTool->RemoveItem(_T("CID_VERSION"));
		m_pLocalizeTool->RemoveItem(_T("CID_COPYRIGHT"));
		m_pLocalizeTool->RemoveItem(_T("CID_EMAIL"));

		CVersionUpdate	verUpdate;
		verUpdate.CheckNewVersion(&g_LyricsDownloader, false);
	}

	void OnShow()
	{
		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("About $Product$")));
		UpdateLicenseInfo();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_BUYNOW")))
		{
			OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_REGISTER));
			return true;
		}
		else if (nId == GetIDByName(_T("CID_ENTER_CODE")))
		{
			m_pParent->SwitchToPage(_T("CID_CODE"));
		}
		return false;
	}

	void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_DOWNLOAD_END)
		{
			CEventDownloadEnd		*pDlEvt = (CEventDownloadEnd *)pEvent;
			if (pDlEvt->pTask->m_dwDownOkID == DOWNOKID_CHECK_VERSION_MANUAL)
			{
				int		nRet = ERR_OK;

				if (pDlEvt->pTask->m_errResult == ERR_OK)
				{
					CVersionUpdate	verCheck;
					CVersionInfo	versionInfo;

					// Get new version info
					nRet = verCheck.GetUpdateInfo(pDlEvt->pTask, versionInfo);
					if (nRet == ERR_OK)
					{
						CVersion		verCur(MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
						if (verCur < versionInfo.verNew)
						{
							// New version
							tstring		strNewVersion = TOLOCAL(_T("New version of $Product$ is available"));
							strNewVersion += _T(" ");
							strNewVersion += versionInfo.verNew.To();

							SetUIObjectProperty(_T("CID_NEW_VERSION"), SZ_PN_TEXT, strNewVersion.c_str());
							SetUIObjectVisible(_T("CID_CHECK_VERSION"), false, false);
							SetUIObjectVisible(_T("CID_NEW_VERSION"), true, false);
							InvalidateRect();
						}
						else
						{
							SetUIObjectProperty(_T("CID_CHECK_VERSION"), SZ_PN_TEXT, TOLOCAL(_T("Your $Product$ is up to date.")));
							InvalidateUIObject(_T("CID_CHECK_VERSION"));
						}
						return;
					}
				}
				else
					nRet = pDlEvt->pTask->m_errResult;

				SetUIObjectProperty(_T("CID_CHECK_VERSION"), SZ_PN_TEXT, ERROR2STR_LOCAL(nRet));
				InvalidateUIObject(_T("CID_CHECK_VERSION"));
			}
		}
	}

};


class CPageEnterCode : public CSkinPageHandler
{
public:
	CPageEnterCode(bool bEndToClose) : CSkinPageHandler(_T("CID_CODE"),
		NULL, _T("CID_AM_SHOW_CODE_F_RIGHT"), NULL, _T("CID_AM_HIDE_CODE_2_RIGHT"))
	{
		m_bEndToClose = bEndToClose;
	}

	~CPageEnterCode()
	{
	}

	void OnShow()
	{
		TCHAR			szName[256];
		TCHAR			szRegCode[256];

		g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, CountOf(szName));
		if (g_LicenseType == LT_REGISTERED)
			_tcscpy_safe(szRegCode, CountOf(szRegCode), TOLOCAL(_T("Registered")));
		else
			g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));

		m_pSkinWnd->SetUIObjectText(_T("CID_E_NAME"), szName);
		m_pSkinWnd->SetUIObjectText(_T("CID_E_CODE"), szRegCode);

		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Registration")));
	}

	bool OnCheckRegCode()
	{
		tstring	strName = GetUIObjectText(_T("CID_E_NAME"));
		tstring	strCode = GetUIObjectText(_T("CID_E_CODE"));

		TrimStr(strName);
		TrimStr(strCode);

		if (strName.empty() || strCode.empty())
		{
			m_pSkinWnd->SetFocusUIObject(_T("CID_E_NAME"));
			return false;
		}

		if (g_LicenseType == LT_REGISTERED && _tcscmp(strCode.c_str(), TOLOCAL(_T("Registered"))) == 0)
			return true;

		tstring		strMsg;

		if (MLVerifyRegCode(strName.c_str(), strCode.c_str()))
		{
			g_Profile.EncryptWriteProfileString(SZ_REGNAME, strName.c_str());
			g_Profile.EncryptWriteProfileString(SZ_REGCODE, strCode.c_str());

			strMsg = TOLOCAL(_T("You have registered successfully!"));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Thanks for your registering!"));

			m_pSkinWnd->MessageOut(strMsg.c_str());

			// 设置全局注册信息:)
			g_LicenseType = LT_REGISTERED;
			return true;
		}
		else
		{
			strMsg = TOLOCAL(_T("The registration name and code you entered are invalid, please double check and try again."));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Please copy and paste the code from the original source if you continue to receive this error."));
			m_pSkinWnd->MessageOut(strMsg.c_str());
			return false;
		}
	}

	void OnOK()
	{
		if (OnCheckRegCode())
		{
			if (m_bEndToClose)
				CSkinPageHandler::OnCancel();
			else
				m_pParent->SwitchToLastPage();
		}
	}

	void OnCancel()
	{
		if (m_bEndToClose)
			CSkinPageHandler::OnCancel();
		else
			m_pParent->SwitchToLastPage();
	}

protected:
	bool		m_bEndToClose;

};

//////////////////////////////////////////////////////////////////////////

CDlgAbout::CDlgAbout() : CMPSkinDialog(_T("AboutBox.xml"))
{
	m_pageHandler.AddPage(new CPageAbout());
	m_pageHandler.AddPage(new CPageEnterCode(false));

	m_pageHandler.SetDefaultPage(_T("CID_C_ABOUT"));
}

CDlgAbout::~CDlgAbout()
{
}

//////////////////////////////////////////////////////////////////////////

class CPageNotice : public CSkinPageHandler
{
public:
	CPageNotice() : CSkinPageHandler(_T("CID_C_NOTICE"), 
		_T("CID_AM_SHOW_NOTICE_F_LEFT"), _T("CID_AM_SHOW_NOTICE_F_RIGHT"),
		_T("CID_AM_HIDE_NOTICE_2_LEFT"), NULL)
	{
	}

	~CPageNotice()
	{
	}

	void OnShow()
	{
		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("$Product$ (Evaluation Version)")));
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_BUYNOW")))
		{
			OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_REGISTER));
			m_pSkinWnd->Destroy();
			return true;
		}
		else if (nId == GetIDByName(_T("CID_ENTER_CODE")))
		{
			m_pParent->SwitchToPage(_T("CID_CODE"));
		}
		else if (nId == GetIDByName(_T("CID_NOTICE_OK")))
		{
			m_pSkinWnd->Destroy();
		}
		return false;
	}

};


class CDlgRegistration : public CMPSkinDialog
{
public:
	CDlgRegistration() : CMPSkinDialog(_T("Registration.xml"))
	{
		m_pageHandler.AddPage(new CPageNotice());
		m_pageHandler.AddPage(new CPageEnterCode(true));

		m_pageHandler.SetDefaultPage(_T("CID_C_NOTICE"));
	}

};

void ShowRegisterNotice(CBaseWnd *pWnd, BOOL bForceShow)
{
	if (!bForceShow 
		&& (g_LicenseType == LT_REGISTERED || !g_bEngEdition))
		return;

#ifdef _MPLAYER
	return;
#endif

	//
	//	if (g_LicenseType != LT_EXPIRED && g_LicenseType != LT_CRACK)
	//	{
	//		if (!bForceShow && !g_Profile.GetProfileInt(_T("ShowRegNotice"), TRUE))
	//			return;
	//	}

	CMPSkinDialog::ShowDialog(new CDlgRegistration(), pWnd);
}
