#ifndef _LYRICSOUTPLUGIN_H_
#define _LYRICSOUTPLUGIN_H_

#ifndef UNICODE
#pragma message("UNICODE must be defined!")
#endif

interface IMLyrOutHost
{
	virtual int GetLineCount() = 0;
	virtual bool GetLyricsOfLine(int nLine, TCHAR szLyrics[], int nBuffLen, int &nBegTime, int &nEndTime) = 0;
	virtual int GetCurLine() = 0;
	virtual int GetPlayPos() = 0;
	virtual bool GetMediaFile(TCHAR szFile[], int nBuffLen) = 0;
#ifdef _WIN32
	virtual HWND GetMainWnd() = 0;
#endif
};

#define ML_LYR_OUT_VERSION		2

interface ILyricsOut
{
	enum
	{
		NOTIF_CUR_LINE		= 1,			// OnCurLineChanged
		NOTIF_HALF_CUR_LINE	= 1 << 1,		// OnNextLine
		NOTIF_PLAY_POS		= 1 << 2,		// OnPlayPos
	};

#ifdef _WIN32
	virtual void Config(HWND hWndParent) = 0;
	virtual void About(HWND hWndParent) = 0;
#endif
	virtual LPCTSTR GetDescription() = 0;

	// uNotifyFlag, return NOTIF_CUR_LINE, NOTIF_...
	virtual bool OnInit(unsigned int *uNotifyFlag) = 0;

	virtual void OnCurLineChanged(int nLine, LPCTSTR szLyrics, int nBegTime, int nEndTime) = 0;
	virtual void OnHalfOfCurLine(int nLine) = 0;
	virtual void OnPlayPos(int uPos) = 0;
	virtual void OnLyricsChanged() = 0;
	virtual void OnSongChanged(LPCTSTR szArtist, LPCTSTR szTitle, int nDuration) = 0;
	virtual void OnQuit() = 0;

};



#endif // _LYRICSOUTPLUGIN_H_
