// LyricsOutPluginMgr.cpp: implementation of the CLyricsOutPluginMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsOutPluginMgr.h"
#include "Helper.h"

typedef int (*FunInitMLyricsOutPlugin)(HINSTANCE hDllInstance, IMLyrOutHost *pMLyrOutHost, ILyricsOut **ppLyricsOut);

CLyricsOutPluginMgr		g_lyrOutPlguinMgr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLyricsOutPluginMgr::CLyricsOutPluginMgr()
{
	m_bActive = false;
	m_bHalfOfCurLine = 0;
	m_dwNotifyFlag = 0;
	m_nCLBegTime = m_nCLHalftime = m_nCLendtime = 0;
	m_nNLBegTime = m_nNLEndtime = 0;
	m_nCurLine = 0;
}

CLyricsOutPluginMgr::~CLyricsOutPluginMgr()
{

}

void PromptForLogitechLCD(LPCTSTR szModuleName)
{
	if (_tcsicmp(FileGetName(szModuleName), _T("mlp_G15.dll")) != 0)
		return;

	if (g_Profile.GetProfileBool(_T("G15LCDPrompted"), false))
		return;

	g_Profile.WriteProfileInt(_T("G15LCDPrompted"), true);

	tstring	strMsg = TOLOCAL(_T("You have Logitech LCD device installed, lyrics can be displayed on it."));
	strMsg += _T("\r\n");
	strMsg += TOLOCAL(_T("Do you want to visit our website for more information?"));
	int nRet = g_mpApp.MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL);
	if (nRet == IDOK)
		OpenUrl(g_mpApp.GetMainWnd(), GetStrName(SN_HTTP_HELP_G15_LCD));
}

int CLyricsOutPluginMgr::Init()
{
	int				nRet;
	CMLFileFind		find;
	HMODULE			hModule;
	tstring			strPluginDir;
	tstring			strPluginFile;
	FunInitMLyricsOutPlugin	funInitMLyricsOutPlugin;

	// Uninstall the plug-in which was selected last time.
	strPluginFile = g_Profile.GetProfileString(_T("UninstLyrOutPluginFile"), _T(""));
	if (!strPluginFile.empty())
	{
		SetFileNoReadOnly(strPluginFile.c_str());
		if (DeleteFile(strPluginFile.c_str()))
		{
			g_Profile.WriteProfileString(_T("UninstLyrOutPluginFile"), _T(""));
		}
	}

	m_dwNotifyFlag = 0;

	strPluginDir = GetWorkingFolder();
	strPluginDir += _T("Plugins\\");

	DBGLOG0(_T("Begin to load lyrics output plugins."));

	if (!find.OpenDir(strPluginDir.c_str(), _T("*.dll")))
	{
		DBGLOG1(_T("Can't open plugin dir: %s"), strPluginDir.c_str());
		return ERR_OK;
	}

	while (find.FindNext())
	{
		if (find.IsCurDir())
			continue;

		if (_tcsnicmp(find.GetCurName(), _T("mlp_"), 4) == 0)
		{
			strPluginFile = strPluginDir;
			strPluginFile += find.GetCurName();
			hModule = LoadLibrary(strPluginFile.c_str());
			if (!hModule)
			{
				ERRLOG2(_T("Failed to Load plugin: %s, %s"), strPluginFile.c_str(), CWin32Error().Description());
				continue;
			}

			funInitMLyricsOutPlugin = (FunInitMLyricsOutPlugin)GetProcAddress(hModule, "InitMLyricsOutPlugin");
			if (funInitMLyricsOutPlugin)
			{
				Plugins	plugin;
				nRet = (*funInitMLyricsOutPlugin)(hModule, this, &plugin.plyricsOut);
				if (nRet != ML_LYR_OUT_VERSION || plugin.plyricsOut == NULL)
				{
					FreeLibrary(hModule);
					g_mpApp.MessageOut(CStrPrintf(_T("Init lyrics output plugin: %s FAILED!\r\nMaybe incorrect plugin version!"), strPluginFile.c_str()).c_str());
				}
				else
				{
					plugin.strPluginFile = strPluginFile;
					plugin.hModule = hModule;
					plugin.uNotifyFlag = 0;
					plugin.bInitOK = plugin.plyricsOut->OnInit(&plugin.uNotifyFlag);
					if (plugin.bInitOK)
					{
						m_dwNotifyFlag |= plugin.uNotifyFlag;
						m_vPlugins.push_back(plugin);
						PromptForLogitechLCD(strPluginFile.c_str());
					}
				}
			}
			else
			{
				FreeLibrary(hModule);
				DBGLOG1(_T("GetProAddress FAILED: %s, InitMLyricsOutPlugin"), strPluginFile.c_str());
			}
		}
	}

	m_bActive = !m_vPlugins.empty();
	if (m_bActive)
	{
		g_eventDispather.RegisterHanlder(ET_LYRICS_CHANGED, this);
		g_eventDispather.RegisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

	return ERR_OK;
}

void CLyricsOutPluginMgr::Quit()
{
	V_PLUGINS::iterator		it, itEnd;

	if (m_bActive)
	{
		g_eventDispather.UnregisterHanlder(ET_LYRICS_CHANGED, this);
		g_eventDispather.UnregisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
			plugin.plyricsOut->OnQuit();
		FreeLibrary(plugin.hModule);
	}
	m_vPlugins.clear();
}

void CLyricsOutPluginMgr::GetLoadedPlugins(vector<tstring> &vPlugins)
{
	V_PLUGINS::iterator		it, itEnd;
	tstring		str;

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
		{
			if (plugin.bInitOK)
			{
				str = plugin.plyricsOut->GetDescription();
			}
			else
			{
				str = _T("Failed to load.");
			}
			str += _T(" (");
			str += FileGetName(plugin.strPluginFile.c_str());
			str += _T(")");
			vPlugins.push_back(str);
		}
	}
}

