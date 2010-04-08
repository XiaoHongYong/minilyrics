#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageSystem.h"
#include "INetTestDlg.h"


//////////////////////////////////////////////////////////////////////////

class CPagePfInternet : public CPagePfBase
{
public:
	CPagePfInternet() : CPagePfBase(GetPageID(), PAGE_INET, _T("CMD_SYSTEM_INTERNET"))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_INTERNET"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();


		//
		// HTTP 代理设置
		//
		TCHAR	szProxyServer[256];
		int		nProxyPort;
		BOOL	bUseProxy;

		bUseProxy = CMLProfile::InetGetProxy(szProxyServer, nProxyPort);
		CheckButton(_T("CID_USE_PROXY"), Tobool(bUseProxy));

		// Proxy server address
		SetUIObjectText(_T("CID_HTTP_PROXY"), g_Profile.GetProfileString(_T("ProxyServer"), _T("")));
		SetUIObjectText(_T("CID_HTTP_PORT"), g_Profile.GetProfileString(_T("ProxyPort"), _T("")));

		// Proxy user name and password
		SetUIObjectText(_T("CID_USER"), g_Profile.GetProfileString(_T("ProxyUser"), _T("")));
		TCHAR	szPwd[256];
		g_Profile.EncryptGetProfileString(_T("ProxyPassEnc"), _T(""), szPwd, CountOf(szPwd));
		if (IsEmptyString(szPwd))
			g_Profile.GetProfileString(_T("ProxyPass"), _T(""), szPwd, CountOf(szPwd));
		SetUIObjectText(_T("CID_PASS"), szPwd);
	}

	void OnDestroy()
	{
		SaveProxyServerUserPwd();

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_LOAD_IE_PROXY")))
		{
			bool		bUseProxy;
			tstring		strSvr;
			int			nPort;
			if (LoadProxySvrFromIE(bUseProxy, strSvr, nPort))
			{
				int		nHttpProxyType;

				CheckButton(_T("CID_USE_PROXY"), Tobool(bUseProxy));
				if (bUseProxy)
					nHttpProxyType = HTTP_PROXY_OURS;
				else
					nHttpProxyType = HTTP_PROXY_NONE;

				g_Profile.WriteProfileInt(_T("ProxyType"), nHttpProxyType);

				SetUIObjectText(_T("CID_HTTP_PROXY"), strSvr.c_str());
				SetUIObjectText(_T("CID_HTTP_PORT"), CStrPrintf(_T("%d"), nPort).c_str());
			}
		}
		else if (nId == GetIDByName(_T("CID_USE_PROXY")))
		{
			int		nHttpProxyType;

			if (IsButtonChecked(nId))
				nHttpProxyType = HTTP_PROXY_OURS;
			else
				nHttpProxyType = HTTP_PROXY_NONE;
			g_Profile.WriteProfileInt(_T("ProxyType"), nHttpProxyType);
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

protected:
	void SaveProxyServerUserPwd()
	{
		g_Profile.WriteProfileString(_T("ProxyServer"), 
			GetUIObjectText(_T("CID_HTTP_PROXY")).c_str());

		g_Profile.WriteProfileString(_T("ProxyPort"), 
			GetUIObjectText(_T("CID_HTTP_PORT")).c_str());

		tstring strUser = GetUIObjectText(_T("CID_USER"));
		g_Profile.WriteProfileString(_T("ProxyUser"), strUser.c_str());

		tstring strPwd = GetUIObjectText(_T("CID_PASS"));
		g_Profile.EncryptWriteProfileString(_T("ProxyPassEnc"), strPwd.c_str());
		g_Profile.WriteProfileString(_T("ProxyPass"), _T(""));

		if (strUser.size())
		{
			stringex	strBase64, strPwdUser;
			TCHAR			szPwdUser[256];

			_stprintf_s(szPwdUser, CountOf(szPwdUser), _T("%s:%s"), strUser.c_str(), strPwd.c_str());
			ConvertStr(szPwdUser, -1, strPwdUser);
			encodeuu(strPwdUser, strPwdUser.size(), strBase64, true);
#ifdef _UNICODE
			wstringex	strW;
			ConvertStr(strBase64.c_str(), strBase64.size(), strW);
			g_Profile.WriteProfileString(_T("Base64ProxyUserPass"), strW.c_str());
#else
			g_Profile.WriteProfileString(_T("Base64ProxyUserPass"), strBase64.c_str());
#endif
		}
	}

};

//////////////////////////////////////////////////////////////////////////

#ifndef _MPLAYER

ID_TO_STRING		_id2strPlayers[] = { { 0, _T("Winamp2") }, { 0, _T("Winamp5") },
	{ 0, _T("Windows Media Player") }, { 0, _T("Foobar2000") }, 
	{ 0, _T("RealPlayer") }, { 0, _T("Quintessential Player") }, 
	{ 0, _T("iTunes") }, { 0, _T("Silverjuke") },
	{ 0, _T("MediaMonkey") }, { 0, _T("KMPlayer") }, 
	{ 0, _T("JetAudio") }, { 0, _T("Yahoo! Music Jukebox") }, 
	{ 0, _T("Bsplayer") }, { 0, _T("Media Jukebox") }, 
	{ 0, _T("XMPlay") }, { 0, _T("AIMP2") },
	{ 0, _T("VLC Media Player") },
	{ 0, NULL }
};

#endif // #ifndef _MPLAYER

class CPagePfStartup: public CPagePfBase
{
public:
	CPagePfStartup() : CPagePfBase(GetPageID(), PAGE_STARTUP, _T("CMD_SYSTEM_STARTUP"))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_STARTUP"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();

		AddOptBool(ET_NULL, SZ_SECT_UI, _T("CheckNewVer"), true, _T("CID_C_CHECK_NEW_VERSION"));

		InitCheckButtons();

#ifndef _MPLAYER
		CSkinContainer *pContainer = (CSkinContainer *)GetUIObjectById(GetPageID(), CSkinContainer::ClassName());
		ASSERT(pContainer);
		if (!pContainer)
			return;

		CUIObject *pObjFrame = GetUIObjectById(_T("CID_STARTUP_FRAME"));
		ASSERT(pObjFrame);
		if (!pObjFrame)
			return;

		CStrPrintf	strWidth(_T("(%s)/2-10"), pObjFrame->m_formWidth.GetFormula());
		CStrPrintf	strP1Left(_T("%s + 10"), pObjFrame->m_formLeft.GetFormula());
		CStrPrintf	strP2Left(_T("%s+(%s)/2+10"), pObjFrame->m_formLeft.GetFormula(),
			pObjFrame->m_formWidth.GetFormula());
		CFormula	formTop;

		formTop.SetFormula(pObjFrame->m_formTop.GetFormula());
		formTop.Increase(25);

		for (int i = 0; _id2strPlayers[i].szId != NULL; i++)
		{
			bool bLeft = (i % 2 == 0);

			if (_id2strPlayers[i].dwId == 0)
				_id2strPlayers[i].dwId = m_pSkinWnd->GetSkinFactory()->AllocCustomID();

			CSkinNStatusButton *pButton = (CSkinNStatusButton*)m_pSkinWnd->GetSkinFactory()->CreateDynamicCtrl(
				pContainer, _T("NormalCheckBox"), _id2strPlayers[i].dwId,
				bLeft ? strP1Left.c_str() : strP2Left.c_str(), formTop.GetFormula(), strWidth.c_str());
			if (pButton)
			{
				pButton->SetProperty(SZ_PN_TEXT, _id2strPlayers[i].szId);
				if (g_Profile.GetProfileInt(_id2strPlayers[i].szId, TRUE))
					pButton->SetStatus(true);
			}

			if (!bLeft)
				formTop.Increase(30);
		}
#endif // #ifndef _MPLAYER
	}

	bool OnCustomCommand(int nId)
	{
#ifndef _MPLAYER
		//
		// Should MiniLyrics get started with the checked player?
		//
		LPCTSTR		szPlayer;
		szPlayer = IDToString(_id2strPlayers, nId, _T(""));
		if (!IsEmptyString(szPlayer))
		{
			g_Profile.WriteProfileInt(szPlayer, IsButtonChecked(nId));
			return true;
		}
#endif // #ifndef _MPLAYER

		return CPagePfBase::OnCustomCommand(nId);
	}

	void OnDestroy()
	{
#ifndef _MPLAYER
		RegWriteProfileInt(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\UIPlugins\\{46B5EE7F-3B6B-4079-A756-5EFC10B1F50B}"), 
			_T("Running"), IsButtonChecked(IDC_WMP));
#endif // #ifndef _MPLAYER

		CPagePfBase::OnDestroy();
	}

protected:

};

