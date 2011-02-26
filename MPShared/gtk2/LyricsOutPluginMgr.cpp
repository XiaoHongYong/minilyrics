// LyricsOutPluginMgr.cpp: implementation of the CLyricsOutPluginMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
}

void CLyricsOutPluginMgr::GetLoadedPlugins(vector<tstring> &vPlugins)
{
/*	V_PLUGINS::iterator		it, itEnd;
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
	}*/
}

void CLyricsOutPluginMgr::ConfigurePlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
}

void CLyricsOutPluginMgr::AboutPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
}

void CLyricsOutPluginMgr::UninstPlugin(int nPluginIdx, CBaseWnd *pWndParent)
{
}

void CLyricsOutPluginMgr::OnPlayPos(int uPos)
{
}

void CLyricsOutPluginMgr::OnSongChanged()
{
}

void CLyricsOutPluginMgr::OnLyricsChanged()
{
}

int CLyricsOutPluginMgr::GetLineCount()
{
}

bool CLyricsOutPluginMgr::GetLyricsOfLine(int nLine, TCHAR szLyrics[], int nBuffLen, int &nBegTime, int &nEndTime)
{
	return false;
}

int CLyricsOutPluginMgr::GetCurLine()
{
	return 0;
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
	if (CMPlayerAppBase::GetMainWnd())
		return CMPlayerAppBase::GetMainWnd()->GetHandle();
	else
		return NULL;
}
#endif
