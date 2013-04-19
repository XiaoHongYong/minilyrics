#include <sys/time.h>
#include "../base/base.h"
#include "PlayerImp.h"
#import "EyeTunes.h"
#include <cmath>
#include <pthread.h>

CPlayerImp	g_Player;
pthread_t	g_threadTimer = 0;
volatile bool g_bQuitTimer = false;

#define TIMER_DURATION_GET_POS		100000

LPVOID ThreadTimer(LPVOID param)
{
	//g_Player.InitUpdateTrackInfo();

	while (!g_bQuitTimer)
	{
		if (usleep(TIMER_DURATION_GET_POS) != 0)
			Log("Failed to sleep");
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
	
	if (m_bWorking)
		return;

	m_bWorking = true;

	pthread_create(&g_threadTimer, NULL, ThreadTimer, NULL);
	g_bQuitTimer = false;
}

void CPlayerImp::OnDisconnect()
{
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
	char		szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];
	static char			fileNameLast[256 + 128] = { 0 };
	static UInt64		sizeInBytesLast = 0;
	
	CopyTrackInfoField(szMediaUrl, CountOf(szMediaUrl), pTrackInfo->fileName);
	
	// Is current track changed?
	if (strcmp(fileNameLast, szMediaUrl) == 0 && sizeInBytesLast == pTrackInfo->sizeInBytes)
		return;
	strcpy(fileNameLast, szMediaUrl);
	sizeInBytesLast = pTrackInfo->sizeInBytes;

	m_dwBegTickCount = GetTickCount();
	m_dwPos = 0;

	CopyTrackInfoField(szArtist, CountOf(szArtist), pTrackInfo->artist);
	CopyTrackInfoField(szAlbum, CountOf(szAlbum), pTrackInfo->album);
	CopyTrackInfoField(szTitle, CountOf(szTitle), pTrackInfo->name);

	g_MLService.OnMediaInfoChanged(szMediaUrl, szArtist, szAlbum, szTitle, pTrackInfo->totalTimeInMS);

	m_playerState = PS_PLAYING;
	g_MLService.OnPlayStateChanged(m_playerState);
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

void CPlayerImp::InitUpdateTrackInfo()
{
	EyeTunes *iTunes = [EyeTunes sharedInstance];
	if (iTunes == nil)
		return;

	ETTrack *track = [iTunes currentTrack];

	m_dwPos = [iTunes playerPosition] * 1000;
	m_dwBegTickCount = GetTickCount() - m_dwPos;

	char szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];
	strcpy_safe(szArtist, CountOf(szArtist), [[track artist] UTF8String]);
	strcpy_safe(szAlbum, CountOf(szAlbum), [[track album] UTF8String]);
	strcpy_safe(szTitle, CountOf(szTitle), [[track name] UTF8String]);
	strcpy_safe(szMediaUrl, CountOf(szMediaUrl), [[track name] UTF8String]);

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
