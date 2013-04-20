#include <sys/time.h>
#include "../base/base.h"
#include "PlayerImp.h"
#import "EyeTunes.h"
#include <cmath>
#include <pthread.h>

CPlayerImp	g_Player;
pthread_t	g_threadTimer = 0;
volatile bool g_bQuitTimer = false;
volatile bool g_bMediaChanged = false;

static char	fileNameLast[256 + 128] = { 0 };

#define TIMER_DURATION_GET_POS		100000

LPVOID ThreadTimer(LPVOID param)
{
	g_Player.UpdateTrackInfo();

	while (!g_bQuitTimer)
	{
		if (usleep(TIMER_DURATION_GET_POS) != 0)
			Log("Failed to sleep");
		if (g_bMediaChanged)
		{
			g_Player.UpdateTrackInfo();
			g_bMediaChanged = false;
		}
		g_Player.OnTimer(CPlayerServiceBase::TIMER_GETPOS);
	}

	return NULL;
}

char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource)
{
    char	*cp = strDestination;
	char	*cpEnd = strDestination + nLenMax - 1;
	
	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}
	
	*cp = '\0';
	
    return(strDestination);
}


CPlayerImp::CPlayerImp()
{
	m_dwPos = 0;
	m_dwBegTickCount = 0;
	m_bWorking = false;
}

CPlayerImp::~CPlayerImp()
{
}


void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	strcpy(playerInfo.playerName, "iTunes");
}

void CPlayerImp::OnConnected()
{
	Log("OnConnected.");
	
	g_bMediaChanged = true;
	fileNameLast[0] = '\0';

	if (m_bWorking)
		return;

	m_bWorking = true;

	pthread_create(&g_threadTimer, NULL, ThreadTimer, NULL);
	g_bQuitTimer = false;
}

void CPlayerImp::OnDisconnect()
{
	Log("OnDisconnect.");

	m_bWorking = false;

	if (g_threadTimer != 0)
	{
		g_bQuitTimer = true;
		pthread_join(g_threadTimer, NULL);
		g_threadTimer = 0;
	}
}

void CPlayerImp::Play()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes play];
}

void CPlayerImp::Pause()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes pause];
}

void CPlayerImp::PlayPause()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes playPause];
}

void CPlayerImp::Stop()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes stop];

}

void CPlayerImp::Prev()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes previousTrack];
}

void CPlayerImp::Next()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes nextTrack];
}

void CPlayerImp::SeekTo(int nMsPos)
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes != nil)
		[iTunes setPlayerPosition:nMsPos / 1000];
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		if (m_playerState == PS_PLAYING)
		{
			m_dwPos = GetTickCount() - m_dwBegTickCount;
			// Log("Play position changed: %d", m_dwPos);
			g_MLService.OnPlayPosChanged(m_dwPos);
		}
	}
}

void CopyTrackInfoField(LPSTR szField, size_t sizeField, const UniChar *value)
{
	EmptyStr(szField);
	CFStringRef unValue = CFStringCreateWithCharacters(NULL, value + 1, value[0]);
	CFStringGetCString(unValue, szField, sizeField, kCFStringEncodingUTF8);
	CFRelease(unValue);
}

void CPlayerImp::OnMediaChanged(ITTrackInfo	*pTrackInfo)
{
	// Can't call [iTunes currentTrack] in this thread, it might cause dead lock.
	// So, set this flag, and call it in another thread.
	g_bMediaChanged = true;

	m_dwBegTickCount = GetTickCount();
	m_dwPos = 0;
}

void CPlayerImp::OnPlayStateChanged(PLAYER_STATE newStat)
{
	if (m_playerState == newStat)
		return;
	if (newStat == PS_PLAYING)
	{
/*		EyeTunes *iTunes = [EyeTunes sharedInstance];
		if (iTunes != nil)
		{
			if (std::abs((int)m_dwPos - [[EyeTunes sharedInstance] playerPosition] * 1000) >= 1000)
				m_dwPos = [[EyeTunes sharedInstance] playerPosition];
		}*/

		m_dwBegTickCount = GetTickCount() - m_dwPos;
	}

	m_playerState = newStat;
	g_MLService.OnPlayStateChanged(g_Player.m_playerState);
}

void CPlayerImp::UpdateTrackInfo()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes == nil)
		return;

	ETTrack *track = [iTunes currentTrack];
	if (track == nil)
		return;

	char szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];

	strcpy_safe(szMediaUrl, CountOf(szMediaUrl), [[track location] UTF8String]);

	// Is current track changed?
	if (strcmp(fileNameLast, szMediaUrl) == 0)
		return;
	strcpy(fileNameLast, szMediaUrl);

	strcpy_safe(szArtist, CountOf(szArtist), [[track artist] UTF8String]);
	strcpy_safe(szAlbum, CountOf(szAlbum), [[track album] UTF8String]);
	strcpy_safe(szTitle, CountOf(szTitle), [[track name] UTF8String]);

	g_MLService.OnMediaInfoChanged(szMediaUrl, szArtist, szAlbum, szTitle, [track duration] * 1000);

	DescType state = [iTunes playerState];
	if (state == kETPlayerStatePlaying)
		m_playerState = PS_PLAYING;
	else if (state == kETPlayerStatePaused)
		m_playerState = PS_PAUSED;
	else
		m_playerState = PS_STOPED;

	g_MLService.OnPlayStateChanged(m_playerState);
}
