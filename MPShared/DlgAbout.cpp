#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "WaitingDlg.h"
#include "DlgAbout.h"

#define SZ_EX_POOL_MSG		_T("Activate.Msg")
#define SZ_EX_POOL_SUCCEED	_T("Activate.Succeed")

tstring LicenseStatusToStr(CLicenseManager::LicenseInfo &licenseInfo)
{
	tstring strMsg;
	if (licenseInfo.licenseStatus == LS_ACTIVE)
	{
		if (licenseInfo.bOnlineVerified)
		{
			strMsg = TOLOCAL_T("You MiniLyrics was activated successfully.");
			strMsg += _T("\n");
			strMsg += TOLOCAL_T("Thank you for supporting the development of MiniLyrics.");
		}
		else
		{
			strMsg = TOLOCAL_T("Your activation code is verified OK.");
			strMsg += _T("\n");
			strMsg += TOLOCAL_T("Please click 'Activate' button to activate your MiniLyrics.");
		}
	}
	else if (licenseInfo.licenseStatus == LS_TRIAL)
	{
		if (licenseInfo.strCode.size())
		{
			strMsg = TOLOCAL_T("Your activation code is invalid.");
			strMsg += _T("\n");
			strMsg += TOLOCAL_T("If you have purchased MiniLyrics, please contact us.");
		}
		else
		{
			strMsg = TOLOCAL_T("*** TRIAL VERSION ***");
			strMsg += _T("\n");
			strMsg += TOLOCAL_T("Please activate MiniLyrics.");
		}
	}
	else if (licenseInfo.licenseStatus == LS_EXPIRED)
	{
		strMsg = TOLOCAL_T("Your license has get expired.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Please renew your license.");
	}
	else if (licenseInfo.licenseStatus == LS_REVOKED)
	{
		strMsg = TOLOCAL_T("Your activation code has been revoked.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Maybe you have refund your order.");
	}
	else if (licenseInfo.licenseStatus == LS_TRANSFERED)
	{
		strMsg = TOLOCAL_T("Your activation code has been transferred to another devices.");
		strMsg += _T("\n");
		strMsg += CStrPrintf(TOLOCAL_T("Your code can be used on %d devices."), licenseInfo.nLicensedUser).c_str();
	}

	return strMsg;
}

class CActivateWorkObj : public CWorkObjBase
{
public:
	CActivateWorkObj()
	{
		m_bEnableCancel = true;
	}

	virtual DWORD DoTheJob()
	{
		m_strMsg = _T("");

		m_session.Init(GetAppNameLong().c_str());
		m_nResult = m_session.Connect();
		if (m_nResult == ERR_OK)
			m_nResult = m_session.ActivateLicense(m_strMsg, m_retMsg);

		return CWorkObjBase::DoTheJob();
	}

	virtual void CancelJob()
	{
		m_session.Close();

		CWorkObjBase::CancelJob();
	}

public:
	tstring					m_strMsg;
	int						m_nResult;
	CMLClientSession		m_session;
	MLMsgRetActivateLicense m_retMsg;

};

//////////////////////////////////////////////////////////////////////////

class CPageActivateWaitMessage : public CPageWaitMessage
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	virtual void OnWorkEnd();

};

UIOBJECT_CLASS_NAME_IMP(CPageActivateWaitMessage, _T("Container.ActivateWaitMsg"))

//////////////////////////////////////////////////////////////////////////