void CLyricsOutPluginMgr::ConfigurePlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size())
		return;

	Plugins		&plugin = m_vPlugins[nPluginIdx];
	if (plugin.bInitOK)
		plugin.plyricsOut->Config(pWndParent->GetHandle());
#endif
}

void CLyricsOutPluginMgr::AboutPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size())
		return;

	Plugins		&plugin = m_vPlugins[nPluginIdx];
	if (plugin.bInitOK)
		plugin.plyricsOut->About(pWndParent->GetHandle());
#endif
}

void CLyricsOutPluginMgr::UninstPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size())
		return;

	Plugins		&plugin = m_vPlugins[nPluginIdx];
	if (plugin.bInitOK)
	{
		int		nRet;
		tstring	str = TOLOCAL(_T("Are you sure you want to permanently uninstall this plug-in?"));
		str += _T("\n");
		str += TOLOCAL(_T("$Product$ will uninstall it at next restart."));
		nRet = pWndParent->MessageOut(str.c_str(), MB_YESNO);
		if (nRet == IDYES)
			g_Profile.WriteProfileString(_T("UninstLyrOutPluginFile"), plugin.strPluginFile.c_str());
	}
#endif
}

// IEventHandler
void CLyricsOutPluginMgr::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_LYRICS_CHANGED)
	{
		OnLyricsChanged();
	}
	else if (pEvent->eventType == ET_LYRICS_DRAW_UPDATE)
	{
		OnPlayPos(g_LyricData.GetPlayElapsedTime());
	}
	else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
	{
		OnSongChanged();
	}
}

