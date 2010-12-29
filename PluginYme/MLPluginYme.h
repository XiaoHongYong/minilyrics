// MLPluginYme.h : Declaration of the CMLPluginYme
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include "PluginYme.h"
#include "PlayerImp.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// #define ERROUT(str)		::MessageBox(NULL, str, _T("MLError!"), MB_OK)

// CMLPluginYme
class ATL_NO_VTABLE CMLPluginYme :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IMLPluginYme, &IID_IMLPluginYme, &LIBID_PluginYmeLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IPersistStreamInitImpl<CMLPluginYme>,
	public IOleControlImpl<CMLPluginYme>,
	public IOleObjectImpl<CMLPluginYme>,
	public IOleInPlaceActiveObjectImpl<CMLPluginYme>,
	public IViewObjectExImpl<CMLPluginYme>,
	public IOleInPlaceObjectWindowlessImpl<CMLPluginYme>,
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
	public IObjectSafetyImpl<CMLPluginYme, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
	public CComCoClass<CMLPluginYme, &CLSID_MLPluginYme>,
	public CComControl<CMLPluginYme>,
	public IRMPPlugin,
	public IDispEventImpl<100, CMLPluginYme, &DIID_DPlaylistEngineEvents, &LIBID_RMPMediaPlayerLib, 1, 0>
{
public:


	CMLPluginYme()
	{
	}

	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
		OLEMISC_INSIDEOUT |
		OLEMISC_ACTIVATEWHENVISIBLE |
		OLEMISC_SETCLIENTSITEFIRST
		)

		DECLARE_REGISTRY_RESOURCEID(IDR_MLPLUGINYME)


	DECLARE_NOT_AGGREGATABLE(CMLPluginYme)

	BEGIN_COM_MAP(CMLPluginYme)
		COM_INTERFACE_ENTRY(IMLPluginYme)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IViewObjectEx)
		COM_INTERFACE_ENTRY(IViewObject2)
		COM_INTERFACE_ENTRY(IViewObject)
		COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceObject)
		COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
		COM_INTERFACE_ENTRY(IOleControl)
		COM_INTERFACE_ENTRY(IOleObject)
		COM_INTERFACE_ENTRY(IPersistStreamInit)
		COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
		COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafety)
#endif
		COM_INTERFACE_ENTRY(IRMPPlugin)
	END_COM_MAP()

	BEGIN_PROP_MAP(CMLPluginYme)
		PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
		PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
		// Example entries
		// PROP_ENTRY("Property Description", dispid, clsid)
		// PROP_PAGE(CLSID_StockColorPage)
	END_PROP_MAP()


	BEGIN_MSG_MAP(CMLPluginYme)
		CHAIN_MSG_MAP(CComControl<CMLPluginYme>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	// IMLPluginYme
public:
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		// Set Clip region to the rectangle specified by di.prcBounds
		HRGN hRgnOld = NULL;
		if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
			hRgnOld = NULL;
		bool bSelectOldRgn = false;

		HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		if (hRgnNew != NULL)
		{
			bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
		}

		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("MiniLyrics plugin for Yahoo! Music Jukebox. Please view lyrics in the MiniLyrics' window.");
#ifndef _WIN32_WCE
		TextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			pszText,
			lstrlen(pszText));
#else
		ExtTextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			ETO_OPAQUE,
			NULL,
			pszText,
			ATL::lstrlen(pszText),
			NULL);
#endif

		if (bSelectOldRgn)
			SelectClipRgn(di.hdcDraw, hRgnOld);

		return S_OK;
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IRMPPlugin Methods
	CComQIPtr< IRMPMediaPlayer >	m_pMediaPlayer;

