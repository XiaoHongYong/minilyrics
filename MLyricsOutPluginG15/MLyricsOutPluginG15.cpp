// MLyricsOutPlugin.cpp: implementation of the MLyricsOutPlugin class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include "MLyricsOutPluginG15.h"

HINSTANCE				g_hInst = NULL;
IMLyrOutHost			*g_pMLyrOutHost;
CG15LCDLyricsOutPlugin	g_g15LcdPlugin;

extern "C"
{
	__declspec (dllexport) int InitMLyricsOutPlugin(HINSTANCE hDllInstance, IMLyrOutHost *pMLyrOutHost, ILyricsOut **ppLyricsOut)
	{
		g_hInst = hDllInstance;
		*ppLyricsOut = &g_g15LcdPlugin;
		g_pMLyrOutHost = pMLyrOutHost;

		return ML_LYR_OUT_VERSION;
	}
};

//////////////////////////////////////////////////////////////////////////

#define PLAYER_POS_UPDATE_SPAN	500

#include "TinyBaseWnd.h"


class CG15TimerWnd : public CTinyBaseWnd
{
public:
	enum {
		TIMER_SHOW_LOGO		= 1,
		TIMER_CHECK_BTN		= 2,
		TIMER_POS_UPDATE	= 3,

		TIME_OUT_CHECK_BTN	= 50,
		TIME_OUT_POS_UPDATE	= 500,
	};

	virtual void OnCreate() {
		SetTimer(TIMER_SHOW_LOGO, 3000);
		// SetTimer(TIMER_CHECK_BTN, TIME_OUT_CHECK_BTN);
		SetTimer(TIMER_POS_UPDATE, TIME_OUT_POS_UPDATE);
	}

	virtual void OnDestroy() { }

	virtual void OnTimer(int nTimerID)
		{ g_g15LcdPlugin.OnTimer(nTimerID); }

public:

};

CG15TimerWnd		g_wndG15Timer;

#define SZ_PRODUCT_NAME		_T("MiniLyrics for Logitech LCD")

//////////////////////////////////////////////////////////////////////////

CG15LCDLyricsOutPlugin::CG15LCDLyricsOutPlugin()
{
	m_szMediaInfo[0] = 0;
	m_nMediaLength = 0;

	m_mediaInfo = NULL;
	m_lyricsLine = NULL;
	m_playingProgress = NULL;
	m_mediaInfoColor = NULL;
	m_miniLyricsColor = NULL;
	m_lyricsLineColor = NULL;
	m_playingProgressColor = NULL;
}

CG15LCDLyricsOutPlugin::~CG15LCDLyricsOutPlugin()
{
}

void CG15LCDLyricsOutPlugin::Config(HWND hWndParent)
{
	MessageBox(hWndParent, L"MiniLyris for Logitech G15 LCD plug-in.", L"MiniLyrics", MB_OK);
}

void CG15LCDLyricsOutPlugin::About(HWND hWndParent)
{
	MessageBox(hWndParent, L"MiniLyris for Logitech G15 LCD plug-in.", L"MiniLyrics", MB_OK);
}

LPCTSTR CG15LCDLyricsOutPlugin::GetDescription()
{
	return L"MiniLyris for Logitech G15 LCD plug-in.";
}

bool CG15LCDLyricsOutPlugin::OnInit(unsigned int *uNotifyFlag)
{
	HRESULT hRes = m_lcd.Initialize(_T("MiniLyrics lyrics display for Logitech LCD"), LG_DUAL_MODE, FALSE, TRUE);
	if (hRes != S_OK)
		return false;

	*uNotifyFlag = NOTIF_CUR_LINE;

	g_wndG15Timer.Create(g_hInst, _T("G15 TimerWnd"));

	InitLCDObjectsMonochrome();
	InitLCDObjectsColor();

	return true;
}

void CG15LCDLyricsOutPlugin::OnCurLineChanged(int nLine, LPCTSTR szLyrics, int nBegTime, int nEndTime)
{
	m_lcd.SetText(m_lyricsLine, szLyrics);
	m_lcd.SetText(m_lyricsLineColor, szLyrics);

	m_lcd.Update();
}

void CG15LCDLyricsOutPlugin::OnHalfOfCurLine(int nLine)
{
}

void FormatPlayPos(int nPlayPos, LPTSTR szPos)
{
	int		nMin, nSec;

	nSec = (nPlayPos) % 60;
	nMin = nPlayPos / (60);

	wsprintf(szPos, _T("[%02d:%02d]"), nMin, nSec);
}

void CG15LCDLyricsOutPlugin::OnPlayPos(int uPos)
{
}

void CG15LCDLyricsOutPlugin::OnLyricsChanged()
{
}

void CG15LCDLyricsOutPlugin::OnSongChanged(LPCTSTR szArtist, LPCTSTR szTitle, int nDuration)
{
	m_nMediaLength = nDuration;

	_tcscpy(m_szMediaInfo, szArtist);
	_tcscat(m_szMediaInfo, _T(" - "));
	_tcscat(m_szMediaInfo, szTitle);

	m_lcd.SetText(m_mediaInfo, m_szMediaInfo);
	m_lcd.SetText(m_lyricsLine, _T(""));

	m_lcd.SetText(m_mediaInfoColor, m_szMediaInfo);
	m_lcd.SetText(m_lyricsLineColor, _T(""));

	m_lcd.Update();
}

void CG15LCDLyricsOutPlugin::OnQuit()
{
	g_wndG15Timer.Destroy();
	m_lcd.ShutDown();
}