//////////////////////////////////////////////////////////////////////////

CPagePfSystemRoot::CPagePfSystemRoot() : CPagePfBase(GetPageID(), PAGE_UNKNOWN, _T("CMD_ROOT_SYSTEM"))
{
	AddPage(new CPagePfInternet());
	AddPage(new CPagePfStartup());
}

LPCTSTR CPagePfSystemRoot::GetPageID()
{
	return _T("CID_C_SYSTEM_ROOT");
}

void CPagePfSystemRoot::OnCreate()
{
	CPagePfBase::OnCreate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_SYSTEM"));
}

/*
bool CPagePfSystemRoot::OnCustomCommand(int nId)
{
	if (nId == GetIDByName(_T("CMD_SYSTEM_INTERNET")))
		SwitchToPage(CPagePfInternet::GetPageID());
	else if (nId == GetIDByName(_T("CMD_SYSTEM_STARTUP")))
		SwitchToPage(CPagePfStartup::GetPageID());
	else
		return CPagePfBase::OnCustomCommand(nId);

	return true;
}*/


//////////////////////////////////////////////////////////////////////////

CPreferPageInternet::CPreferPageInternet() : CPreferencePageBase(IDD_PAGE_INTERNET, _T("Internet"))
{
	m_PageID = PAGE_INET;
}