public:
	STDMETHOD(Initialize)(LPDISPATCH mediaPlayer)
	{
		// MessageBox(L"MiniLyrics for YME", L"", MB_OK);

		m_pMediaPlayer = mediaPlayer;
		g_Player.m_pMediaPlayer = mediaPlayer;

		// Get the service drawer so we can add to it:
		CComQIPtr< IRMPServiceDrawer > pServiceDrawer;
		CComPtr< IDispatch > pDisp;
		HRESULT hr = m_pMediaPlayer->get_ServiceDrawer( &pDisp );
		if ( FAILED(hr) )
			return E_FAIL; // returning anything other than S_OK means the plugin will not load
		else // service drawer obtained!
			pServiceDrawer = pDisp;

		pDisp.Release(); // we don't need the IDispatch part of the service drawer anymore

		// Note that when a plugin is loaded, a hidden root node is already entered in the drawer.
		// We retrieve that node and un-hide it to display ourselves:
		CComQIPtr< IRMPServiceDrawerNode > pServiceDrawerNode;
		pServiceDrawer->get_RootNode( &pDisp );
		pServiceDrawerNode = pDisp;
		pServiceDrawerNode->put_Visible( VARIANT_TRUE ); // make ourselves visible

		// Hook up to the playlist engine's event callback:
		CComQIPtr< IDispatch > playlistEngine; // we fetch the playlist engine into here...
		m_pMediaPlayer->get_PlaylistEngine( &playlistEngine ); // fetched...really should check return val here!
		DispEventAdvise( playlistEngine ); // says that we want to be notified of events coming from the playlist engine

		RunMiniLyrics(g_hInstance, NULL, &g_Player);

		return S_OK;
	}
	STDMETHOD(Shutdown)()
	{
		return S_OK;
	}
	STDMETHOD(UICommand)(UICommandID command, BSTR mode, BSTR data)
	{
		return S_OK;
	}
	STDMETHOD(get_Name)(BSTR * pluginName)
	{
		CComBSTR name("MiniLyrics Plugin");
		*pluginName = name.Detach();

		return S_OK;
	}
	STDMETHOD(get_Description)(BSTR * descriptionText)
	{
		CComBSTR desc("MiniLyrics is the best synchronized lyrics viewer.");
		*descriptionText = desc.Detach();

		return S_OK;
	}
	STDMETHOD(get_SafeToShutdown)(VARIANT_BOOL * shutdownOK)
	{
		// Un-hook from the playlist engine's event callback:
		CComQIPtr< IDispatch > playlistEngine; // we fetch the playlist engine into here...
		m_pMediaPlayer->get_PlaylistEngine( &playlistEngine ); // fetched...really should check return val here!
		DispEventUnadvise( playlistEngine ); // no longer receive playlist engine events

		g_MLService.Quit();

		m_pMediaPlayer.Release();

		return S_OK;
	}
	STDMETHOD(get_SafeToShutdownReason)(BSTR * descriptionText)
	{
		return S_OK;
	}

public:
	virtual void STDMETHODCALLTYPE OnPlayTrack(IDispatch * RMPMediaObject, LONG indexToBePlayed)
	{
		// If we stop playback, clear the currently
		// playing display:

		if (m_pMediaPlayer.p == NULL)
			return;

		if (indexToBePlayed < 0)
		{
			g_MLService.OnMediaInfoChanged(NULL);
			return;
		}

		EmptyStr(g_Player.m_szArtist);
		EmptyStr(g_Player.m_szAlbum);
		EmptyStr(g_Player.m_szTitle);

		// Get the currently playing object:
		CComQIPtr< IRMPMediaObject > pMediaObject = RMPMediaObject;
		CComQIPtr< IDispatch > pDisp;
		CComQIPtr< IRMPMetadata > pMetadata;

		if (pMediaObject.p == NULL)
			return;

		pMediaObject->get_Metadata(&pDisp);
		pMetadata = pDisp;

		if (pMetadata.p == NULL)
			return;

		_bstr_t		artist, title, album, length, url;
		MLMediaInfo	mediaInfo;

		// get length
		pMetadata->GetPredefinedValue(METADATA_DURATION, length.GetAddress());
		mediaInfo.nMediaLength = _wtoi((const WCHAR *)length);

		// get title and artist
		pMetadata->GetPredefinedValue(METADATA_ARTIST, artist.GetAddress());
		mediaInfo.szArtist = artist;

		pMetadata->GetPredefinedValue(METADATA_ALBUM, album.GetAddress());
		mediaInfo.szAlbum = album;

		pMetadata->GetPredefinedValue(METADATA_TITLE, title.GetAddress());
		mediaInfo.szTitle = title;

		// get media source
		pMediaObject->get_Path(url.GetAddress());
		mediaInfo.szMediaUrl = url;

		g_MLService.OnMediaInfoChanged(&mediaInfo);
	}

	BEGIN_SINK_MAP(CMLPluginYme)
		SINK_ENTRY_EX(100, DIID_DPlaylistEngineEvents, 0x2783, OnPlayTrack)
	END_SINK_MAP()

};

OBJECT_ENTRY_AUTO(__uuidof(MLPluginYme), CMLPluginYme)
