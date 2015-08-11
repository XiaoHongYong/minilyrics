#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "SpotifyPlayerService.h"

#include <wininet.h>
// 
// VOID CALLBACK 
// CallBack(
//     __in HINTERNET hInternet,
//     __in DWORD_PTR dwContext,
//     __in DWORD dwInternetStatus,
//     __in_bcount(dwStatusInformationLength) LPVOID lpvStatusInformation,
//     __in DWORD dwStatusInformationLength
//     )
// /*++
// 
// Routine Description:
//     Callback routine for asynchronous WinInet operations
// 
// Arguments:
//      hInternet - The handle for which the callback function is called.
//      dwContext - Pointer to the application defined context.
//      dwInternetStatus - Status code indicating why the callback is called.
//      lpvStatusInformation - Pointer to a buffer holding callback specific data.
//      dwStatusInformationLength - Specifies size of lpvStatusInformation buffer.
// 
// Return Value:
//     None.
// 
// --*/
// {
//     InternetCookieHistory cookieHistory;
//     PREQUEST_CONTEXT ReqContext = (PREQUEST_CONTEXT)dwContext;
//  
//     UNREFERENCED_PARAMETER(dwStatusInformationLength);
// 
//     fprintf(stderr, "Callback Received for Handle %p \t", hInternet);
//     
//     switch(dwInternetStatus)
//     {
//         case INTERNET_STATUS_COOKIE_SENT:
//             fprintf(stderr, "Status: Cookie found and will be sent with request\n");
//             break;
//             
//         case INTERNET_STATUS_COOKIE_RECEIVED:
//             fprintf(stderr, "Status: Cookie Received\n");
//             break;
//             
//         case INTERNET_STATUS_COOKIE_HISTORY:
// 
//             fprintf(stderr, "Status: Cookie History\n");
// 
//             ASYNC_ASSERT(lpvStatusInformation);
//             ASYNC_ASSERT(dwStatusInformationLength == sizeof(InternetCookieHistory));
// 
//             cookieHistory = *((InternetCookieHistory*)lpvStatusInformation);
//             
//             if(cookieHistory.fAccepted)
//             {
//                 fprintf(stderr, "Cookie Accepted\n");
//             }
//             if(cookieHistory.fLeashed)
//             {
//                 fprintf(stderr, "Cookie Leashed\n");
//             }        
//             if(cookieHistory.fDowngraded)
//             {
//                 fprintf(stderr, "Cookie Downgraded\n");
//             }        
//             if(cookieHistory.fRejected)
//             {
//                 fprintf(stderr, "Cookie Rejected\n");
//             }
// 
//  
//             break;
//             
//         case INTERNET_STATUS_CLOSING_CONNECTION:
//             fprintf(stderr, "Status: Closing Connection\n");
//             break;
//             
//         case INTERNET_STATUS_CONNECTED_TO_SERVER:
//             fprintf(stderr, "Status: Connected to Server\n");
//             break;
//             
//         case INTERNET_STATUS_CONNECTING_TO_SERVER:
//             fprintf(stderr, "Status: Connecting to Server\n");
//             break;
//             
//         case INTERNET_STATUS_CONNECTION_CLOSED:
//             fprintf(stderr, "Status: Connection Closed\n");
//             break;
//             
//         case INTERNET_STATUS_HANDLE_CLOSING:
//             fprintf(stderr, "Status: Handle Closing\n");
// 
//             //
//             // Signal the cleanup routine that it is
//             // safe to cleanup the request context
//             //
//             
//             ASYNC_ASSERT(ReqContext);
//             SetEvent(ReqContext->CleanUpEvent);
//             
//             break;
//             
//         case INTERNET_STATUS_HANDLE_CREATED:
//             ASYNC_ASSERT(lpvStatusInformation);
//             fprintf(stderr,
//                     "Handle %x created\n", 
//                     ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwResult);
// 
//             break;
//             
//         case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
//             fprintf(stderr, "Status: Intermediate response\n");
//             break;
//             
//         case INTERNET_STATUS_RECEIVING_RESPONSE:
//             fprintf(stderr, "Status: Receiving Response\n");    
//             break;
//             
//         case INTERNET_STATUS_RESPONSE_RECEIVED:
//             ASYNC_ASSERT(lpvStatusInformation);
//             ASYNC_ASSERT(dwStatusInformationLength == sizeof(DWORD));
//             
//             fprintf(stderr, "Status: Response Received (%d Bytes)\n", *((LPDWORD)lpvStatusInformation));
//             
//             break;
//             
//         case INTERNET_STATUS_REDIRECT:
//             fprintf(stderr, "Status: Redirect\n");
//             break;
// 
//         case INTERNET_STATUS_REQUEST_COMPLETE:
//             fprintf(stderr, "Status: Request complete\n");
// 
//             ASYNC_ASSERT(lpvStatusInformation);
//             
//             ProcessRequest(ReqContext, ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwError);
// 
//             break;
//             
//         case INTERNET_STATUS_REQUEST_SENT:
//             ASYNC_ASSERT(lpvStatusInformation);
//             ASYNC_ASSERT(dwStatusInformationLength == sizeof(DWORD));
// 
//             fprintf(stderr,"Status: Request sent (%d Bytes)\n", *((LPDWORD)lpvStatusInformation));
//             break;
//             
//         case INTERNET_STATUS_DETECTING_PROXY:
//             fprintf(stderr, "Status: Detecting Proxy\n");
//             break;            
//             
//         case INTERNET_STATUS_RESOLVING_NAME:
//             fprintf(stderr, "Status: Resolving Name\n");
//             break;
//             
//         case INTERNET_STATUS_NAME_RESOLVED:
//             fprintf(stderr, "Status: Name Resolved\n");
//             break;
//             
//         case INTERNET_STATUS_SENDING_REQUEST:
//             fprintf(stderr, "Status: Sending request\n");
//             break;
//             
//         case INTERNET_STATUS_STATE_CHANGE:
//             fprintf(stderr, "Status: State Change\n");
//             break;
//             
//         case INTERNET_STATUS_P3P_HEADER:
//             fprintf(stderr, "Status: Received P3P header\n");
//             break;
//             
//         default:
//             fprintf(stderr, "Status: Unknown (%d)\n", dwInternetStatus);
//             break;
//     }
// 
//     return;
// }
// 
// bool InetGet(LPCTSTR serverName, int port, LPCTSTR action, LPCTSTR uri, LPCTSTR referer, string &content) {
// 	LPCTSTR userAgent = _T("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.90 Safari/537.36");
// 	HINTERNET hInet = InternetOpen(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
// 	if (hInet == NULL) {
// 		return false;
// 	}
// 
// 	INTERNET_STATUS_CALLBACK hCallback = InternetSetStatusCallback(hInet, 
// 		(INTERNET_STATUS_CALLBACK)CallBack);
// 
// 	if (hCallback == INTERNET_INVALID_STATUS_CALLBACK)
// 	{
// 		fprintf(stderr, "InternetSetStatusCallback failed with INTERNET_INVALID_STATUS_CALLBACK\n");
// 		goto Exit;
// 	}
// 
// 	HINTERNET hConnect = InternetConnect(hInet, serverName, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
// 	if (hConnect == NULL) {
// 		return false;
// 	}
// 
// 	PCTSTR rgpszAcceptTypes[] = {_T("text/*"), NULL};
// 	HINTERNET hHttp = HttpOpenRequest(hConnect, action, uri, NULL, referer, NULL,
// 		INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS,
// 		NULL);
// 	if (hConnect == NULL) {
// 		return false;
// 	}
// 
// 	while (true)
// 	{
// 		const int BUF_LEN = 1024;
// 		char buf[BUF_LEN];
// 		DWORD size = 0;
// 		// Keep coping in 25 bytes chunks, while file has any data left.
// 		// Note: bigger buffer will greatly improve performance.
// 		if (!InternetReadFile (hHttp, buf, BUF_LEN,  &size))
// 			return false;
// 
// 		if (size)
// 			content.append(buf, size);
// 		else
// 			break;  // Condition of dwSize=0 indicate EOF. Stop.
// 	}
// 
// 	InternetCloseHandle(hInet);
// 	InternetCloseHandle(hConnect);
// 	InternetCloseHandle(hHttp);
// 
// exit:
// 
// 	return true;
// }

