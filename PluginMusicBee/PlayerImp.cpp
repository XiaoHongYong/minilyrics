#include "../MLPluginLib/MLPlugin.h"
#include "../MPShared/MLVersion.h"
#include "PlayerImp.h"
#include "MusicBeeInterface_1_0.h"

CPlayerImp		g_Player;
PLAYER_STATE	g_playerState = PS_STOPED;

using namespace MusicBeePlugin;

//////////////////////////////////////////////////////////////////////////

HANDLE		g_hThread = NULL;
DWORD		g_dwThreadID = 0;

DWORD WINAPI ThreadML(LPVOID lpParam)
{
	RunMiniLyrics(g_hInstance, NULL, &g_Player);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DisconnectMiniLyrics(&g_Player);

	return 0;
}

// DLL needs to be named MB_xxxxxxxx.dll and placed in the Plugins folder where MusicBee is installed
// the plugin will automatically be added to the MusicBee Plugin preferences panel and needs to be explicitly enabled in that panel

MusicBeeApiInterface mbApiInterface;
PluginInfo about;

PLAYER_STATE GetPlayerState()
{
	switch (mbApiInterface.Player_GetPlayState())
	{
	case Playing: return PS_PLAYING;
	case Paused: return PS_PAUSED;
	default: return PS_STOPED;
	}
}

PluginInfo* Initialise(MusicBeeApiInterface* apiInterface)
{
	mbApiInterface = *apiInterface;
	about.PluginInfoVersion = PluginInfoVersion;
	about.Name = L"MiniLyrics for MusicBee";
	about.Description = L"Display lyrics for your favorite music.";
	about.Author = L"Crintsoft";
	about.TargetApplication = L"lyrics";  // current only applies to artwork, lyrics or instant messenger name that appears in the provider drop down selector or target Instant Messenger
	about.Type = General;
	about.VersionMajor = ML_MAJOR_VERSION;  // your plugin version
	about.VersionMinor = ML_MINOR_VERSION;
	about.Revision = AUTO_BUILD_NUM;
	about.MinInterfaceVersion = MinInterfaceVersion;
	about.MinApiRevision = MinApiRevision;
	about.ReceiveNotifications = PlayerEvents;
	about.ConfigurationPanelHeight = 0;  // height in pixels that musicbee should reserve in a panel for config settings. When set, a handle to an empty panel will be passed to the Configure function
	return &about;
}

// configuration dialog
// if this function is removed or false is returned, MusicBee will display an About dialog when the user clicks Configure
int Configure(void *panelHandle)
{
	// save any persistent settings in a sub-folder of this path
	LPCWSTR dataPath = mbApiInterface.Setting_GetPersistentStoragePath();
	// panelHandle will only be set if you set about.ConfigurationPanelHeight to a non-zero value
	// keep in mind the panel width is scaled according to the font the user has selected
	// Panel configPanel = (Panel)Panel.FromHandle(panelHandle);
	// if about.ConfigurationPanelHeight is set to 0, you can display your own popup window
	//...
	mbApiInterface.MB_ReleaseString(dataPath);
	return (int)false;
}


// called by MusicBee when the user clicks Apply or Save in the MusicBee Preferences screen.
// its up to you to figure out whether anything has changed and needs updating
void SaveSettings()
{
	// save any persistent settings in a sub-folder of this path
	LPCWSTR dataPath = mbApiInterface.Setting_GetPersistentStoragePath();
}


