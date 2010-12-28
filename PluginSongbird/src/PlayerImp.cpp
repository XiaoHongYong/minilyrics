#include "MLPlugin.h"
#include "PlayerImp.h"
#include <string>
using namespace std;

CPlayerImp		g_Player;

#include "sbMiniLyricsService.h"
#include <sbILibrary.h>
#include <sbIMediacoreEvent.h>
#include <sbIMediacoreEventTarget.h>
#include <sbIMediacoreManager.h>
#include <sbIMediacorePlaybackControl.h>
#include <sbIMediacoreSequencer.h>

#include <sbIMediacoreManager.h>
#include <sbIMediacorePlaybackControl.h>

// Songbird imports.
#include <sbILibrary.h>
#include <sbILibraryManager.h>
#include <sbIPropertyArray.h>
#include <sbStandardProperties.h>
#include <sbVariantUtils.h>

#define SB_PREFS_ROOT                     NS_LITERAL_STRING("songbird.")


// the exported function that XMPlay calls to get the plugin interface
extern "C"
{
	BOOL WINAPI DllMain(HANDLE hDLL, DWORD reason, LPVOID reserved)
	{
		switch (reason) {
		case DLL_PROCESS_ATTACH:
			g_hInstance = (HINSTANCE)hDLL;
			break;

		case DLL_PROCESS_DETACH:
			break;
		}
		return TRUE;
	}

}

inline int _abs(int number)
{
	return( number>=0 ? number : -number );
}

class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
		SetTimer(m_hWnd, TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
		KillTimer(m_hWnd, TIMER_CHECK_MEDIA);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
		{
			DWORD		nPos;
			DWORD		dwTimeNew;

			static		bool bPauseCheck = false;
			static		DWORD nPosOld = 0;

			dwTimeNew = GetTickCount();
			nPos = dwTimeNew - m_dwTimeBeg;

			if (bPauseCheck)
			{
				if (m_nPosQuery != nPosOld)
				{
					bPauseCheck = false;
					m_dwTimeBeg = dwTimeNew - m_nPosQuery;
					g_MLService.OnPlayTimeChanged(m_nPosQuery);
				}
				return;
			}

			if (_abs(nPos - m_nPosQuery) > 1000)
			{
				// use Seek or pause? let's adjust it.
				m_dwTimeBeg = dwTimeNew - m_nPosQuery;
				g_MLService.OnPlayTimeChanged(m_nPosQuery);
				nPosOld = m_nPosQuery;
				bPauseCheck = true;
			}
			else
				g_MLService.OnPlayTimeChanged(nPos);
		}
		else if (nTimerID == TIMER_CHECK_MEDIA)
		{
			g_Player.DoCheckMediaChange();
			m_nPosQuery = g_Player.GetPlayPos();
		}
	}

	DWORD		m_dwTimeBeg, m_nPosQuery;

};

CPlayerInfoQueryWnd	g_wndInfoQuery;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Songbird");

	return 1;
}