namespace SpotifyAgent
{

CPlayerService		g_Player;
HWND				g_hwndPlayer = NULL;

HWND GetPlayerWindow()
{
	return FindWindow(_T("SpotifyMainWindow"), NULL);
}

///////////////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
//	string content;
//	bool ret = InetGet(_T("localhost"), 4380, L"GET", L"/", L"", content);

//	ERRLOG1(L"CONTENT: %S", content.c_str());

	m_szWndTitleOld[0] = 0;
	m_bSpotifyTitle = true;
	m_bHasMedia = false;
	m_nLenTitleOld = 0;
}

CPlayerService::~CPlayerService()
{
}

void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	playerInfo.hWndPluginService = GetHandle();

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Spotify");
}

void CPlayerService::OnConnected()
{
	g_MLService.AddWndCloseto(NULL, L"SpotifyMainWindow", NULL);
	g_hwndPlayer = GetPlayerWindow();

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, 100);
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerService::Play()
{
	PlayPause();
}

void CPlayerService::Pause()
{
	PlayPause();
}

void CPlayerService::PlayPause()
{
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_SPACE, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_SPACE, 0);
}

void CPlayerService::Stop()
{
	PlayPause();
}

void CPlayerService::Prev()
{
	keybd_event(VK_CONTROL, 0x1D, 0, 0);
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_LEFT, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_LEFT, 0);
	Sleep(100);
	keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);

	m_dwBeginPlay = GetTickCount();
}

