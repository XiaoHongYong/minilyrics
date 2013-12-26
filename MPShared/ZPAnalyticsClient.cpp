#include "StdAfx.h"
#include "ZPAnalyticsClient.h"
#include "MPlayerAppBase.h"
#include "MLVersionStr.h"
#include "LyricShowObj.h"

#ifdef _MPLAYER
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ZP")
#else
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ML")

#ifdef _WIN32
#include "win32/PlayerPluginInstaller.h"
#endif
#endif

// Ping type for MiniLyrics
#define SZ_UD_SETTINGS_NORMAL_MODE		_T("ST_N")		// Option List values
#define SZ_UD_SETTINGS_FLOATING_LYR		_T("ST_F")		// Option List values

// General Setting properties
#define SZ_N_ST_CUR_SKIN				_T("SK")		// Current skin
#define SZ_N_ST_BG_IMAGE				_T("BGI")
#define SZ_N_ST_USE_ALBUM_ARG_BG		_T("UABG")
#define SZ_N_ST_DARK_LYR_BG				_T("DLBG")
#define SZ_N_ST_SLIDE_DELAY_DURATION	_T("DLBG")

// Normal mode or Floating lyrics mode setting properties
#define SZ_N_ST_FONT_NAME				_T("FN")
#define SZ_N_ST_FONT_NAME_OTHERS		_T("FNO")
#define SZ_N_ST_FONT_SIZE				_T("FS")
#define SZ_N_ST_FONT_BOLD				_T("FB")
#define SZ_N_ST_FONT_ITALIC				_T("FI")
#define SZ_N_ST_LYR_DISPLAY_OPT			_T("LDO")
#define SZ_N_ST_LYR_DISPLAY_STYLE		_T("LDS")
#define SZ_N_ST_KARAOKE					_T("KOK")
#define SZ_N_ST_LYRICS_ALIGN			_T("LA")
#define SZ_N_ST_BG_COLOR				_T("BC")
#define SZ_N_ST_FG_COLOR				_T("FC")
#define SZ_N_ST_OVERLAY_BLENDING_MODE	_T("OBM")
#define SZ_N_ST_OUTLINE_LYR				_T("OL")
#define SZ_N_ST_				_T("")

tstring LicenseStatusToString(LicenseStatus ls)
{
	switch (ls)
	{
	case LS_ACTIVE: return _T("Active");
	case LS_TRIAL: return _T("Trail");
	case LS_EXPIRED: return _T("Expired");
	case LS_REVOKED: return _T("Revoked");
	case LS_TRANSFERED: return _T("Transfered");
	default: return StringFromInt(ls);
	}
}

//////////////////////////////////////////////////////////////////////////

CZPAnalyticsClient::CZPAnalyticsClient() :
	CAppAnalyticsClient(SZ_APP_ANALYTICS_PRODUCT_ID, _T(ML_VERSION_STR))
{
}

CZPAnalyticsClient::~CZPAnalyticsClient()
{
}

#ifndef _MPLAYER
tstring ConvertPlayerName()
{
	if (CMPlayerApp::GetInstance()->GetCurrentAppMode() == SA_IPOD_LYRICS_DOWNLOADER)
		return _T("LyricsDownloader");

	tstring name = g_Player.GetPlayerName();
	for (size_t i = 0; i < name.size(); i++)
	{
		if (!IsAlphaDigit(name[i]))
			name.replace(i, 1, 1, '_');
	}
	return name;
}
#endif // #ifndef _MPLAYER

void CZPAnalyticsClient::Quit()
{
#ifndef _MPLAYER
	MapString &usageData = GetUsageData(SZ_UD_PLAYER_RUN_TIME);

	uint32 nAppRunningTimeSec = (GetTickCount() - m_nAppStartTime) / 1000;
	tstring playerName = ConvertPlayerName();
	IncreaseValue(usageData, (_T("N_") + playerName).c_str(), 1);
	IncreaseValue(usageData, (_T("T_") + playerName).c_str(), nAppRunningTimeSec);
#endif // #ifndef _MPLAYER

	CAppAnalyticsClient::Quit();
}

void CZPAnalyticsClient::IncreaseCmdClickCount(int nUID, CmdAccessMethod cam)
{
	tstring strID = CMPlayerApp::GetMPSkinFactory()->GetStringOfID(nUID);
	if (strID.size() > 0)
		CAppAnalyticsClient::IncreaseCmdClickCount(strID.c_str(), cam);
}

void CZPAnalyticsClient::IncreaseNumberOfResult(LPCTSTR szType, int nResult)
{
	CStrPrintf name(_T("%s_%d"), SZ_N_NUMBER_OF_RESULT, nResult);

	IncreaseValue(szType, name.c_str(), 1);
}

#ifdef _DEBUG
void CZPAnalyticsClient::TestSend()
{
	m_lastSendDate.FromTime(CBaseDate::GetCurrentDate().GetTime() - CBaseDate::SECOND_IN_ONE_DAY * 8);
	m_lastWeeklyPingDate.FromTime(CBaseDate::GetCurrentDate().GetTime() - CBaseDate::SECOND_IN_ONE_DAY * 8);

	CheckSendTask();
}
#endif

tstring CZPAnalyticsClient::GetAnalyticsFile()
{
	tstring file = GetAppDataDir();
	file += _T("MiniLyrics.aa");
	return file;
}

