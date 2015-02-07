
#include "iTunesPlugIn.h"
#include "../MLPluginLib/MlPlugin.h"
#include "PlayerImp.h"
#include <string.h>

HWND		g_hWndVisual = NULL;
ITAppProcPtr g_appProc = NULL;
void * g_appCookie = NULL;

void OnTrackChanged(ITTrackInfo *pTrackInfo)
{
	if (!pTrackInfo)
		return;

	if (g_Player.m_hWnd)
	{
		PostMessage(g_Player.m_hWnd, CPlayerImp::WM_SONG_CHANGED, (WPARAM)new ITTrackInfo(*pTrackInfo), 0);
	}
}

//-------------------------------------------------------------------------------------------------
//	VisualPluginHandler
//-------------------------------------------------------------------------------------------------
//
static OSStatus VisualPluginHandler(OSType message,VisualPluginMessageInfo *messageInfo,void *refCon)
{
	OSStatus			status;
	
	status = noErr;

	switch (message)
	{
		/*
			Sent when the visual plugin is registered.  The plugin should do minimal
			memory allocations here.
		*/		
		case kVisualPluginInitMessage:
			g_appCookie	= messageInfo->u.initMessage.appCookie;
			g_appProc	= messageInfo->u.initMessage.appProc;

			messageInfo->u.initMessage.refCon = NULL;
			break;
		/*
			Sent when the visual plugin is unloaded.
		*/
		case kVisualPluginCleanupMessage:
			break;

		/*
			Sent when the visual plugin is enabled/disabled.  iTunes currently enables all
			loaded visual plugins at launch.  The plugin should not do anything here.
		*/
		case kVisualPluginEnableMessage:
		case kVisualPluginDisableMessage:
			break;

		/*
			Sent if the plugin requests idle messages.  Do this by setting the kVisualWantsIdleMessages
			option in the RegisterVisualMessage.options field.
			
			DO NOT DRAW in this routine.  It is for updating internal state only.
		*/
		case kVisualPluginIdleMessage:
			break;

		/*
			Sent if the plugin requests the ability for the user to configure it.  Do this by setting
			the kVisualWantsConfigure option in the RegisterVisualMessage.options field.
		*/
		case kVisualPluginConfigureMessage:
			// status = ConfigureVisual( visualPluginData );
			break;

		/*
			Sent when iTunes is going to show the visual plugin.  At this
			point, the plugin should allocate any large buffers it needs.
		*/
		case kVisualPluginActivateMessage:
			g_hWndVisual = messageInfo->u.activateMessage.view;
			g_Player.OniTunesChangeVisualWnd(g_hWndVisual);
			DrawVisual(g_hWndVisual);
			break;

		/*
			Sent when this visual is no longer displayed.
		*/
		case kVisualPluginDeactivateMessage:
			g_hWndVisual = NULL;
			g_Player.OniTunesChangeVisualWnd(nil);
			break;
		
		/*
			Sent when iTunes is moving the destination view to a new parent window (e.g. to/from fullscreen).
		*/
		case kVisualPluginWindowChangedMessage:
			g_Player.OniTunesMoveVisualWnd();
			break;

		/*
			Sent when iTunes has changed the rectangle of the currently displayed visual.
			
			Note: for custom NSView subviews, the subview's frame is automatically resized.
		*/
		case kVisualPluginFrameChangedMessage:
			g_Player.OniTunesMoveVisualWnd();
			break;
		
		/*
			Sent for the visual plugin to update its internal animation state.
			Plugins are allowed to draw at this time but it is more efficient if they
			wait until the kVisualPluginDrawMessage is sent OR they simply invalidate
			their own subview.  The pulse message can be sent faster than the system
			will allow drawing to support spectral analysis-type plugins but drawing
			will be limited to the system refresh rate.
		*/
		case kVisualPluginPulseMessage:
			break;
		/*
			It's time for the plugin to draw a new frame.
			
			For plugins using custom subviews, you should ignore this message and just
			draw in your view's draw method.  It will never be called if your subview 
			is set up properly.
		*/
		case kVisualPluginDrawMessage:
			DrawVisual(g_hWndVisual);
			break;
		/*
			Sent when the player changes the current track information.  This
			is used when the information about a track changes.
		*/
		case kVisualPluginChangeTrackMessage:
			OnTrackChanged(messageInfo->u.changeTrackMessage.trackInfo);
			break;

		/*
			Sent when the player starts.
		*/
		case kVisualPluginPlayMessage:
			g_Player.m_playerState = PS_PLAYING;
			g_MLService.OnPlayStateChanged(g_Player.m_playerState);
			OnTrackChanged(messageInfo->u.playMessage.trackInfo);
			break;
		/*
			Sent when the player stops or pauses.
		*/
		case kVisualPluginStopMessage:
			g_Player.m_playerState = PS_STOPED;
			g_MLService.OnPlayStateChanged(g_Player.m_playerState);
			break;

		/*
			Sent when the player changes the playback position.
		*/
		case kVisualPluginSetPositionMessage:
			if (g_Player.m_playerState == PS_PLAYING)
				g_Player.m_dwBegTickCount = GetTickCount() - messageInfo->u.setPositionMessage.positionTimeInMS;
			else
			{
				g_Player.m_dwPos = messageInfo->u.setPositionMessage.positionTimeInMS;
				g_MLService.OnPlayPosChanged(g_Player.m_dwPos);
			}
			break;
		/*
			Artwork for the currently playing song is being delivered per a previous request.
			
			Note that NULL for messageInfo->u.coverArtMessage.coverArt means the currently playing song has no artwork.
		*/
		case kVisualPluginCoverArtMessage:
			break;

		default:
			status = unimpErr;
			break;
	}

	return status;	
}


