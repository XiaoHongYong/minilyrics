// ErrorResolveDlg.cpp: implementation of the CErrorResolveDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "ErrorResolveDlg.h"
#include "Helper.h"
#include "PreferenceDlg.h"

void ShowErrorDlg(CBaseWnd *pWnd, int nError)
{
	tstring		strError;
	tstring		strResolve;
	TCHAR		szErrorId[256];

	_stprintf_s(szErrorId, CountOf(szErrorId), _T("%s %d,  "), TOLOCAL(_T("Error Code:")), nError);
	strError = szErrorId;
	strError += ERROR2STR_LOCAL(nError);
	switch (nError)
	{
#ifdef _WIN32
#ifdef _WIN32_WCE
	case WSAECONNREFUSED:
		strResolve  = TOLOCAL(_T("1. First make sure that you can visit our website by your web browser."));
		strResolve += _T("\r\n");
		strResolve += TOLOCAL(_T("2. Check whether you access internet via the network proxy, if so, setup the proxy server address and port in the preference window."));
		break;
#else
	case WSAEACCES:	// 10013
		strResolve = TOLOCAL(_T("Maybe your internet firewall blocked $Product$ to access internet, please enable your media player(example: winamp, windows media player ...) to access internet."));
		break;
	case WSAECONNREFUSED:
		strResolve  = TOLOCAL(_T("1. First make sure that you can visit our website by your web browser."));
		strResolve += _T("\r\n");
		strResolve += TOLOCAL(_T("2. Check whether your internet firewall blocks $Product$ to access internet, please enable your media player(example: winamp, windows media player ...) to access internet."));
		strResolve += _T("\r\n");
		strResolve += TOLOCAL(_T("3. Check whether you access internet via the network proxy, if so, setup the proxy server address and port in the preference window."));
		break;
#endif
	case WSAETIMEDOUT:
		strResolve += TOLOCAL(_T("The server may be too busy, please try again later."));
		break;
#endif
	case ERR_HTTP_403:
	case ERR_HTTP_401:
		strResolve = TOLOCAL(_T("If you access internet via the the network proxy, please setup the proxy server address and port in the preference window."));
		break;
	default:
		pWnd->MessageOut(strError.c_str());
		return;
	}

	strError += _T("\r\n\r\n\r\n");
	strError += strResolve;
	pWnd->MessageOut(strError.c_str());
/*	CErrorResolveDlg		dlg;

	dlg.SetInfo(strError.c_str(), strResolve.c_str());
	dlg.DoModal(pWnd);*/
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
class CErrorResolveDlg : public CBaseDialog  
{
public:
	CErrorResolveDlg();
	virtual ~CErrorResolveDlg();

	BOOL OnInitDialog();

	void SetInfo(LPCTSTR szError, LPCTSTR szResolve)
	{
		m_strError = szError;
		m_strResolve = szResolve;
	}

	void OnCommand(UINT uID, UINT nNotifyCode);

protected:
	tstring			m_strError;
	tstring			m_strResolve;
	CHyperLinkCtrl	m_hlEmail;

};*/
/*
CErrorResolveDlg::CErrorResolveDlg(): CBaseDialog(IDD_ERROR_RESOLVE)
{

}

CErrorResolveDlg::~CErrorResolveDlg()
{

}

BOOL CErrorResolveDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	tstring		str;

	SetDlgItemText(IDC_ERROR, m_strError.c_str());
	SetDlgItemText(IDC_RESOLVE, m_strResolve.c_str());

	str = GetStrName(SN_EMAIL);
	str += _T("?subject=Report bug");
	m_hlEmail.Attach(this, IDC_REPORT, str.c_str());

	return TRUE;
}

void CErrorResolveDlg::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	if (IDC_SETUP_INET == uID)
	{
		CBaseWnd		*pParent;

		pParent = GetParent();

		OnOK();

		CSkinDialog::ShowDialog(new CPreferenceDlg(false, PAGE_INET), this);
	}
	else if (IDC_COPYTOCLIPBOARD == uID)
	{
		SetClipboardText(this, m_strError.c_str());
	}
	else if (IDC_ML_HELP == uID)
	{
		OpenUrl(this, GetStrName(SN_HTTP_HELP));
	}
}
*/