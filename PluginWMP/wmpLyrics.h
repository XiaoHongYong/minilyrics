/////////////////////////////////////////////////////////////////////////////
//
// wmpLyrics.h : Declaration of the CWmpLyrics
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WMPLYRICS_H_
#define __WMPLYRICS_H_

#pragma once

#include "resource.h"
#include "wmpplug.h"
// class CPluginWindow;

// {46B5EE7F-3B6B-4079-A756-5EFC10B1F50B}
DEFINE_GUID(CLSID_WmpLyrics, 0x46b5ee7f, 0x3b6b, 0x4079, 0xa7, 0x56, 0x5e, 0xfc, 0x10, 0xb1, 0xf5, 0xb);

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics
class ATL_NO_VTABLE CWmpLyrics : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWmpLyrics, &CLSID_WmpLyrics>,
    public IWMPEvents,
    public IWMPPluginUI
{
public:
    CWmpLyrics();
    ~CWmpLyrics();

DECLARE_REGISTRY_RESOURCEID(IDR_WMPLYRICS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWmpLyrics)
    COM_INTERFACE_ENTRY(IWMPEvents)
    COM_INTERFACE_ENTRY(IWMPPluginUI)
END_COM_MAP()

    // CComCoClass methods
    HRESULT FinalConstruct();
    void    FinalRelease();

    // IWMPPluginUI methods
    STDMETHODIMP SetCore(IWMPCore *pCore);
    STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow);
    STDMETHODIMP Destroy();
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
    STDMETHODIMP DisplayPropertyPage(HWND hwndParent) { return E_NOTIMPL; }
    STDMETHODIMP GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty);
    STDMETHODIMP SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty);

    // IWMPEvents methods
    void STDMETHODCALLTYPE OpenStateChange( long NewState );
    void STDMETHODCALLTYPE PlayStateChange( long NewState );
    void STDMETHODCALLTYPE AudioLanguageChange( long LangID );
    void STDMETHODCALLTYPE StatusChange();
    void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param );
    void STDMETHODCALLTYPE NewStream();
    void STDMETHODCALLTYPE Disconnect( long Result );
    void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start );
    void STDMETHODCALLTYPE Error();
    void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description );
    void STDMETHODCALLTYPE EndOfStream( long Result );
    void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition);
    void STDMETHODCALLTYPE MarkerHit( long MarkerNum );
    void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit );
    void STDMETHODCALLTYPE CdromMediaChange( long CdromNum );
    void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE MediaChange( IDispatch * Item );
    void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
    void STDMETHODCALLTYPE MediaCollectionChange();
    void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal);
    void STDMETHODCALLTYPE PlaylistCollectionChange();
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted);
    void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue);
    void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject);
    void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem );
    void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
    void STDMETHODCALLTYPE SwitchedToPlayerApplication();
    void STDMETHODCALLTYPE SwitchedToControl();
    void STDMETHODCALLTYPE PlayerDockedStateChange();
    void STDMETHODCALLTYPE PlayerReconnect();
    void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE KeyPress( short nKeyAscii );
    void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY );

private:
    void         ReleaseCore();

    CComPtr<IWMPCore>           m_spCore;
    // CPluginWindow               *m_pPluginWindow;
    CComPtr<IConnectionPoint>   m_spConnectionPoint;
    DWORD                       m_dwAdviseCookie;
public:
	IWMPCore* GetCore(void);
};

#endif //__WMPLYRICS_H_