CPreferPageInternet::~CPreferPageInternet()
{
}

BOOL CPreferPageInternet::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_LangDlgHelper.RemoveDlgItem(IDC_USER);
	m_LangDlgHelper.RemoveDlgItem(IDC_PASS);
	m_LangDlgHelper.RemoveDlgItem(IDC_HTTP_PROXY);

	//
	// HTTP 代理设置
	//
	int		nIdItem;
	TCHAR	szProxyServer[256];
	int		nProxyPort;
	BOOL	bUserProxy;

	bUserProxy = CMLProfile::InetGetProxy(szProxyServer, nProxyPort);
	if (bUserProxy)
		nIdItem = IDC_PROXY_OURS;
	else
		nIdItem = IDC_PROXY_NONE;
	CheckButton(nIdItem, true);

	// 代理服务器地址
	SetDlgItemText(IDC_HTTP_PROXY, g_Profile.GetProfileString(_T("ProxyServer"), _T("")));
	SetDlgItemText(IDC_E_PORT, g_Profile.GetProfileString(_T("ProxyPort"), _T("")));

	TCHAR	szTemp[256];

	// 代理服务器用户名和密码
	g_Profile.GetProfileString(_T("ProxyUser"), _T(""), szTemp, CountOf(szTemp));
	SetDlgItemText(IDC_USER, szTemp);

	g_Profile.EncryptGetProfileString(_T("ProxyPassEnc"), _T(""), szTemp, CountOf(szTemp));
	if (IsEmptyString(szTemp))
		g_Profile.GetProfileString(_T("ProxyPass"), _T(""), szTemp, CountOf(szTemp));
	SetDlgItemText(IDC_PASS, szTemp);

	return TRUE;
}

void CPreferPageInternet::SaveProxyServerUserPwd()
{
	TCHAR	szTemp[256];
	TCHAR	szUser[128];
	TCHAR	szPass[128];

	GetDlgItemText(IDC_HTTP_PROXY, szTemp, CountOf(szTemp));
	g_Profile.WriteProfileString(_T("ProxyServer"), szTemp);

	GetDlgItemText(IDC_E_PORT, szTemp, CountOf(szTemp));
	g_Profile.WriteProfileString(_T("ProxyPort"), szTemp);

	// 代理服务器用户名和密码
	GetDlgItemText(IDC_USER, szUser, CountOf(szUser));
	g_Profile.WriteProfileString(_T("ProxyUser"), szUser);

	GetDlgItemText(IDC_PASS, szPass, CountOf(szPass));
	g_Profile.EncryptWriteProfileString(_T("ProxyPassEnc"), szPass);
	g_Profile.WriteProfileString(_T("ProxyPass"), _T(""));

	if (!IsEmptyString(szUser))
	{
		stringex	strBase64, strPwdUser;
		TCHAR			szPwdUser[256];

		_stprintf_s(szPwdUser, CountOf(szPwdUser), _T("%s:%s"), szUser, szPass);
		ConvertStr(szPwdUser, -1, strPwdUser);
		encodeuu(strPwdUser, strPwdUser.size(), strBase64, true);
#ifdef _UNICODE
		wstringex	strW;
		ConvertStr(strBase64.c_str(), strBase64.size(), strW);
		g_Profile.WriteProfileString(_T("Base64ProxyUserPass"), strW.c_str());
#else
		g_Profile.WriteProfileString(_T("Base64ProxyUserPass"), strBase64.c_str());
#endif
	}
}

void CPreferPageInternet::OnDestroy()
{
	SaveProxyServerUserPwd();

	CPreferencePageBase::OnDestroy();
}

