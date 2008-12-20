#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgAbout.h"
#include "RegistrationDlg.h"

CDlgAbout::CDlgAbout() : CBaseDialog(IDD_ABOUTBOX)
{
}

CDlgAbout::~CDlgAbout()
{
}

void CDlgAbout::OnLanguageChanged()
{
	SetDlgItemText(IDC_VERSION, SZ_ML_VERSION);

	CBaseWnd::OnLanguageChanged();
}

BOOL CDlgAbout::OnInitDialog()
{
	TCHAR			szName[256];
	TCHAR			szRegCode[256];

	SetDlgItemText(IDC_TECHSUPPORT, GetStrName(SN_SUPPORT_MAIL));

	CBaseDialog::OnInitDialog();

	m_hlForum.Attach(this, IDC_FORUM, GetStrName(SN_HTTP_BBS));
	m_hlMail.Attach(this, IDC_TECHSUPPORT, GetStrName(SN_EMAIL));
	m_hlWeb.Attach(this, IDC_WEB_HOME, GetStrName(SN_HTTP_DOMAIN));

	//
	// 版本信息
	m_LangDlgHelper.RemoveDlgItem(IDC_VERSION);
	m_LangDlgHelper.RemoveDlgItem(IDC_NAME);
	m_LangDlgHelper.RemoveDlgItem(IDC_REGISTRATION_CODE);

#ifdef _DEBUG
	ShowDlgItem(IDC_DEBUG_VER, SW_SHOW);
#endif

	SetDlgItemText(IDC_VERSION, SZ_ML_VERSION);

	//
	// 注册信息
	if (g_LicenseType == LT_REGISTERED)
	{
		g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, sizeof(szName));
		_tcscpy(szRegCode, TOLOCAL(_T("Registered")));
		ShowDlgItem(IDC_UN_REG, SW_HIDE);
	}
	else
	{
		_tcscpy(szName, TOLOCAL(_T("Unregistered")));
		_tcscpy(szRegCode, TOLOCAL(_T("Unregistered")));
		ShowDlgItem(IDC_UN_REG, SW_SHOW);
		ShowDlgItem(IDC_S_LICENSE_TO, SW_HIDE);
		ShowDlgItem(IDC_S_REGCODE, SW_HIDE);
		ShowDlgItem(IDC_NAME, SW_HIDE);
		ShowDlgItem(IDC_REGISTRATION_CODE, SW_HIDE);
	}

	SetDlgItemText(IDC_NAME, szName);
	SetDlgItemText(IDC_REGISTRATION_CODE, szRegCode);

	CeSHInitFullScrDlg(this);

	m_cmdBar.Create(this, -1);

	return TRUE;
}

void CDlgAbout::OnDestroy()
{
	m_LangDlgHelper.Destroy();
	m_cmdBar.Destroy();

	CBaseDialog::OnDestroy();
}

void CDlgAbout::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_PURCHASEINFO:
		OpenUrl(this, GetStrName(SN_HTTP_REGISTER));
		break;
	case IDC_ENTERCODE:
		{
			TCHAR			szName[256];
			TCHAR			szRegCode[256];
			CRegistrationDlg	dlg;

			if (IDOK == dlg.DoModal(this))
			{
				g_Profile.EncryptGetProfileString(SZ_REGNAME, _T(""), szName, sizeof(szName));
				_tcscpy(szRegCode, TOLOCAL(_T("Registered")));

				SetDlgItemText(IDC_NAME, szName);
				SetDlgItemText(IDC_REGISTRATION_CODE, szRegCode);
				ShowDlgItem(IDC_UN_REG, SW_HIDE);
				ShowDlgItem(IDC_S_LICENSE_TO, SW_SHOW);
				ShowDlgItem(IDC_S_REGCODE, SW_SHOW);
				ShowDlgItem(IDC_NAME, SW_SHOW);
				ShowDlgItem(IDC_REGISTRATION_CODE, SW_SHOW);
			}
		}
		break;
	}
}