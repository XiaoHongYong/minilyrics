// PluginVLC.cpp : Defines the entry point for the DLL application.
//

#pragma warning ( disable: 4996)

#include <Windows.h>
#include <vlc/libvlc_structures.h>
#include <vlc/libvlc_events.h>
#include <vlc/mediacontrol.h>
#include <vlc_common.h>

#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_meta.h>

VLC_EXPORT( void, __pl_Release, ( vlc_object_t * ) );

#include <vlc_playlist.h>


#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"


#define N_(str) (str)


struct intf_sys_t
{
	bool		bMetaDataRead;
};


int  Open            ( vlc_object_t * );
void Close           ( vlc_object_t * );

int ItemChange( vlc_object_t *, const char *,
					  vlc_value_t, vlc_value_t, void * );

int PlayingChange    ( vlc_object_t *, const char *, vlc_value_t,
							 vlc_value_t, void * );

void Run(intf_thread_t *p_intf);

#define MODULE_NAME  "MiniLyrics"
#define MODULE_STRING  "MiniLyrics"
#define MODULE_NAME_IS_  "MiniLyrics"

int UTF8ToUCS2(unsigned char* text, int length, WCHAR *ucs2, int ubuflen);


void Message(LPCTSTR str)
{
	MessageBox(NULL, str, L"xx", MB_OK);
}

void MessageA(LPCSTR str)
{
	MessageBoxA(NULL, str, "xx", MB_OK);
}

vlc_module_begin();
set_category( CAT_INTERFACE );
set_subcategory( SUBCAT_INTERFACE_CONTROL );
set_shortname( N_( "MiniLyrics" ) );
set_description( N_("MiniLyrics - Auto display lyrics for current playing song") );
// add_submodule( );
set_capability( "interface", 0 );
set_callbacks( Open, Close );
vlc_module_end();


int  Open( vlc_object_t * p_this)
{
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	playlist_t *p_playlist;

	MALLOC_ERR( p_intf->p_sys, intf_sys_t );

	p_playlist = pl_Yield( p_intf );
	var_AddCallback( p_playlist, "item-change", ItemChange, p_intf );
	var_AddCallback( p_playlist, "playlist-current", ItemChange, p_intf );
	pl_Release( p_intf );

	g_hwndPlayer = NULL;

	p_intf->pf_run = Run;

	return VLC_SUCCESS;
}

void Close( vlc_object_t * p_this)
{
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	playlist_t *p_playlist = pl_Yield( p_this );
	input_thread_t              *p_input;

	var_DelCallback( p_playlist, "item-change", ItemChange, p_intf );
	var_DelCallback( p_playlist, "playlist-current", ItemChange, p_intf );

	p_input = p_playlist->p_input;
	if ( p_input )
	{
		vlc_object_yield( p_input );
		var_DelCallback( p_input, "state", PlayingChange, p_intf );
		vlc_object_release( p_input );
	}

	pl_Release( p_this );

	g_MLService.Quit();

	/* Destroy structure */
	free( p_intf->p_sys );
}

void Run(intf_thread_t *p_intf)
{
	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return;

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintfA(szMsg, "Start Foobar2000 plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		vlc_object_lock( p_intf );
		if( !vlc_object_alive( p_intf ) )
		{
			vlc_object_unlock( p_intf );
			msg_Dbg(p_intf, "MiniLyrics plugin is dying");
			return;
		}
		vlc_object_unlock( p_intf );
	}
}

