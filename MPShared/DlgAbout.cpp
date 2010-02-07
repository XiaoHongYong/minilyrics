#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloadMgr.h"
#include "VersionUpdate.h"
#include "DlgAbout.h"
#include "RegistrationDlg.h"

CDlgAbout::CDlgAbout() : CMPSkinDialog(_T("AboutBox.xml"))
{
}

CDlgAbout::~CDlgAbout()
{
}

void CDlgAbout::UpdateLicenseInfo()
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

	InvalidateRect();
}

void CDlgAbout::OnSkinLoaded()
{
	CMPSkinDialog::OnSkinLoaded();

	SetUIObjectProperty(_T("CID_VERSION"), SZ_PN_TEXT, SZ_ML_VERSION);

	// SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_TEXT, SZ_ML_VERSION);
	SetUIObjectProperty(_T("CID_COPYRIGHT"), SZ_PN_LINK, GetStrName(SN_HTTP_DOMAIN));

	SetUIObjectProperty(_T("CID_EMAIL"), SZ_PN_LINK, GetStrName(SN_EMAIL));

	UpdateLicenseInfo();

	g_eventDispather.RegisterHanlder(ET_DOWNLOAD_END, this);

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
		CRegistrationDlg	dlg;

		dlg.DoModal(this);

		UpdateLicenseInfo();
	}

	return true;
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
