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

void InitSB()
{
	RunMiniLyrics(g_hInstance, NULL, &g_Player);
}

//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Songbird");
}

void CPlayerImp::OnConnected()
{
	nsresult rv;

	nsCOMPtr<nsISupportsWeakReference> weakRef = 
		do_GetService(SB_MEDIACOREMANAGER_CONTRACTID, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	rv = weakRef->GetWeakReference(getter_AddRefs(mMM));
	NS_ENSURE_SUCCESS(rv, rv);

	g_MLService.AddWndCloseto(GetPlayerWindow(), NULL, NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	mMM = NULL;
	mdrCurrentArtist = NULL;
	mdrCurrentAlbum = NULL;
	mdrCurrentTrack = NULL;
	mdrCurrentUrl = NULL;
	mdrPlaying = NULL;
	mdrPaused = NULL;
	mdrPosition = NULL;
	mdrCurrentLength = NULL;

	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerImp::Play()
{
	if (!IsSBReady())
		return;

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
}

void CPlayerImp::Pause()
{
	if (!IsSBReady())
		return;

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
}

void CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();
}

void CPlayerImp::Stop()
{
	if (!IsSBReady())
		return;

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
}

void CPlayerImp::Prev()
{
/*	if (!IsSBReady())
		return;

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
}

void CPlayerImp::Next()
{
/*	if (!IsSBReady())
		return;

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
}

void CPlayerImp::SeekTo(int nMsPos)
{
/*	if (!IsSBReady())
		return;

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
}

void CPlayerImp::OnTimer(int nTimerID)
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
				g_MLService.OnPlayPosChanged(m_nPosQuery);
			}
			return;
		}

		if (_abs(nPos - m_nPosQuery) > 1000)
		{
			// use Seek or pause? let's adjust it.
			m_dwTimeBeg = dwTimeNew - m_nPosQuery;
			g_MLService.OnPlayPosChanged(m_nPosQuery);
			nPosOld = m_nPosQuery;
			bPauseCheck = true;
		}
		else
			g_MLService.OnPlayPosChanged(nPos);
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		DoCheckMediaChange();
		m_nPosQuery = GetPlayPos();
	}
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

BOOL CPlayerImp::DoCheckMediaChange()
{
	if (!IsSBReady())
		return FALSE;

	nsString		nsArtist, nsAlbum, nsTitle, nsUrl;
	LPCWSTR			szMediaUrl = NULL, szArtist = NULL, szTitle = NULL, szAlbum = NULL;
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
		szMediaUrl = nsUrl.get();

	if (!szMediaUrl)
	{
		if (!IsEmptyString(szMediaPathOld))
		{
			g_MLService.OnMediaInfoChanged(NULL);
			EmptyStr(szMediaPathOld);
		}
		return TRUE;
	}

	// Media not changed.
	if (lstrcmp(szMediaPathOld, szMediaUrl) == 0)
		return TRUE;
	_tcscpy_safe(szMediaPathOld, CountOf(szMediaPathOld), szMediaUrl);

	// Artist
	if (!mdrCurrentArtist) {
		nsresult rv;
		mdrCurrentArtist = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentArtist->Init( NS_LITERAL_STRING("metadata.artist"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentArtist->GetStringValue(nsArtist)))
		szArtist = nsArtist.get();

	// Album
	if (!mdrCurrentAlbum) {
		nsresult rv;
		mdrCurrentAlbum = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentAlbum->Init( NS_LITERAL_STRING("metadata.album"), SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}
	if (NS_SUCCEEDED(mdrCurrentAlbum->GetStringValue(nsAlbum)))
		szAlbum = nsAlbum.get();

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
		szTitle = nsTitle.get();

	// Length
	if (!mdrCurrentLength) {
		nsresult rv;
		mdrCurrentLength = do_CreateInstance( "@songbirdnest.com/Songbird/DataRemote;1", &rv );
		NS_ENSURE_SUCCESS( rv, rv );
		rv = mdrCurrentLength->Init( NS_LITERAL_STRING("metadata.length"),
			SB_PREFS_ROOT );
		NS_ENSURE_SUCCESS( rv, rv );
	}

	PRInt64		nLength = 0;
	mdrCurrentLength->GetIntValue(&nLength);

	g_MLService.OnMediaInfoChanged(szMediaUrl, szArtist, szAlbum, szTitle, nLength);

	return TRUE;
}
