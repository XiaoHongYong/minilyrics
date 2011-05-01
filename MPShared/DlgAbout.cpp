#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "DlgAbout.h"


class CPageEnterCode : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		CLicenseManager::LicenseInfo	licenseInfo;

		g_licenseManager.GetLicenseInfo(licenseInfo);

		SetUIObjectText(_T("CID_E_NAME"), licenseInfo.strName.c_str(), false);
		SetUIObjectText(_T("CID_E_CODE"), licenseInfo.strCode.c_str(), false);

		m_pSkin->SetCaptionText(TOLOCAL(_T("Activate MiniLyrics")));
	}

	bool OnCheckRegCode()
	{
		tstring	strName = GetUIObjectText(_T("CID_E_NAME"));
		tstring	strCode = GetUIObjectText(_T("CID_E_CODE"));

		TrimStr(strName);
		TrimStr(strCode);

		if (strName.empty() || strCode.empty())
		{
			SetFocusUIObject(_T("CID_E_NAME"));
			return false;
		}

		CLicenseManager::LicenseInfo	licenseInfo;

		g_licenseManager.GetLicenseInfo(licenseInfo);
		if (licenseInfo.licenseStatus == LS_ACTIVE && licenseInfo.bOnlineVerified)
		{
			if (_tcscmp(strCode.c_str(), licenseInfo.strCode.c_str()) == 0)
				return true;
		}

		tstring		strMsg;

		if (g_licenseManager.VerifyCode(strName.c_str(), strCode.c_str(), TRUE))
		{
			// Activate license code online.
			CMLClientSession	session;
			MLMsgRetActivateLicense retActivation;
			int nRet = session.ActivateLicense(strMsg, retActivation);

			strMsg = TOLOCAL(_T("Your MiniLyrics has been activated now."));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Thank you for supporting the development of MiniLyrics."));

			m_pSkin->MessageOut(strMsg.c_str());
			return true;
		}
		else
		{
			strMsg = TOLOCAL(_T("The name and activation code you entered are invalid, please double check and try again."));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Please copy and paste the code from the original source if you continue to receive this error."));
			m_pSkin->MessageOut(strMsg.c_str());
			return false;
		}
	}

	bool OnOK()
	{
		if (OnCheckRegCode())
		{
			if (IsEndToClose())
				return true;
			else
				m_pContainer->SwitchToLastPage(RESULT_OK, true);
		}

		return false;
	}

	bool OnCancel()
	{
		if (IsEndToClose())
			return true;
		else
			m_pContainer->SwitchToLastPage(RESULT_CANCELD, true);
		return false;
	}

protected:
	bool IsEndToClose()
	{
		return GetExPoolBool(_T("EnterCode.Close"), false);
	}

};

UIOBJECT_CLASS_NAME_IMP(CPageEnterCode, _T("Container.EnterCode"))

//////////////////////////////////////////////////////////////////////////

class CPageAbout : public CSkinContainer, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageAbout()
	{
		CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_DOWNLOAD_END, this);
		m_strText = TOLOCAL_MARK(_T("About $Product$"));
	}

	~CPageAbout()
	{
		CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_DOWNLOAD_END, this);
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		tstring		strVer;
#ifdef _MPLAYER
		strVer = VersionNumToStr(g_version);
#else
		strVer = CStrPrintf(_T("%s for %s"), VersionNumToStr(g_version).c_str(), g_Player.GetPlayerName()).c_str();