bool CZPAnalyticsClient::GetProxy(tstring &strServer, int &port, tstring &base64UserPass)
{
	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];

	if (CMLProfile::InetGetProxy(szProxyServer, nProxyPort))
	{
		strServer = szProxyServer;
		base64UserPass = CMLProfile::InetGetBase64ProxyUserPass();
		return true;
	}

	return false;
}

void CZPAnalyticsClient::PrepareProductInfo(MapString &ping, bool bInstallPing, bool bWeeklyPing)
{
	CAppAnalyticsClient::PrepareProductInfo(ping, bInstallPing, bWeeklyPing);

	CLicenseManager::LicenseInfo li;
	g_licenseManager.GetLicenseInfo(li);

	ping[SZ_N_PI_DEVICE_ID] = li.strDeviceID;
}

void CZPAnalyticsClient::PrepareLicenseStatusData(MapString &ping)
{
	CLicenseManager::LicenseInfo li;
	g_licenseManager.GetLicenseInfo(li);

	ping[SZ_N_LS_LICENSE_STATUS] = LicenseStatusToString(li.licenseStatus);
	if (li.licenseStatus == LS_TRIAL)
		return;

	if (li.strCode.size() > 0)
		ping[SZ_N_LS_CODE] = li.strCode;
	ping[SZ_N_LS_NAME] = li.strName;
	ping[SZ_N_LS_START_DATE] = li.strStartDate;
	ping[SZ_N_LS_DURATION_DAYS] = StringFromInt(li.nDurationDays);
	ping[SZ_N_LS_LICENSED_USERS] = StringFromInt(li.nLicensedUser);
	ping[SZ_N_LS_ONLINE_VERIFIED] = StringFromInt(li.bOnlineVerified);
}

inline tstring GetCurSkin()
{
	tstring curSkinWnd = CMPlayerApp::GetMainWnd()->GetSkinWndName();
	if (_tcsicmp(curSkinWnd.c_str(), _T("MiniLyrics")) == 0
		|| _tcsicmp(curSkinWnd.c_str(), _T("ZikiPlayer")) == 0)
		return CMPlayerApp::GetInstance()->GetDefaultSkin();
	else
		return CMPlayerApp::GetInstance()->GetDefaultSkin() + _T("#") + curSkinWnd;
}

void CZPAnalyticsClient::PrepareSettingsData(MapString &ping)
{
	ping[SZ_N_ST_CUR_SKIN] = GetCurSkin();
	ping[SZ_N_ST_BG_IMAGE] = g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("UseBgImage"), _T(""));
	ping[SZ_N_ST_USE_ALBUM_ARG_BG] = g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("UseAlbumArtAsBg"), _T(""));
	ping[SZ_N_ST_DARK_LYR_BG] = g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("DarkenLyrBg"), _T(""));
	ping[SZ_N_ST_SLIDE_DELAY_DURATION] = g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("SlideDelayTime"), _T(""));
}

void CZPAnalyticsClient::PrepareOtherUsageData()
{
	PrepareModeRelatedSettingsData(SZ_SECT_UI, GetUsageData(SZ_UD_SETTINGS_NORMAL_MODE), false);
	PrepareModeRelatedSettingsData(SZ_SECT_FLOATING_LYR, GetUsageData(SZ_UD_SETTINGS_FLOATING_LYR), true);
}

void CZPAnalyticsClient::PrepareModeRelatedSettingsData(LPCTSTR szSection, MapString &ping, bool bFloatingLyr)
{
	// Font related
	int nHeight, nWeight;
	BYTE byItalic;
	tstring strFaceNameLatin9, strFaceNameOthers;
	ProfileGetLyricsFont(szSection, nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers);
	ping[SZ_N_ST_FONT_NAME] = strFaceNameLatin9;
	ping[SZ_N_ST_FONT_NAME_OTHERS] = strFaceNameOthers;
	ping[SZ_N_ST_FONT_BOLD] = StringFromInt(nWeight);
	ping[SZ_N_ST_FONT_ITALIC] = StringFromInt(byItalic);
	ping[SZ_N_ST_LYR_DISPLAY_OPT] = g_Profile.GetString(szSection, _T("LyrDrawOpt"), _T(""));
	ping[SZ_N_ST_KARAOKE] = g_Profile.GetString(szSection, _T("Karaoke"), _T(""));
	ping[SZ_N_ST_LYRICS_ALIGN] = g_Profile.GetString(szSection, _T("VertAlign"), _T(""));
	ping[SZ_N_ST_BG_COLOR] = g_Profile.GetString(szSection, _T("BgColor"), _T(""));
	ping[SZ_N_ST_OUTLINE_LYR] = g_Profile.GetString(szSection, _T("OutlineLyrText"), _T(""));
	if (bFloatingLyr)
		ping[SZ_N_ST_LYR_DISPLAY_STYLE] = g_Profile.GetString(SZ_SECT_FLOATING_LYR, _T("FloatingLyrDispStyle"), _T(""));
	else
		ping[SZ_N_ST_LYR_DISPLAY_STYLE] = g_Profile.GetString(SZ_SECT_LYR_DISPLAY, _T("LyrDisplayStyle"), _T(""));
}