/*
	RegisterVisualPlugin
*/
OSStatus RegisterVisualPlugin( PluginMessageInfo * messageInfo )
{
	PlayerMessageInfo	playerMessageInfo;
	OSStatus			status;
		
	memset( &playerMessageInfo.u.registerVisualPluginMessage, 0, sizeof(playerMessageInfo.u.registerVisualPluginMessage) );

	GetVisualName( playerMessageInfo.u.registerVisualPluginMessage.name );

	SetNumVersion( &playerMessageInfo.u.registerVisualPluginMessage.pluginVersion, kTVisualPluginMajorVersion, kTVisualPluginMinorVersion, kTVisualPluginReleaseStage, kTVisualPluginNonFinalRelease );

	playerMessageInfo.u.registerVisualPluginMessage.options					= 0;
	playerMessageInfo.u.registerVisualPluginMessage.handler					= (VisualPluginProcPtr)VisualPluginHandler;
	playerMessageInfo.u.registerVisualPluginMessage.registerRefCon			= 0;
	playerMessageInfo.u.registerVisualPluginMessage.creator					= kTVisualPluginCreator;
	
	playerMessageInfo.u.registerVisualPluginMessage.pulseRateInHz			= 1;	// update my state N times a second
	playerMessageInfo.u.registerVisualPluginMessage.numWaveformChannels		= 2;
	playerMessageInfo.u.registerVisualPluginMessage.numSpectrumChannels		= 2;
	
	playerMessageInfo.u.registerVisualPluginMessage.minWidth				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.minHeight				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.maxWidth				= 0;	// no max width limit
	playerMessageInfo.u.registerVisualPluginMessage.maxHeight				= 0;	// no max height limit
	
	status = PlayerRegisterVisualPlugin(messageInfo->u.initMessage.appCookie, messageInfo->u.initMessage.appProc,&playerMessageInfo);
		
	return status;
}

#define kTVisualPluginName              L"MiniLyrics Visualizer"

void DrawVisual(HWND destPort)
{
	RECT srcRect;
	GetClientRect(destPort, &srcRect);

	HBRUSH	hBrush;
	HDC		hdc;

	hdc = GetDC(destPort);
	hBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdc, &srcRect, hBrush);
	DeleteObject(hBrush);
	ReleaseDC(destPort, hdc);
}

void GetVisualName(ITUniStr255 name)
{	
	name[0] = (UniChar)wcslen( kTVisualPluginName );
	wcscpy_s( (wchar_t *)&name[1], 255, kTVisualPluginName );
}