void CG15LCDLyricsOutPlugin::InitLCDObjectsMonochrome()
{
	m_lcd.ModifyDisplay(LG_MONOCHROME);

	int y = 0;
	m_mediaInfo = m_lcd.AddText(LG_SCROLLING_TEXT, LG_TINY, DT_CENTER, LGLCD_BW_BMP_WIDTH);
	m_lcd.SetOrigin(m_mediaInfo, 0, 2);
	m_lcd.SetText(m_mediaInfo, _T("MiniLyrics"));
	y += 12;

	m_lyricsLine = m_lcd.AddText(LG_STATIC_TEXT, LG_MEDIUM, DT_CENTER, LGLCD_BW_BMP_WIDTH, 2);
	m_lcd.SetOrigin(m_lyricsLine, 0, y);
	m_lcd.SetText(m_lyricsLine, _T("MiniLyrics lyrics display for Logitech LCD."));

	m_playingProgress = m_lcd.AddProgressBar(LG_FILLED);
	m_lcd.SetProgressBarSize(m_playingProgress, LGLCD_BW_BMP_WIDTH - 2, 2);
	m_lcd.SetOrigin(m_playingProgress, 1, LGLCD_BW_BMP_HEIGHT - 2);
}

void CG15LCDLyricsOutPlugin::InitLCDObjectsColor()
{
	m_lcd.ModifyDisplay(LG_COLOR);

	/***************/
	/* FIRST PAGE */
	/***************/
	// m_background.LoadFromResource(NULL, AfxGetInstanceHandle(), IDR_BACKGROUND, _T("PNG"));
	// HBITMAP bmpBkg_ = m_background.GetHBITMAP();
	// m_lcd.SetBackground(bmpBkg_);
	m_lcd.SetBackground(RGB(0, 0, 0));

	m_miniLyricsColor = m_lcd.AddText(LG_STATIC_TEXT, LG_TINY, DT_LEFT, LGLCD_QVGA_BMP_WIDTH - 20);
	m_lcd.SetOrigin(m_miniLyricsColor, 10, 10);
	m_lcd.SetText(m_miniLyricsColor, SZ_PRODUCT_NAME);
	m_lcd.SetTextFontColor(m_miniLyricsColor, RGB(192, 192, 192));

	m_mediaInfoColor = m_lcd.AddText(LG_STATIC_TEXT, LG_TINY, DT_LEFT, LGLCD_QVGA_BMP_WIDTH - 20);
	m_lcd.SetOrigin(m_mediaInfoColor, 10, 50);
	m_lcd.SetText(m_mediaInfoColor, _T(""));
	m_lcd.SetTextFontColor(m_mediaInfoColor, RGB(255, 255, 255));

	m_lyricsLineColor = m_lcd.AddText(LG_STATIC_TEXT, LG_MEDIUM, DT_LEFT, LGLCD_QVGA_BMP_WIDTH - 20, 2);
	m_lcd.SetOrigin(m_lyricsLineColor, 10, 110);
	m_lcd.SetText(m_lyricsLineColor, _T("MiniLyrics lyrics display for Logitech LCD."));
	m_lcd.SetTextFontColor(m_lyricsLineColor, RGB(255, 255, 255));

	m_playingProgressColor = m_lcd.AddProgressBar(LG_FILLED);
	m_lcd.SetProgressBarSize(m_playingProgressColor, LGLCD_QVGA_BMP_WIDTH - 20, 10);
	m_lcd.SetOrigin(m_playingProgressColor, 10, LGLCD_QVGA_BMP_HEIGHT - 10);
}

void CG15LCDLyricsOutPlugin::OnTimer(int nID)
{
	switch (nID) {
	case CG15TimerWnd::TIMER_SHOW_LOGO:
		{
			static int n = 0;
			n++;
			if (n > 10)
				n = 0;
			if (n == 10)
				m_lcd.SetText(m_mediaInfo, SZ_PRODUCT_NAME);
			else if (n == 0)
				m_lcd.SetText(m_mediaInfo, m_szMediaInfo);
			else
				break;

			m_lcd.Update();
		}
		break;
	case CG15TimerWnd::TIMER_POS_UPDATE:
		{
			/*int		nPosNow = (g_pMLyrOutHost->GetPlayPos() + 500) / 1000;
			if (nPosNow != m_nPlayPosLast)
			{
				m_nPlayPosLast = nPosNow;

				TCHAR		szPos[64];
				FormatPlayPos(m_nPlayPosLast, szPos);
				m_lcd.SetText(m_miniLyricsColor, szPos);
			}*/

			int		nProgressPercent = (g_pMLyrOutHost->GetPlayPos() * 100 / (float)m_nMediaLength);

			m_lcd.SetProgressBarPosition(m_playingProgress, nProgressPercent);

			m_lcd.SetProgressBarPosition(m_playingProgressColor, nProgressPercent);

			m_lcd.Update();
		}
		break;

/*	case CG15TimerWnd::TIMER_CHECK_BTN:
		{
			//
			//
			//
			m_lcd.ModifyDisplay(LG_MONOCHROME);

			if (m_lcd.ButtonTriggered(LG_BUTTON_1))
			{
				m_lcd.SetText(m_mediaInfo, _T("PlayPause"));
			}
			else if (m_lcd.ButtonTriggered(LG_BUTTON_2))
			{
				m_lcd.SetText(m_mediaInfo, _T("Prev"));
			}
			else if (m_lcd.ButtonTriggered(LG_BUTTON_3))
			{
				m_lcd.SetText(m_mediaInfo, _T("Next"));
			}
		}
		break;*/
	}
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