// MusicBee is closing the plugin (plugin is being disabled by user or MusicBee is shutting down)
void Close(PluginCloseReason reason)
{
	PostThreadMessage(g_dwThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject(g_hThread, INFINITE);
	CloseHandle(g_hThread);
	g_hThread = NULL;
}

// uninstall this plugin - clean up any persisted files
void Uninstall()
{
}

// recieve event notifications from MusicBee
// you need to set about.ReceiveNotificationFlags = PlayerEvents to receive all notifications, and not just the startup event
void ReceiveNotification(LPCWSTR sourceFileUrl, NotificationType type)
{
	// perform some action depending on the notification type
	switch (type)
	{
	case PluginStartup:
		{
			// Create a thread to pump the messages...
			// ASSERT(g_hThread == NULL);
			g_hThread = CreateThread(NULL, 0, ThreadML, NULL, 0, &g_dwThreadID);
		}
		break;
	case TrackChanged:
		g_Player.OnMediaChanged();
		break;
	case PlayStateChanged:
		g_Player.OnPlayerStateChanged();
		break;
	}
}

// return an array of lyric or artwork provider names this plugin supports
// the providers will be iterated through one by one and passed to the RetrieveLyrics/ RetrieveArtwork function in order set by the user in the MusicBee Tags(2) preferences screen until a match is found
// the last item on the array needs to be NULL
LPCWSTR* GetProviders()
{
	return NULL;
}

// return lyrics for the requested artist/title from the requested provider
// only required if PluginType = LyricsRetrieval
// dispose the returned string the next time this API is called
// return NULL if no lyrics are found
LPCWSTR RetrieveLyrics(LPCWSTR sourceFileUrl, LPCWSTR artist, LPCWSTR trackTitle, LPCWSTR album, bool synchronisedPreferred, LPCWSTR provider)
{
	return NULL;
}

// return Base64 string representation of the artwork binary data from the requested provider
// only required if PluginType = ArtworkRetrieval
// dispose the returned string the next time this API is called
// return NULL if no artwork is found
LPCWSTR RetrieveArtwork(LPCWSTR sourceFileUrl, LPCWSTR albumArtist, LPCWSTR album, LPCWSTR provider)
{
	// see http://www.adp-gmbh.ch/cpp/common/base64.html has an example of a base64 encoder
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////


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

//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_dwBegTime = 0;
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = NULL;
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"MusicBee");
}

void CPlayerImp::OnConnected()
{
	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, 1000);

	// Update Media info
	OnMediaChanged();

	// Update player state
	OnPlayerStateChanged();
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
}

void CPlayerImp::Play()
{
	if (GetPlayerState() != PS_PLAYING)
		mbApiInterface.Player_PlayPause();
}

void CPlayerImp::Pause()
{
	if (GetPlayerState() == PS_PAUSED)
		mbApiInterface.Player_PlayPause();
}

void CPlayerImp::PlayPause()
{
	mbApiInterface.Player_PlayPause();
}

void CPlayerImp::Stop()
{
	if (GetPlayerState() != PS_STOPED)
		mbApiInterface.Player_Stop();
}

void CPlayerImp::Prev()
{
	mbApiInterface.Player_PlayPreviousTrack();
}

void CPlayerImp::Next()
{
	mbApiInterface.Player_PlayNextTrack();
}

void CPlayerImp::SeekTo(int nMsPos)
{
	mbApiInterface.Player_SetPosition(nMsPos);
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (!g_MLService.IsValid())
		return;

	if (nTimerID == TIMER_CHECK_MEDIA)
	{
		int posGood = mbApiInterface.Player_GetPosition();
		DWORD timeNow = GetTickCount();

		int pos = timeNow - m_dwBegTime;
		if (_abs(pos - posGood) >= 1000)
		{
			m_dwBegTime = timeNow - posGood;
			g_MLService.OnPlayPosChanged(posGood);
		}
	}
	else
	{
		if (g_playerState != PS_PLAYING)
			return;

		ASSERT(nTimerID == TIMER_GETPOS);
		g_MLService.OnPlayPosChanged(GetTickCount() - m_dwBegTime);
	}
}

void CPlayerImp::OnMediaChanged()
{
	LPCWSTR artist = mbApiInterface.NowPlaying_GetFileTag(Artist);
	LPCWSTR album = mbApiInterface.NowPlaying_GetFileTag(Album);
	LPCWSTR title = mbApiInterface.NowPlaying_GetFileTag(TrackTitle);
	LPCWSTR url = mbApiInterface.NowPlaying_GetFileUrl();

	int duration = mbApiInterface.NowPlaying_GetDuration();
	g_MLService.OnMediaInfoChanged(url, artist, album, title, duration);

	mbApiInterface.MB_ReleaseString(artist);
	mbApiInterface.MB_ReleaseString(album);
	mbApiInterface.MB_ReleaseString(title);
	mbApiInterface.MB_ReleaseString(url);
}

void CPlayerImp::OnPlayerStateChanged()
{
	g_playerState = GetPlayerState();
	g_MLService.OnPlayStateChanged(g_playerState);

	int posGood = mbApiInterface.Player_GetPosition();
	DWORD timeNow = GetTickCount();
	m_dwBegTime = timeNow - posGood;
	g_MLService.OnPlayPosChanged(posGood);
}
