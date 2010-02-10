#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloadMgr.h"
#include "VersionUpdate.h"
#include "DlgAbout.h"
#include "RegistrationDlg.h"

CDlgAbout::CDlgAbout(bool bEnterCode) : CMPSkinDialog(_T("AboutBox.xml"))
{
	m_bEnterCode = bEnterCode;
}

CDlgAbout::~CDlgAbout()
{
}

void CDlgAbout::UpdateLicenseInfo()
{
	TCHAR			szName[256];
	bool			bHasLicense = false;

	//
	// 注册信息
	if (g_LicenseType == LT_REGISTERED)
	{
		g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, sizeof(szName));
		SetUIObjectProperty(_T("CID_LICENSE_TO"), SZ_PN_TEXT, szName);
		bHasLicense = true;
	}

	SetUIObjectVisible(_T("CID_LICENSE"), bHasLicense, false);
	SetUIObjectVisible(_T("CID_LICENSE_TO"), bHasLicense, false);
	SetUIObjectVisible(_T("CID_UNREGISTERED"), !bHasLicense, false);

	InvalidateRect();
}

void CDlgAbout::OnSkinLoaded()
{
	CMPSkinDialog::OnSkinLoaded();

	if (m_bEnterCode)
	{
		StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_SHOW_CODE")));
		return;
	}

	SetUIObjectProperty(_T("CID_VERSION"), SZ_PN_TEXT, SZ_ML_VERSION);

	// SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_TEXT, SZ_ML_VERSION);
	SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_LINK, GetStrName(SN_HTTP_DOMAIN));

	SetUIObjectProperty(_T("CID_EMAIL"), SZ_PN_LINK, GetStrName(SN_EMAIL));

	UpdateLicenseInfo();

	g_eventDispather.RegisterHanlder(ET_DOWNLOAD_END, this);

	StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_SHOW_ABOUT")));

	CVersionUpdate	verUpdate;
	verUpdate.CheckNewVersion(&g_LyricsDownloader, false);
}

void CDlgAbout::CloseSkin()
{
	g_eventDispather.UnregisterHanlder(ET_DOWNLOAD_END, this);

	CMPSkinDialog::CloseSkin();
}

bool CDlgAbout::OnCustomCommand(int nId)
{
	if (CMPSkinDialog::OnCustomCommand(nId))
		return true;

	if (nId == m_pSkinFactory->GetIDByName(_T("CID_BUYNOW")))
	{
		OpenUrl(this, GetStrName(SN_HTTP_REGISTER));
	}
	else if (nId == m_pSkinFactory->GetIDByName(_T("CID_ENTER_CODE")))
	{
		StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_HIDE_ABOUT")));

		StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_SHOW_CODE")));
		UpdateRegCode();
		SetUIObjectVisible(_T("CID_CODE"), true, false);

		SetCaptionText(TOLOCAL(_T("Registration")));
	}

	return true;
}

void CDlgAbout::OnOK()
{
	if (OnCheckRegCode())
	{
		if (m_bEnterCode)
		{
			CMPSkinDialog::OnOK();
			return;
		}

		UpdateLicenseInfo();

		HideCodeShowAbout();
	}
}

void CDlgAbout::OnCancel()
{
	if (m_bEnterCode)
	{
		CMPSkinDialog::OnCancel();
		return;
	}

	HideCodeShowAbout();
}

bool CDlgAbout::OnCheckRegCode()
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

		MessageOut(strMsg.c_str());

		// 设置全局注册信息:)
		g_LicenseType = LT_REGISTERED;
		return true;
	}
	else
	{
		strMsg = TOLOCAL(_T("The registration name and code you entered are invalid, please double check and try again."));
		strMsg += _T("\n");
		strMsg += TOLOCAL(_T("Please copy and paste the code from the original source if you continue to receive this error."));
		MessageOut(strMsg.c_str());
		return false;
	}
}

void CDlgAbout::UpdateRegCode()
{
	TCHAR			szName[256];
	TCHAR			szRegCode[256];

	g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, CountOf(szName));
	if (g_LicenseType == LT_REGISTERED)
		_tcscpy_safe(szRegCode, CountOf(szRegCode), TOLOCAL(_T("Registered")));
	else
		g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));

	SetUIObjectText(_T("CID_E_NAME"), szName);
	SetUIObjectText(_T("CID_E_CODE"), szRegCode);
}

void CDlgAbout::OnEvent(const IEvent *pEvent)
{
	CMPSkinDialog::OnEvent(pEvent);

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

void CDlgAbout::OnAnimationDone(int nID)
{
	if (nID == m_pSkinFactory->GetIDByName(_T("CID_AM_HIDE_ABOUT")))
	{
		SetUIObjectVisible(_T("CID_ABOUT"), false, false);

		InvalidateRect();
	}
	else if (nID == m_pSkinFactory->GetIDByName(_T("CID_AM_HIDE_CODE")))
	{
		SetUIObjectVisible(_T("CID_CODE"), false, false);

		InvalidateRect();
	}
}

void CDlgAbout::HideCodeShowAbout()
{
	StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_HIDE_CODE")));

	StartAnimation(m_pSkinFactory->GetIDByName(_T("CID_AM_SHOW_ABOUT_LEFT")));
	SetUIObjectVisible(_T("CID_ABOUT"), true, false);

	SetCaptionText(TOLOCAL(_T("About $Product$")));
}
