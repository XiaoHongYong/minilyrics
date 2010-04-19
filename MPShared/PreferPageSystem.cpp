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

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();


		//
		// HTTP ´úÀíÉèÖÃ
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

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

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

void CPagePfSystemRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

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