class CPageEnterCode : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageEnterCode()
	{
		m_pActivateWorkObj = NULL;
		m_strText = TOLOCAL_MARK(_T("MiniLyrics Activation"));
	}

	~CPageEnterCode()
	{
		if (m_pActivateWorkObj)
			delete m_pActivateWorkObj;
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		CLicenseManager::LicenseInfo	licenseInfo;

		g_licenseManager.GetLicenseInfo(licenseInfo);

		SetUIObjectText(_T("CID_E_NAME"), licenseInfo.strName.c_str(), false);
		SetUIObjectText(_T("CID_E_CODE"), licenseInfo.strCode.c_str(), false);
	}

	void OnCheckRegCode()
	{
		tstring	strName = GetUIObjectText(_T("CID_E_NAME"));
		tstring	strCode = GetUIObjectText(_T("CID_E_CODE"));

		TrimStr(strName);
		TrimStr(strCode);

		if (strName.empty() || strCode.empty())
		{
			SetFocusUIObject(_T("CID_E_NAME"));
			return;
		}

		/* CLicenseManager::LicenseInfo	licenseInfo;

		g_licenseManager.GetLicenseInfo(licenseInfo);
		if (licenseInfo.licenseStatus == LS_ACTIVE && licenseInfo.bOnlineVerified)
		{
			if (_tcscmp(strCode.c_str(), licenseInfo.strCode.c_str()) == 0)
				return;
		}*/

		tstring		strMsg;

		if (g_licenseManager.VerifyCode(strName.c_str(), strCode.c_str(), TRUE))
		{
			// Activate license code online.
			if (m_pActivateWorkObj)
				delete m_pActivateWorkObj;
			m_pActivateWorkObj = new CActivateWorkObj();

			CPageActivateWaitMessage *pPageWait = (CPageActivateWaitMessage *)m_pContainer->GetChildByClass(CPageActivateWaitMessage::ClassName());
			ASSERT(pPageWait);
			if (pPageWait)
			{
				pPageWait->StartWork(TOLOCAL(_T("Activate MiniLyrics")), 
					TOLOCAL(_T("The activation code is being verified, it may take a few seconds.")),
					m_pActivateWorkObj);
			}
		}
		else
		{
			strMsg = TOLOCAL(_T("The name and activation code you entered are invalid, please double check and try again."));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Please copy and paste the code from the original source if you continue to receive this error."));
			m_pSkin->MessageOut(strMsg.c_str());
		}
	}

	bool OnOK()
	{
		OnCheckRegCode();
		return false;
	}

protected:
	CActivateWorkObj		*m_pActivateWorkObj;

};

UIOBJECT_CLASS_NAME_IMP(CPageEnterCode, _T("Container.EnterCode"))

//////////////////////////////////////////////////////////////////////////

class CPageActivateResult : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageActivateResult()
	{
		m_strText = TOLOCAL_MARK(_T("MiniLyrics Activation"));
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		GET_ID_BY_NAME(CID_TRY_AGAIN);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		tstring strMsg = GetExPoolStr(SZ_EX_POOL_MSG);
		bool bSucceeded = GetExPoolBool(SZ_EX_POOL_SUCCEED, false);

		SetUIObjectVisible(_T("CID_C_ACTIVATION_OK"), bSucceeded, false);
		SetUIObjectVisible(_T("CID_C_ACTIVATION_FAILED"), !bSucceeded, false);
		if (!bSucceeded)
			SetUIObjectText(_T("CID_RESULT"), strMsg.c_str(), false);
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_TRY_AGAIN)
			m_pContainer->SwitchToPage(CPageEnterCode::ClassName(), false, 0, true);
		else
			return CSkinContainer::OnCustomCommand(nId);

		return true;
	}

protected:
	int					CID_TRY_AGAIN;

};