void CLyricsOutPluginMgr::OnPlayPos(int uPos)
{
	V_PLUGINS::iterator		it, itEnd;

	itEnd = m_vPlugins.end();

	if (m_dwNotifyFlag & ILyricsOut::NOTIF_PLAY_POS)
	{
		for (it = m_vPlugins.begin(); it != itEnd; ++it)
		{
			Plugins		&plugin = *it;
			if (plugin.bInitOK)
			{
				plugin.plyricsOut->OnPlayPos(uPos);
			}
		}
	}

	if (m_dwNotifyFlag & (ILyricsOut::NOTIF_CUR_LINE | ILyricsOut::NOTIF_HALF_CUR_LINE))
	{
		VLyrLines		&lyrLines = g_LyricData.GetRawLyrics();
		PML_ROW			pRow;

		int				nCurLineOld = m_nCurLine;
		bool			bHalfOfCurLineOld = m_bHalfOfCurLine;

		if (uPos < m_nCLBegTime || uPos >= m_nNLBegTime || m_nCurLine == -1
			|| m_nCurLine >= (int)lyrLines.size())
		{
			m_nCurLine = g_LyricData.GetCurPlayLine(lyrLines);
			if (m_nCurLine >= 0 && m_nCurLine < (int)lyrLines.size())
			{
				pRow = lyrLines[m_nCurLine];

				m_nCLBegTime = pRow->nBegTime;
				m_nCLendtime = pRow->nEndTime;
				m_nCLHalftime = (m_nCLBegTime + m_nCLendtime) / 2;

				if (m_nCurLine + 1 < (int)lyrLines.size())
				{
					pRow = lyrLines[m_nCurLine + 1];
					m_nNLBegTime = pRow->nBegTime;
					m_nNLEndtime = pRow->nEndTime;
				}
				else
					m_nNLBegTime = m_nNLEndtime = pRow->nEndTime;
				if (uPos >= m_nCLHalftime)
					m_bHalfOfCurLine = true;
				else
					m_bHalfOfCurLine = false;
			}
			else
				m_bHalfOfCurLine = false;
		}

		if (!m_bHalfOfCurLine && uPos >= m_nCLHalftime)
			m_bHalfOfCurLine = true;

		if (m_nCurLine != nCurLineOld)
		{
			tstring			strLyrCurline;
			if (m_nCurLine != -1)
			{
				pRow = lyrLines[m_nCurLine];
				g_LyricData.LyricsRowToText(pRow, strLyrCurline);
			}

			for (it = m_vPlugins.begin(); it != itEnd; ++it)
			{
				Plugins		&plugin = *it;
				if (plugin.bInitOK && (plugin.uNotifyFlag & ILyricsOut::NOTIF_CUR_LINE))
				{
					plugin.plyricsOut->OnCurLineChanged(m_nCurLine, strLyrCurline.c_str(), m_nCLBegTime, m_nCLendtime);
				}
			}
		}

		if (m_bHalfOfCurLine && m_bHalfOfCurLine != bHalfOfCurLineOld)
		{
			for (it = m_vPlugins.begin(); it != itEnd; ++it)
			{
				Plugins		&plugin = *it;
				if (plugin.bInitOK && (plugin.uNotifyFlag & ILyricsOut::NOTIF_HALF_CUR_LINE))
				{
					plugin.plyricsOut->OnHalfOfCurLine(m_nCurLine);
				}
			}
		}
	}
}

void CLyricsOutPluginMgr::OnSongChanged()
{
	V_PLUGINS::iterator		it, itEnd;

	if (m_vPlugins.empty())
		return;

	m_nCurLine = -1;

	int		nDuration;
	nDuration = g_Player.GetMediaLength();

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
		{
			plugin.plyricsOut->OnSongChanged(g_Player.GetArtist(), g_Player.GetTitle(), nDuration);
		}
	}
}

void CLyricsOutPluginMgr::OnLyricsChanged()
{
	V_PLUGINS::iterator		it, itEnd;

	m_nCurLine = -1;

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
		{
			plugin.plyricsOut->OnLyricsChanged();
		}
	}
}

int CLyricsOutPluginMgr::GetLineCount()
{
	VLyrLines		&lyrLines = g_LyricData.GetRawLyrics();
	return lyrLines.size();
}

bool CLyricsOutPluginMgr::GetLyricsOfLine(int nLine, TCHAR szLyrics[], int nBuffLen, int &nBegTime, int &nEndTime)
{
	VLyrLines		&lyrLines = g_LyricData.GetRawLyrics();
	if (nLine < 0 || nLine >= (int)lyrLines.size())
		return false;

	tstring			str;
	PML_ROW			pRow = lyrLines[nLine];
	g_LyricData.LyricsRowToText(pRow, str);
	_tcscpy_safe(szLyrics, nBuffLen, str.c_str());
	nBegTime = pRow->nBegTime;
	nEndTime = pRow->nEndTime;

	return true;
}

int CLyricsOutPluginMgr::GetCurLine()
{
	VLyrLines		&lyrLines = g_LyricData.GetRawLyrics();

	return g_LyricData.GetCurPlayLine(lyrLines);
}

int CLyricsOutPluginMgr::GetPlayPos()
{
	return g_LyricData.GetPlayElapsedTime();
}

bool CLyricsOutPluginMgr::GetMediaFile(TCHAR szFile[], int nBuffLen)
{
	EmptyStr(szFile);
	_tcscpy_safe(szFile, nBuffLen, g_Player.GetSrcMedia());
	return !IsEmptyString(szFile);
}

#ifdef _WIN32
HWND CLyricsOutPluginMgr::GetMainWnd()
{
	if (g_mpApp.GetMainWnd())
		return g_mpApp.GetMainWnd()->GetHandle();
	else
		return NULL;
}
#endif