DWORD CPlayerImp::GetPlayerState()
{
	if (!IsSBReady())
		return PS_STOPED;

	if (!mdrPlaying) {
		nsresult rv;
		mdrPlaying = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, PS_STOPED );
		rv = mdrPlaying->Init( NS_LITERAL_STRING("faceplate.playing"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, PS_STOPED );
	}
	PRBool bPlaying;
	if (NS_SUCCEEDED(mdrPlaying->GetBoolValue(&bPlaying)) && bPlaying)
		return PS_PLAYING;

	if (!mdrPaused)
	{
		nsresult rv;
		mdrPaused = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, PS_STOPED);
		rv = mdrPaused->Init( NS_LITERAL_STRING("faceplate.paused"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, PS_STOPED);
	}
	PRBool bPaused;
	if (NS_SUCCEEDED(mdrPaused->GetBoolValue(&bPaused)) && bPaused)
		return PS_PAUSED;

	return PS_STOPED;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

DWORD CPlayerImp::GetPlayPos()
{
	if (!IsSBReady())
		return 0;

	nsresult rv;

	if (!mdrPosition) {
		nsresult rv;
		mdrPosition = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, 0);
		rv = mdrPosition->Init( NS_LITERAL_STRING("metadata.position"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, 0);
	}
	PRInt64	nPosition;
	if (NS_SUCCEEDED(mdrPosition->GetIntValue(&nPosition)))
		return (DWORD)nPosition;

	return 0;
}

BOOL CPlayerImp::Play()
{
	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	if (GetPlayerState() == PS_PAUSED)
	{
		nsCOMPtr<sbIMediacoreManager> manager = 
			do_QueryReferent(mMM, &rv);
		NS_ENSURE_SUCCESS(rv, rv);

		nsCOMPtr<sbIMediacorePlaybackControl> playbackControl;
		rv = manager->GetPlaybackControl(getter_AddRefs(playbackControl));
		NS_ENSURE_SUCCESS(rv, rv);

		if ( playbackControl ) {
			rv = playbackControl->Play();
			NS_ENSURE_SUCCESS(rv, rv);
		}
	}
	else
	{
		nsCOMPtr<sbIMediacoreManager> manager = 
			do_QueryReferent(mMM, &rv);
		NS_ENSURE_SUCCESS(rv, rv);

		nsCOMPtr<sbIMediacoreSequencer> sequencer;
		rv = manager->GetSequencer(getter_AddRefs(sequencer));
		NS_ENSURE_SUCCESS(rv, rv);

		rv = sequencer->Play();
		NS_ENSURE_SUCCESS( rv, rv );
	}

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	nsCOMPtr<sbIMediacoreManager> manager = 
		do_QueryReferent(mMM, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<sbIMediacorePlaybackControl> playbackControl;
	rv = manager->GetPlaybackControl(getter_AddRefs(playbackControl));
	NS_ENSURE_SUCCESS(rv, rv);

	if ( playbackControl ) {
		rv = playbackControl->Pause();
		NS_ENSURE_SUCCESS(rv, rv);
	}

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (!IsSBReady())
		return FALSE;

	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	nsCOMPtr<sbIMediacoreManager> manager = 
		do_QueryReferent(mMM, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<sbIMediacorePlaybackControl> playbackControl;
	rv = manager->GetPlaybackControl(getter_AddRefs(playbackControl));
	NS_ENSURE_SUCCESS(rv, rv);

	if ( playbackControl ) {
		rv = playbackControl->Stop();
		NS_ENSURE_SUCCESS(rv, rv);
	}

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
/*	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	nsCOMPtr<sbIMediacoreManager> manager = 
		do_QueryReferent(mMM, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<sbIMediacoreSequencer> sequencer;
	rv = manager->GetSequencer(getter_AddRefs(sequencer));
	NS_ENSURE_SUCCESS(rv, rv);

	rv = sequencer->Previous();
	NS_ENSURE_SUCCESS( rv, rv );
*/
	return TRUE;
}

BOOL CPlayerImp::Next()
{
/*	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	nsCOMPtr<sbIMediacoreManager> manager = 
		do_QueryReferent(mMM, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<sbIMediacoreSequencer> sequencer;
	rv = manager->GetSequencer(getter_AddRefs(sequencer));
	NS_ENSURE_SUCCESS(rv, rv);

	rv = sequencer->Next();
	NS_ENSURE_SUCCESS( rv, rv );
*/
	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
/*	if (!IsSBReady())
		return FALSE;

	nsresult rv;

	if (!mdrPosition) {
		nsresult rv;
		mdrPosition = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrPosition->Init( NS_LITERAL_STRING("metadata.position"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	PRInt64	nPosition = nMsPos;
	rv = mdrPosition->SetIntValue(nPosition);
	NS_ENSURE_SUCCESS( rv, rv );
*/
	return FALSE;
}

BOOL CPlayerImp::Init()
{
	nsresult rv;

	nsCOMPtr<nsISupportsWeakReference> weakRef = 
		do_GetService(SB_MEDIACOREMANAGER_CONTRACTID, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	rv = weakRef->GetWeakReference(getter_AddRefs(mMM));
	NS_ENSURE_SUCCESS(rv, rv);

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();

	mMM = NULL;
	mdrCurrentArtist = NULL;
	mdrCurrentAlbum = NULL;
	mdrCurrentTrack = NULL;
	mdrCurrentUrl = NULL;
	mdrPlaying = NULL;
	mdrPaused = NULL;
	mdrPosition = NULL;
	mdrCurrentLength = NULL;
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}

BOOL CPlayerImp::DoCheckMediaChange()
{
	if (!IsSBReady())
		return FALSE;

	nsString		nsArtist, nsAlbum, nsTitle, nsUrl;
	MLMediaInfo		mediaInfo;

	static TCHAR			szMediaPathOld[MAX_PATH] = L"";

	// Url
	if (!mdrCurrentUrl) {
		nsresult rv;
		mdrCurrentUrl = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentUrl->Init( NS_LITERAL_STRING("metadata.url"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentUrl->GetStringValue(nsUrl)))
		mediaInfo.szMediaUrl = nsUrl.get();

	if (!mediaInfo.szMediaUrl)
	{
		if (!IsEmptyString(szMediaPathOld))
		{
			g_MLService.OnMediaInfoChanged(NULL);
			EmptyStr(szMediaPathOld);
		}
		return TRUE;
	}

	// Media not changed.
	if (lstrcmp(szMediaPathOld, mediaInfo.szMediaUrl) == 0)
		return TRUE;
	_tcscpy_safe(szMediaPathOld, CountOf(szMediaPathOld), mediaInfo.szMediaUrl);

	// Artist
	if (!mdrCurrentArtist) {
		nsresult rv;
		mdrCurrentArtist = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentArtist->Init( NS_LITERAL_STRING("metadata.artist"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentArtist->GetStringValue(nsArtist)))
		mediaInfo.szArtist = nsArtist.get();

	// Album
	if (!mdrCurrentAlbum) {
		nsresult rv;
		mdrCurrentAlbum = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentAlbum->Init( NS_LITERAL_STRING("metadata.album"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentAlbum->GetStringValue(nsAlbum)))
		mediaInfo.szAlbum = nsAlbum.get();

	// Title
	if (!mdrCurrentTrack) {
		nsresult rv;
		mdrCurrentTrack = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentTrack->Init( NS_LITERAL_STRING("metadata.title"),
			SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentTrack->GetStringValue(nsTitle)))
		mediaInfo.szTitle = nsTitle.get();

	// Length
	if (!mdrCurrentLength) {
		nsresult rv;
		mdrCurrentLength = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentLength->Init( NS_LITERAL_STRING("metadata.length"),
			SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}

	PRInt64		nLength;
	if (NS_SUCCEEDED(mdrCurrentLength->GetIntValue(&nLength)))
		mediaInfo.nMediaLength = (unsigned long)nLength;

	g_MLService.OnMediaInfoChanged(&mediaInfo);

	return TRUE;
}
