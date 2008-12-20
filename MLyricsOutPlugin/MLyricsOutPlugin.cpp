// MLyricsOutPlugin.cpp: implementation of the MLyricsOutPlugin class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include "MLyricsOutPlugin.h"

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

CG15LCDLyricsOutPlugin::CG15LCDLyricsOutPlugin()
{

}

CG15LCDLyricsOutPlugin::~CG15LCDLyricsOutPlugin()
{

}

void CG15LCDLyricsOutPlugin::Config(HWND hWndParent)
{
	MessageBox(hWndParent, L"Configuration...", L"G15", MB_OK);
}

void CG15LCDLyricsOutPlugin::About(HWND hWndParent)
{
	MessageBox(hWndParent, L"About...", L"G15", MB_OK);
}

LPCTSTR CG15LCDLyricsOutPlugin::GetDescription()
{
	return L"Your plugin's description...";
}

bool CG15LCDLyricsOutPlugin::OnInit(unsigned int *uNotifyFlag)
{
	*uNotifyFlag = NOTIF_CUR_LINE | NOTIF_HALF_CUR_LINE;

	return true;
}

void CG15LCDLyricsOutPlugin::OnCurLineChanged(int nLine, LPCTSTR szLyrics, int nBegTime, int nEndTime)
{
	//
	MessageBox(NULL, szLyrics, _T("Current line"), MB_OK);
}

void CG15LCDLyricsOutPlugin::OnHalfOfCurLine(int nLine)
{
	TCHAR		szLyrics[512] = L"";
	int			nBegTime, nEndTime;
	bool		bRet;

	bRet = g_pMLyrOutHost->GetLyricsOfLine(nLine + 1, szLyrics, sizeof(szLyrics)/sizeof(TCHAR), nBegTime, nEndTime);
	if (bRet)
	{
		// show Next line?
		MessageBox(NULL, szLyrics, _T("Next line"), MB_OK);
	}
}

void CG15LCDLyricsOutPlugin::OnPlayPos(int uPos)
{

}

void CG15LCDLyricsOutPlugin::OnLyricsChanged()
{
}

void CG15LCDLyricsOutPlugin::OnSongChanged(LPCTSTR szArtist, LPCTSTR szTitle, int nDuration)
{
	
}

void CG15LCDLyricsOutPlugin::OnQuit()
{
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
// 
// extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
// {
// 	//	g_hInst = (HINSTANCE)hInst;
// 	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
// 	{
// 	}
// 
// 	return TRUE;
// }
