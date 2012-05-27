
#pragma once

#include "win32/VolumeOutMaster.h"
#include "PlayerSmoothTimer.h"
#include "win32/MLPlayerMgr.h"

///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos of player timely.

class CPlayerEventDispatcher : public CBaseWnd
{
public:
	CPlayerEventDispatcher() { m_nTimeOutUpdateLyr = 40; }
	~CPlayerEventDispatcher() { }
	
	enum
	{
		TIMER_LYR_DRAW_UPDATE		= 1,
	};
	
	void Init()
	{
		CBaseWnd::CreateEx(MSG_WND_CLASS_NAME, _T("PlayerEventWnd"), 0, 0, 1, 1, NULL, 0);

		m_volMaster.Init(GetHandle());

		// latest playlist and latest playing song file will loaded while main window is created.
		LPCTSTR			szCmdLine;
		
		// get command line
		szCmdLine = GetCommandLine();
		szCmdLine = CmdLineNext(szCmdLine);
		if (_tcsicmp(szCmdLine, _T("/iPodLyricsDownloader")) == 0)
			szCmdLine = _T("");
		if (IsEmptyString(szCmdLine))
		{
			// Load default playlist
			tstring		strCurrentPlaylist;
			
			strCurrentPlaylist = g_Profile.GetProfileString(_T("Latest Playlist"), _T(""));
			if (!g_Player.LoadPlaylist(strCurrentPlaylist.c_str(), true))
				g_Player.NewCurrentPlaylist();

			int	nNowPlaying = g_Profile.GetProfileInt(SZ_SECT_PLAYER, _T("NowPlayingIdx"), 0);
			g_Player.SetCurrentMediaInPlaylist(nNowPlaying);
		}
		else
		{
			// excute cmdline
			SendCommandLine(CMPlayerAppBase::GetMainWnd()->GetHandle(), szCmdLine);
		}
	}
	
	void Quit()
	{
		m_playerSmoothTimer.Stop();

		m_volMaster.Quit();

		Destroy();
	}
	
	long GetMasterVolume()
	{
		return m_volMaster.GetVolume();
	}
	
	void SetMasterVolume(long volume)
	{
		m_volMaster.SetVolume(volume);
	}

	void SetLyrDrawUpdateFast(bool bFast)
	{
		if (g_Player.GetIMPlayer() != NULL)
		{
			if (bFast)
				m_nTimeOutUpdateLyr = 20;
			else
				m_nTimeOutUpdateLyr = 40;
			
			StartLyrDrawUpdate();
		}
		else
		{
			if (bFast)
			{
				if (!m_playerSmoothTimer.IsRunning())
					m_playerSmoothTimer.Start();
			}
			else
				m_playerSmoothTimer.Stop();
		}
	}
	
	void StartLyrDrawUpdate()
	{ SetTimer(TIMER_LYR_DRAW_UPDATE, m_nTimeOutUpdateLyr); }
	
	void StopLyrDrawUpdate()
	{ KillTimer(TIMER_LYR_DRAW_UPDATE); }
	
	bool OnCreate()
	{
		if (!CBaseWnd::OnCreate())
			return false;
		
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
		
		return true;
	}
	
	void OnPaint(CMLGraphics *pGraph, CRect &rcClip)
	{
		ShowWindow(SW_HIDE);
	}
	
	void OnTimer(UINT nIDEvent)
	{
		ASSERT(nIDEvent == TIMER_LYR_DRAW_UPDATE);
		
		if (g_Player.IsUseSeekTimeAsPlayingTime())
			return;
		
		int nPlayPos = g_Player.GetPlayPos();
		g_LyricData.SetPlayElapsedTime(nPlayPos);
		
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
		
		DispatchPlayPosEvent(nPlayPos);
	}
	
	
	void OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
	{
		if (wParam == ML_ACTIVATE && pcds->dwData == ML_ACTIVATE)
		{
			CMPSkinMainWnd *pWnd = CMPlayerAppBase::GetMainWnd();
			if (pWnd)
				pWnd->Activate();
			else
				ERRLOG0(_T("MiniLyrics main window hasn't been started yet, can't activate it."));
		}
		else if (wParam == ML_SEND_CMD_LINE && pcds->dwData == ML_SEND_CMD_LINE)
		{
#ifndef _MPLAYER
			AppMode appMode = SA_UNKNOWN;
			ProcessCmdLine((LPCTSTR)pcds->lpData, appMode);
			if (appMode == SA_IPOD_LYRICS_DOWNLOADER && appMode != CMPlayerApp::GetInstance()->GetCurrentAppMode())
			{
				// Change to another app mode
				CMPlayerApp::GetInstance()->RestartToAppMode(appMode);
			}
#endif
		}
	}
	
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_COPYDATA)
			OnCopyData(wParam, (COPYDATASTRUCT *)lParam);
#ifndef _MPLAYER
		else if (message == MM_MIXM_CONTROL_CHANGE)
		{
			// Volume changed message.
			CEventPlayerSettingChanged	*pEvent = new CEventPlayerSettingChanged();
			pEvent->eventType = ET_PLAYER_SETTING_CHANGED;
			pEvent->settingType = IMPEvent::MPS_VOLUME;
			pEvent->nValue = g_Player.GetVolume();
			
			CMPlayerApp::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
		}
#endif
		
		return CBaseWnd::WndProc(message, wParam, lParam);
	}

	CPlayerSmoothTimer &GetSmoothTimer() { return m_playerSmoothTimer; }
	
protected:
	int					m_nTimeOutUpdateLyr;
	CVolumeOutMaster	m_volMaster;
	CPlayerSmoothTimer	m_playerSmoothTimer;

};

extern CPlayerEventDispatcher		g_playerEventDispatcher;
