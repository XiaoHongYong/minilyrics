// LyricsOutPluginMgr.h: interface for the CLyricsOutPluginMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSOUTPLUGINMGR_H__BF14D1D9_B19E_4E8D_9606_08A1B168E815__INCLUDED_)
#define AFX_LYRICSOUTPLUGINMGR_H__BF14D1D9_B19E_4E8D_9606_08A1B168E815__INCLUDED_

#include "../LyricsOutPlugin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLyricsOutPluginMgr : public IMLyrOutHost
{
public:
	struct Plugins
	{
		tstring		strPluginFile;
		ILyricsOut	*plyricsOut;
		bool		bInitOK;
		unsigned int uNotifyFlag;
	};

	CLyricsOutPluginMgr();
	virtual ~CLyricsOutPluginMgr();

	int Init();
	void Quit();

	bool IsActive() { return m_bActive; }

	void GetLoadedPlugins(vector<tstring> &vPlugins);
	void ConfigurePlugin(int nPluginIdx, CBaseWnd *pWndParent);
	void AboutPlugin(int nPluginIdx, CBaseWnd *pWndParent);
	void UninstPlugin(int nPluginIdx, CBaseWnd *pWndParent);

	void OnPlayPos(int uPos);
	void OnLyricsChanged();
	void OnSongChanged();

	virtual int GetLineCount();
	virtual bool GetLyricsOfLine(int nLine, TCHAR szLyrics[], int nBuffLen, int &nBegTime, int &nEndTime);
	virtual int GetCurLine();
	virtual int GetPlayPos();
	virtual bool GetMediaFile(TCHAR szFile[], int nBuffLen);
#ifdef _WIN32
	virtual HWND GetMainWnd();
#endif

protected:
	typedef vector<Plugins>		V_PLUGINS;
	V_PLUGINS				m_vPlugins;
	bool					m_bActive;
	DWORD					m_dwNotifyFlag;

	int						m_nCurLine;
	bool					m_bHalfOfCurLine;
	int						m_nCLBegTime, m_nCLHalftime, m_nCLendtime;	// CL = cur line
	int						m_nNLBegTime, m_nNLEndtime;

};

extern CLyricsOutPluginMgr		g_lyrOutPlguinMgr;

#endif // !defined(AFX_LYRICSOUTPLUGINMGR_H__BF14D1D9_B19E_4E8D_9606_08A1B168E815__INCLUDED_)