void CPreferPageInternet::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_LOAD_IE_PROXY:
		{
			bool		bUseProxy;
			tstring		strSvr;
			int			nPort;
			if (LoadProxySvrFromIE(bUseProxy, strSvr, nPort))
			{
				int		nHttpProxyType;

				// 代理类型
				if (bUseProxy)
					nHttpProxyType = HTTP_PROXY_OURS;
				else
					nHttpProxyType = HTTP_PROXY_NONE;

				CheckButton(IDC_PROXY_NONE, bUseProxy);
				CheckButton(IDC_PROXY_OURS, !bUseProxy);

				g_Profile.WriteProfileInt(_T("ProxyType"), nHttpProxyType);

				SetDlgItemText(IDC_HTTP_PROXY, strSvr.c_str());
				SetDlgItemInt(IDC_E_PORT, nPort, false);
			}
		}
		break;
	case IDC_TEST:
		{
			CINetTestDlg		dlg;

			SaveProxyServerUserPwd();

			dlg.DoModal(this);
		}
		break;
	case IDC_PROXY_OURS:
	case IDC_PROXY_NONE:
		{
			//
			// HTTP 代理设置
			//
			int		nHttpProxyType;

			// 代理类型
			if (IsButtonChecked(IDC_PROXY_OURS))
				nHttpProxyType = HTTP_PROXY_OURS;
			else
				nHttpProxyType = HTTP_PROXY_NONE;
			g_Profile.WriteProfileInt(_T("ProxyType"), nHttpProxyType);
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

#ifndef _MPLAYER

ID_TO_STRING		_id2strPlayer[] = { { IDC_WINAMP2, _T("Winamp2") }, { IDC_WINAMP5, _T("Winamp5") },
{ IDC_WMP, _T("Windows Media Player") }, { IDC_FOOBAR, _T("Foobar2000") }, 
{ IDC_REALONE, _T("RealPlayer") }, { IDC_QCD, _T("Quintessential Player") }, 
{ IDC_ITUNES, _T("iTunes") }, { IDC_SILVERJUKE, _T("Silverjuke") },
{ IDC_MEDIAMONKEY, _T("MediaMonkey") }, { IDC_KMPLAYER, _T("KMPlayer") }, 
{ IDC_JETAUDIO, _T("JetAudio") }, { IDC_YME, _T("Yahoo! Music Jukebox") }, 
{ IDC_BSPLAYER, _T("Bsplayer") }, { IDC_MEDIAJUKEBOX, _T("Media Jukebox") }, 
{ IDC_XMPLAY, _T("XMPlay") }, { IDC_AIMP2, _T("AIMP2") },
{ IDC_VLC, _T("VLC Media Player") },
{ 0, NULL }
};

#endif // #ifndef _MPLAYER

CPPageStartup::CPPageStartup() : CPreferencePageBase(IDD_PAGE_STARTUP, _T("Startup"))
{
	m_PageID = PAGE_STARTUP;
}

CPPageStartup::~CPPageStartup()
{
}

BOOL CPPageStartup::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	AddOptBool(ET_NULL, SZ_SECT_UI, _T("CheckNewVer"), true, IDC_C_CHECK_NEW_VERSION);

	InitCheckButtons();

#ifndef _MPLAYER
	for (int i = 0; _id2strPlayer[i].szId != NULL; i++)
	{
		if (g_Profile.GetProfileInt(_id2strPlayer[i].szId, TRUE))
			CheckButton(_id2strPlayer[i].dwId, true);
	}
#endif // #ifndef _MPLAYER

	return TRUE;
}

void CPPageStartup::OnCommand(UINT uID, UINT nNotifyCode)
{
	CPreferencePageBase::OnCommand(uID, nNotifyCode);

#ifndef _MPLAYER
	//
	// Should MiniLyrics get started with the checked player?
	//
	LPCTSTR		szPlayer;
	szPlayer = IDToString(_id2strPlayer, uID, _T(""));
	if (!IsEmptyString(szPlayer))
	{
		g_Profile.WriteProfileInt(szPlayer, IsButtonChecked(uID));
		return;
	}
#endif // #ifndef _MPLAYER

	ProcessCommand(uID, nNotifyCode);
}

void CPPageStartup::OnDestroy()
{
#ifndef _MPLAYER
	RegWriteProfileInt(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\UIPlugins\\{46B5EE7F-3B6B-4079-A756-5EFC10B1F50B}"), 
		_T("Running"), IsButtonChecked(IDC_WMP));
#endif // #ifndef _MPLAYER

	CPreferencePageBase::OnDestroy();
}
