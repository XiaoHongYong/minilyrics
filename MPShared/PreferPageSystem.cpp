#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageSystem.h"


//////////////////////////////////////////////////////////////////////////

class CPagePfInternet : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfInternet() : CPagePfBase(PAGE_INET, _T("CMD_SYSTEM_INTERNET"))
	{
		m_bInitailed = false;
	}

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
		tstring strPwd = g_Profile.EncryptGetProfileString(_T("ProxyPassEnc"), _T(""));
		if (strPwd.empty())
			strPwd = g_Profile.GetProfileString(_T("ProxyPass"), _T(""));
		SetUIObjectText(_T("CID_PASS"), strPwd.c_str());

		m_bInitailed = true;
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
		if (!m_bInitailed)
			return;

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
			stringex		strPwdUser;
			TCHAR			szPwdUser[256];

			_stprintf_s(szPwdUser, CountOf(szPwdUser), _T("%s:%s"), strUser.c_str(), strPwd.c_str());
			ConvertStr(szPwdUser, -1, strPwdUser);
			tstring		strBase64;
			encodeuu(strPwdUser, strPwdUser.size(), strBase64, true, false);
			g_Profile.WriteProfileString(_T("Base64ProxyUserPass"), strBase64.c_str());
		}
	}

	bool		m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfInternet, _T("Container.Inet"))

//////////////////////////////////////////////////////////////////////////

#ifndef _MPLAYER

ID_TO_STRING		_id2strPlayers[] = { { 0, _T("Winamp2") }, { 0, _T("Winamp5") },
	{ 0, _T("Windows Media Player") }, { 0, _T("Foobar2000") }, 
	{ 0, _T("RealPlayer") }, { 0, _T("Quintessential Player") }, 
	{ 0, _T("iTunes") }, { 0, _T("Silverjuke") },
	{ 0, _T("MediaMonkey") }, { 0, _T("KMPlayer") }, 
	{ 0, _T("JetAudio") }, { 0, _T("Yahoo! Music Jukebox") }, 
	{ 0, _T("BSPlayer") }, { 0, _T("Media Jukebox") }, 
	{ 0, _T("XMPlay") }, { 0, _T("AIMP2") },
	{ 0, _T("VLC Media Player") },
	{ 0, NULL }
};

int   _idWmp = 0;

#endif // #ifndef _MPLAYER

class CPagePfStartup: public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfStartup() : CPagePfBase(PAGE_STARTUP, _T("CMD_SYSTEM_STARTUP"))
	{
		m_bInitailed = false;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		m_bInitailed = true;

		AddOptBool(ET_NULL, SZ_SECT_UI, _T("CheckNewVer"), true, _T("CID_C_CHECK_NEW_VERSION"));

		InitCheckButtons();

#ifndef _MPLAYER
		CUIObject *pObjFrame = GetUIObjectById(_T("CID_STARTUP_FRAME"), NULL);
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
				_id2strPlayers[i].dwId = m_pSkin->GetSkinFactory()->AllocUID();

			if (_tcscmp(_id2strPlayers[i].szId, _T("Windows Media Player")) == 0)
				_idWmp = _id2strPlayers[i].dwId;

			CSkinNStatusButton *pButton = (CSkinNStatusButton*)m_pSkin->GetSkinFactory()->CreateDynamicCtrl(
				this, _T("NormalCheckBox"), _id2strPlayers[i].dwId,
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
		if (m_bInitailed)
		{
			ASSERT(_idWmp != UID_INVALID);
			RegWriteProfileInt(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\UIPlugins\\{46B5EE7F-3B6B-4079-A756-5EFC10B1F50B}"), 
				_T("Running"), IsButtonChecked(_idWmp));
		}
#endif // #ifndef _MPLAYER

		CPagePfBase::OnDestroy();
	}

protected:
	bool		m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfStartup, _T("Container.Startup"))

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfSystemRoot, _T("PreferPage.SystemRoot"))

CPagePfSystemRoot::CPagePfSystemRoot() : CPagePfBase(PAGE_UNKNOWN, _T("CMD_ROOT_SYSTEM"))
{
}

void CPagePfSystemRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_SYSTEM"));
}

void RegisterPfSystemPages(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPagePfInternet);
	AddUIObjNewer2(pSkinFactory, CPagePfStartup);
	AddUIObjNewer2(pSkinFactory, CPagePfSystemRoot);
}

