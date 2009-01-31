// LyricsOutPluginMgr.cpp: implementation of the CLyricsOutPluginMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "../MPlayerApp.h"
#include "../Helper.h"
#include "LyricsOutPluginMgr.h"

CLyricsOutPluginMgr		g_lyrOutPlguinMgr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLyricsOutPluginMgr::CLyricsOutPluginMgr()
{

}

CLyricsOutPluginMgr::~CLyricsOutPluginMgr()
{

}

int CLyricsOutPluginMgr::Init()
{
	return ERR_OK;
}

void CLyricsOutPluginMgr::Quit()
{
	V_PLUGINS::iterator		it, itEnd;

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
		{
			plugin.plyricsOut->OnQuit();
		}
	}
	m_vPlugins.clear();
}

void CLyricsOutPluginMgr::GetLoadedPlugins(vector<tstring> &vPlugins)
{
	V_PLUGINS::iterator		it, itEnd;
	TCHAR		szFileName[MAX_PATH];
	tstring		str;

	itEnd = m_vPlugins.end();
	for (it = m_vPlugins.begin(); it != itEnd; ++it)
	{
		Plugins		&plugin = *it;
		if (plugin.bInitOK)
		{
			FileGetName(szFileName, CountOf(szFileName), plugin.strPluginFile.c_str());
			if (plugin.bInitOK)
			{
				str = plugin.plyricsOut->GetDescription();
			}
			else
			{
				str = _T("Failed to load.");
			}
			str += _T(" (");
			str += szFileName;
			str += _T(")");
			vPlugins.push_back(str);
		}
	}
}

void CLyricsOutPluginMgr::ConfigurePlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= m_vPlugins.size())
		return;

	Plugins		&plugin = m_vPlugins[nPluginIdx];
	if (plugin.bInitOK)
		plugin.plyricsOut->Config(pWndParent->GetHandle());
#endif
}

void CLyricsOutPluginMgr::AboutPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= m_vPlugins.size())
		return;

	Plugins		&plugin = m_vPlugins[nPluginIdx];
	if (plugin.bInitOK)
		plugin.plyricsOut->About(pWndParent->GetHandle());
#endif
}

void CLyricsOutPluginMgr::UninstPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
#ifdef _WIN32
	if (nPluginIdx < 0 || nPluginIdx >= m_vPlugins.size())
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
		CMLyrLines		&lyrLines = g_LyricData.GetRawLyrics();
		PML_ROW			pRow;

		int				nCurLineOld = m_nCurLine;
		bool			bHalfOfCurLineOld = m_bHalfOfCurLine;

		if (uPos < m_nCLBegTime || uPos >= m_nNLBegTime || m_nCurLine == -1
			|| m_nCurLine >= lyrLines.size())
		{
			m_nCurLine = g_LyricData.GetCurPlayLine(lyrLines);
			if (m_nCurLine >= 0 && m_nCurLine < lyrLines.size())
			{
				pRow = lyrLines[m_nCurLine];

				m_nCLBegTime = pRow->nBegTime;
				m_nCLendtime = pRow->nEndTime;
				m_nCLHalftime = (m_nCLBegTime + m_nCLendtime) / 2;

				if (m_nCurLine + 1 < lyrLines.size())
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
	CVMLLines		&lyrLines = g_LyricData.GetRawLyrics();
	return lyrLines.size();
}

bool CLyricsOutPluginMgr::GetLyricsOfLine(int nLine, TCHAR szLyrics[], int nBuffLen, int &nBegTime, int &nEndTime)
{
	CMLyrLines		&lyrLines = g_LyricData.GetRawLyrics();
	if (nLine < 0 || nLine >= lyrLines.size())
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
	CMLyrLines		&lyrLines = g_LyricData.GetRawLyrics();

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
	return !EmptyStr(szFile);
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