UIOBJECT_CLASS_NAME_IMP(CPageActivateResult, _T("Container.ActivateResult"))

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
		SetUIObjectText(_T("CID_COPYRIGHT"), CStrPrintf(_T("%s  2001-2011 Crintsoft MiniLyrics"), TOLOCAL_T("Copyright")).c_str());

		SetUIObjectProperty(_T("CID_FEEDBACK"), SZ_PN_LINK, GetStrName(SN_HTTP_FEEDBACK));

		CVersionUpdate	verUpdate;
		verUpdate.CheckNewVersion(false);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		CLicenseManager::LicenseInfo	licenseInfo;
		g_licenseManager.GetLicenseInfo(licenseInfo);


		tstring	strLicensedTo = TOLOCAL_T("Licensed to: ");
		strLicensedTo += licenseInfo.strName;

		tstring strLicense;
		if (licenseInfo.bOnlineVerified)
		{
			strLicense = TOLOCAL_T("License: ");
			if (licenseInfo.nLicensedUser > 1)
				strLicense += CStrPrintf(TOLOCAL_T("%d Users"), licenseInfo.nLicensedUser).c_str();
			else
				strLicense += TOLOCAL_T("Single User");
			if (licenseInfo.nDurationDays > 0)
			{
				strLicense += _T(", ");
				strLicense += CStrPrintf(TOLOCAL_T("%d Days"), licenseInfo.nDurationDays).c_str();

				if (licenseInfo.strStartDate.size())
				{
					strLicense += _T("\n"); strLicense += TOLOCAL_T("Start Date:"); strLicense += _T(" "); 
					strLicense += licenseInfo.strStartDate;
				}
				if (licenseInfo.strExpireDate.size())
				{
					strLicense += _T("\n"); strLicense += TOLOCAL_T("Expiration Date:"); strLicense += _T(" ");
					strLicense += licenseInfo.strExpireDate;
				}
			}
		}

		SetUIObjectText(_T("CID_LICENSE_TO"), strLicensedTo.c_str());
		SetUIObjectText(_T("CID_LICENSE"), strLicense.c_str());

		SetUIObjectVisible(_T("CID_LICENSE"), !strLicense.empty(), false);
		SetUIObjectVisible(_T("CID_LICENSE_TO"), !strLicensedTo.empty(), false);

		SetUIObjectText(_T("CID_ACT_NOTICE"), LicenseStatusToStr(licenseInfo).c_str());
		bool bActivateNotice = !licenseInfo.bOnlineVerified || licenseInfo.licenseStatus != LS_ACTIVE;
		SetUIObjectVisible(_T("CID_ACT_NOTICE"), bActivateNotice, false);
		SetUIObjectVisible(_T("CID_ENTER_CODE"), bActivateNotice, false);
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

void CPageActivateWaitMessage::OnWorkEnd()
{
	if (m_pWorkObj->IsJobCanceled())
	{
		m_pContainer->SwitchToLastPage(0, true);
	}
	else
	{
		tstring strMsg;
		bool bSucceed = false;
		CActivateWorkObj *pActivateWork = (CActivateWorkObj *)m_pWorkObj;
		if (pActivateWork->m_nResult == ERR_OK)
		{
			CLicenseManager::LicenseInfo licenseInfo;
			g_licenseManager.GetLicenseInfo(licenseInfo);
			ASSERT(licenseInfo.licenseStatus == pActivateWork->m_retMsg.licenseStatus);

			if (pActivateWork->m_retMsg.licenseStatus == LS_ACTIVE)
				bSucceed = true;

			strMsg = LicenseStatusToStr(licenseInfo);
		}
		else
			strMsg = ERROR2STR_LOCAL(pActivateWork->m_nResult);

		SetExPool(SZ_EX_POOL_SUCCEED, bSucceed);
		SetExPool(SZ_EX_POOL_MSG, strMsg.c_str());
		m_pContainer->SwitchToPage(CPageActivateResult::ClassName(), false, 0, true);
	}
}

void ShowAboutDialog(CBaseWnd *pWndParent)
{
	CSkinApp::GetInstance()->ShowDialog(pWndParent, _T("AboutBox.xml"));
}

void ShowActivateDialog(CBaseWnd *pWndParent)
{
	CSkinApp::GetInstance()->ShowDialog(pWndParent, _T("Registration.xml"));
}

//////////////////////////////////////////////////////////////////////////

void RegisterAboutPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageAbout);
	AddUIObjNewer2(pSkinFactory, CPageEnterCode);
	AddUIObjNewer2(pSkinFactory, CPageActivateResult);
	AddUIObjNewer2(pSkinFactory, CPageActivateWaitMessage);
}