int ReadMetaData( intf_thread_t *p_this )
{
	playlist_t          *p_playlist;
	input_thread_t      *p_input;
	input_item_t        *p_item;
	static char			sz_LastFile[256] = { 0 };

	char *psz_title = NULL;
	char *psz_artist = NULL;
	char *psz_album = NULL;
	char *psz_url = NULL;

	intf_sys_t          *p_sys = p_this->p_sys;

	p_playlist = pl_Yield( p_this );
	PL_LOCK;
	p_input = p_playlist->p_input;
	if (!p_input)
	{
		PL_UNLOCK;
		pl_Release( p_this );
		return( VLC_SUCCESS );
	}

	vlc_object_yield( p_input );
	PL_UNLOCK;
	pl_Release( p_this );

	if( p_input->b_dead || !input_GetItem(p_input)->psz_name )
	{
		/* Not playing anything ... */
		vlc_object_release( p_input );
		return VLC_SUCCESS;
	}

	psz_url = input_item_GetURL( input_GetItem( p_input ) );
	if (strcmp(sz_LastFile, psz_url) == 0)
	{
		// song file Not changed.
		vlc_object_release( p_input );
		return VLC_SUCCESS;
	}

	psz_artist = input_item_GetArtist( input_GetItem( p_input ) );
	psz_album = input_item_GetAlbum( input_GetItem( p_input ) );
	psz_title = input_item_GetTitle( input_GetItem( p_input ) );
	if( !psz_artist ) psz_artist = strdup( "" );
	if( !psz_album ) psz_album = strdup( "" );
	if( !psz_title )
		psz_title = input_item_GetName( input_GetItem( p_input ) );

	int64_t		nLength;

	input_Control(p_input, INPUT_GET_LENGTH, &nLength);
	g_Player.m_nLength = nLength;

// 
// 	double		dbPos;
// 	input_Control(p_input, INPUT_GET_POSITION, &dbPos);
// 
// 	input_Control(p_input, INPUT_SET_POSITION, dbPos);
// 
	wchar_t		wszFullTitle[256];

	UTF8ToUCS2((BYTE *)psz_url, strlen((char *)psz_url) + 1, g_Player.m_szSrcMedia, CountOf(g_Player.m_szSrcMedia));

	UTF8ToUCS2((BYTE *)psz_artist, strlen((char *)psz_artist) + 1, g_Player.m_szArtist, CountOf(g_Player.m_szArtist));

	UTF8ToUCS2((BYTE *)psz_album, strlen((char *)psz_album) + 1, g_Player.m_szAlbum, CountOf(g_Player.m_szAlbum));

	UTF8ToUCS2((BYTE *)psz_title, strlen((char *)psz_title) + 1, g_Player.m_szTitle, CountOf(g_Player.m_szTitle));

	UTF8ToUCS2((BYTE *)psz_title, strlen((char *)psz_title) + 1, wszFullTitle, CountOf(wszFullTitle));

	g_Player.OnSongChanged(wszFullTitle);

// 
// 	snprintf(psz_tmp,
// 		MAX_PATH,
// 		"ar: %s, ti: %s, al: %s",
// 		psz_artist,
// 		psz_title,
// 		psz_album );
// 	// 	free( psz_title );
// 	// 	free( psz_artist );
// 	// 	free( psz_album );
// 
// 	MessageA(psz_tmp);

	vlc_object_release( p_input );

	return VLC_SUCCESS;
}

/*****************************************************************************
* PlayingChange: Playing status change callback
*****************************************************************************/
static int PlayingChange( vlc_object_t *p_this, const char *psz_var,
						 vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
	intf_thread_t   *p_intf = ( intf_thread_t* ) p_data;
	intf_sys_t      *p_sys  = p_intf->p_sys;

	VLC_UNUSED( p_this ); VLC_UNUSED( psz_var );

	MessageA("PlayingChange");

	//    if( p_intf->b_dead )
	//        return VLC_SUCCESS;

	if (!p_sys->bMetaDataRead && newval.i_int >= PLAYING_S)
	{
		ReadMetaData( p_intf );
	}

	if( newval.i_int >= END_S )
	{
		g_Player.NotifyPlayerStatChanged(PS_STOPED);
	}
	else if (newval.i_int == PAUSE_S )
	{
		g_Player.NotifyPlayerStatChanged(PS_PAUSED);
	}
	else if (newval.i_int == PLAYING_S )
	{
		g_Player.NotifyPlayerStatChanged(PS_PLAYING);
	}

	return VLC_SUCCESS;
}

/*****************************************************************************
* ItemChange: Playlist item change callback
*****************************************************************************/
static int ItemChange( vlc_object_t *p_this, const char *psz_var,
					  vlc_value_t oldval, vlc_value_t newval, void *param )
{
	(void)psz_var;    (void)oldval;    (void)newval;
	intf_thread_t *p_intf = (intf_thread_t *)param;
	char psz_tmp[MAX_PATH];
	char *psz_title = NULL;
	char *psz_artist = NULL;
	char *psz_album = NULL;
	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield( p_this );

	p_input = p_playlist->p_input;
	pl_Release( p_this );

	if( !p_input ) return VLC_SUCCESS;
	vlc_object_yield( p_input );

	if( p_input->b_dead || !input_GetItem(p_input)->psz_name )
	{
		/* Not playing anything ... */
		vlc_object_release( p_input );
		return VLC_SUCCESS;
	}

	var_AddCallback( p_input, "state", PlayingChange, p_intf );

	/* Playing something ... */
	ReadMetaData(p_intf);
// 	psz_artist = input_item_GetArtist( input_GetItem( p_input ) );
// 	psz_album = input_item_GetAlbum( input_GetItem( p_input ) );
// 	psz_title = input_item_GetTitle( input_GetItem( p_input ) );
// 	if( !psz_artist ) psz_artist = strdup( "" );
// 	if( !psz_album ) psz_album = strdup( "" );
// 	if( !psz_title )
// 		psz_title = input_item_GetName( input_GetItem( p_input ) );
// 
// 	snprintf(psz_tmp,
// 		MAX_PATH,
// 		"ar: %s, ti: %s, al: %s",
// 		psz_artist,
// 		psz_title,
// 		psz_album );
// // 	free( psz_title );
// // 	free( psz_artist );
// // 	free( psz_album );
// 
// 	MessageA(psz_tmp);

	vlc_object_release( p_input );

	return VLC_SUCCESS;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MessageA("DllMain");
		g_hInstance = hModule;
	}

    return TRUE;
}
