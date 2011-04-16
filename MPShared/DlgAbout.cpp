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

		TCHAR			szName[256];
		TCHAR			szRegCode[256];

		g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, CountOf(szName));
		g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));

		SetUIObjectText(_T("CID_E_NAME"), szName, false);
		SetUIObjectText(_T("CID_E_CODE"), szRegCode, false);

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

		if (g_LicenseType == LT_REGISTERED)
		{
			TCHAR	szRegCodeOrg[256];
			g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCodeOrg, CountOf(szRegCodeOrg));
			if (_tcscmp(strCode.c_str(), szRegCodeOrg) == 0)
				return true;
		}

		tstring		strMsg;

		if (MLVerifyRegCode(strName.c_str(), strCode.c_str()))
		{
			g_Profile.EncryptWriteProfileString(SZ_REGNAME, strName.c_str());
			g_Profile.EncryptWriteProfileString(SZ_REGCODE, strCode.c_str());

			strMsg = TOLOCAL(_T("Your MiniLyrics has been activated now."));
			strMsg += _T("\n");
			strMsg += TOLOCAL(_T("Thank you for supporting the development of MiniLyrics."));

			m_pSkin->MessageOut(strMsg.c_str());

			g_LicenseType = LT_REGISTERED;
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
	}

	~CPageAbout()
	{
		CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_DOWNLOAD_END, this);
	}

	void UpdateLicenseInfo()
	{
		TCHAR			szName[256];
		bool			bHasLicense = false;

		//
		// ×¢²áÐÅÏ¢
		if (g_LicenseType == LT_REGISTERED)
		{
			g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, sizeof(szName));
			SetUIObjectProperty(_T("CID_LICENSE_TO"), SZ_PN_TEXT, szName);
			bHasLicense = true;
		}

		SetUIObjectVisible(_T("CID_LICENSE"), bHasLicense, false);
		SetUIObjectVisible(_T("CID_LICENSE_TO"), bHasLicense, false);
		SetUIObjectVisible(_T("CID_UNREGISTERED"), !bHasLicense, false);

		Invalidate();
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

		m_pSkin->SetCaptionText(TOLOCAL(_T("About $Product$")));
		UpdateLicenseInfo();
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

		if (g_LicenseType == LT_REGISTERED)
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
	if (!bForceShow 
		&& (g_LicenseType == LT_REGISTERED || !g_bEngEdition))
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