void CPlayerService::Next()
{
	keybd_event(VK_CONTROL, 0x1D, 0, 0);
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_RIGHT, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_RIGHT, 0);
	Sleep(100);
	keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);
}

void CPlayerService::SeekTo(int nMsPos)
{
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		if (m_bHasMedia && !m_bSpotifyTitle)
			g_MLService.OnPlayPosChanged(GetTickCount() - m_dwBeginPlay);
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		if (!IsWindow(g_hwndPlayer))
			g_hwndPlayer = FindWindow(_T("SpotifyMainWindow"), NULL);
		if (!g_hwndPlayer)
		{
			g_MLService.OnDisconnected();
			return;
		}

		WCHAR		szWndTitle[256];
		int			nLen, i;
		nLen = GetWindowTextW(g_hwndPlayer, szWndTitle, CountOf(szWndTitle));
		if (nLen <= 0)
		{
			OnNoMedia();
			return;
		}

		if (m_nLenTitleOld == nLen 
			&& lstrcmpW(m_szWndTitleOld, szWndTitle) == 0)
		{
			if (m_bSpotifyTitle)
			{
				m_bSpotifyTitle = false;
				m_dwBeginPlay += GetTickCount() - m_dwBeginPaused;
			}
			return;
		}

		if (lstrcmpW(szWndTitle, L"Spotify") == 0)
		{
			if (!m_bSpotifyTitle)
			{
				m_bSpotifyTitle = true;
				m_dwBeginPaused = GetTickCount();
			}
			return;
		}

		m_bSpotifyTitle = false;

		lstrcpyW(m_szWndTitleOld, szWndTitle);
		m_nLenTitleOld = nLen;

		const int	MAX_TAG = 10;
		LPCWSTR		vszTag[10] = { 0 };
		LPWSTR		szCur;
		LPCWSTR		SZ_SPLLITOR = L" - ";
		const int	LEN_SZ_SPLLITOR = wcslen(SZ_SPLLITOR);

		// Replace the separator '¨C' with '-'
		for (i = 0; i < nLen; i++)
		{
			if (szWndTitle[i] == 0x2013) // L'¨C'
				szWndTitle[i] = L'-';
		}

		// Split title.
		// Spotify - Britney Spears ¨C Overprotected - Album Version (Radio Edit)
		szCur = szWndTitle;
		for (i = 0; i < MAX_TAG; i++)
		{
			vszTag[i] = szCur;
			szCur = wcsstr(szCur, SZ_SPLLITOR);
			if (szCur)
			{
				szCur[0] = '\0';
				szCur += LEN_SZ_SPLLITOR;
			}
			else
				break;
		}

		LPCTSTR szTitle = vszTag[2];
		LPCTSTR szArtist = vszTag[1];
		LPCTSTR szAlbum = vszTag[3];

		if (szTitle == NULL || IsEmptyString(szTitle))
			m_bHasMedia = false;
		else
		{
			m_bHasMedia = true;
			m_dwBeginPlay = GetTickCount() + 2500;
		}

		g_MLService.OnMediaInfoChanged(NULL, szArtist, szAlbum, szTitle, 0);
	}
}

void CPlayerService::OnNoMedia()
{
	m_bHasMedia = false;
	g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
}

//////////////////////////////////////////////////////////////////////////

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = GetPlayerWindow();
	return hWndPlayer != NULL;
}

}