#endif
		SetUIObjectProperty(_T("CID_VERSION"), SZ_PN_TEXT, strVer.c_str());

		SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_LINK, GetStrName(SN_HTTP_DOMAIN));

		SetUIObjectProperty(_T("CID_EMAIL"), SZ_PN_LINK, GetStrName(SN_EMAIL));

		CVersionUpdate	verUpdate;
		verUpdate.CheckNewVersion(false);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		CLicenseManager::LicenseInfo	licenseInfo;
		g_licenseManager.GetLicenseInfo(licenseInfo);

		if (licenseInfo.licenseStatus == LS_ACTIVE)
			SetUIObjectProperty(_T("CID_LICENSE_TO"), SZ_PN_TEXT, licenseInfo.strName.c_str());

		SetUIObjectVisible(_T("CID_LICENSE"), licenseInfo.licenseStatus == LS_ACTIVE, false);
		SetUIObjectVisible(_T("CID_LICENSE_TO"), licenseInfo.licenseStatus == LS_ACTIVE, false);
		SetUIObjectVisible(_T("CID_UNREGISTERED"), licenseInfo.licenseStatus != LS_ACTIVE, false);
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_BUYNOW")))
			OpenUrl(m_pSkin, GetStrName(SN_HTTP_REGISTER));
		else if (nId == GetIDByName(_T("CID_ENTER_CODE")))
			m_pContainer->SwitchToPage(CPageEnterCode::ClassName(), false, 0, true);
		else
			return false;

		return true;
	}

	void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_DOWNLOAD_END)
		{
			CEventDownloadEnd		*pDlEvt = (CEventDownloadEnd *)pEvent;
			if (pDlEvt->pTask->taskType == DTT_CHECK_VERSION_NOUI)
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
						if (g_version < versionInfo.verNew)
						{
							// New version
							tstring		strNewVersion = TOLOCAL(_T("New version of $Product$ is available"));
							strNewVersion += _T(" ");
							strNewVersion += VersionNumToStr(versionInfo.verNew);

							SetUIObjectProperty(_T("CID_NEW_VERSION"), SZ_PN_TEXT, strNewVersion.c_str());
							SetUIObjectProperty(_T("CID_NEW_VERSION"), SZ_PN_LINK, GetStrName(SN_HTTP_DOMAIN));
							SetUIObjectVisible(_T("CID_CHECK_VERSION"), false, false);
							SetUIObjectVisible(_T("CID_NEW_VERSION"), true, false);
							Invalidate();
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

UIOBJECT_CLASS_NAME_IMP(CPageAbout, _T("Container.About"));

//////////////////////////////////////////////////////////////////////////

class CPageNotice : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		if (g_licenseManager.IsLicenseValid())
			m_pSkin->SetCaptionText(TOLOCAL(_T("$Product$ (Registered Version)")));
		else
			m_pSkin->SetCaptionText(TOLOCAL(_T("$Product$ (Evaluation Version)")));
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_BUYNOW")))
		{
			OpenUrl(m_pSkin, GetStrName(SN_HTTP_REGISTER));
			m_pSkin->PostDestroy();
			return true;
		}
		else if (nId == GetIDByName(_T("CID_ENTER_CODE")))
		{
			m_pContainer->SwitchToPage(CPageEnterCode::ClassName(), false, 0, true);
		}
		else if (nId == GetIDByName(_T("CID_NOTICE_OK")))
		{
			m_pSkin->PostDestroy();
		}
		return false;
	}

};

UIOBJECT_CLASS_NAME_IMP(CPageNotice, _T("Container.RegNotice"))

//////////////////////////////////////////////////////////////////////////

void ShowAboutDialog(CBaseWnd *pWndParent)
{
	CSkinApp::GetInstance()->ShowDialog(pWndParent, _T("AboutBox.xml"));
}

void ShowRegisterNotice(CBaseWnd *pWndParent, BOOL bForceShow)
{
	if (!bForceShow && g_licenseManager.IsLicenseValid())
		return;

#ifdef _MPLAYER
	return;
#endif

	CSkinApp::GetInstance()->ShowDialog(pWndParent, _T("Registration.xml"));
}

//////////////////////////////////////////////////////////////////////////

void RegisterAboutPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageAbout);
	AddUIObjNewer2(pSkinFactory, CPageEnterCode);
	AddUIObjNewer2(pSkinFactory, CPageNotice);
}
