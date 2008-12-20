/////////////////////////////////////////////////////////////////////////////
//
// wmpLyricsEvents.cpp : Implementation of CWmpLyrics events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "wmpLyrics.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

void CWmpLyrics::OpenStateChange( long NewState )
{
    switch (NewState)
    {
    case wmposUndefined:
        break;
	case wmposPlaylistChanging:
        break;
	case wmposPlaylistLocating:
        break;
	case wmposPlaylistConnecting:
        break;
	case wmposPlaylistLoading:
        break;
	case wmposPlaylistOpening:
        break;
	case wmposPlaylistOpenNoMedia:
        break;
	case wmposPlaylistChanged:
        break;
	case wmposMediaChanging:
        break;
	case wmposMediaLocating:
        break;
	case wmposMediaConnecting:
        break;
	case wmposMediaLoading:
        break;
	case wmposMediaOpening:
        break;
	case wmposMediaOpen:
        break;
	case wmposBeginCodecAcquisition:
        break;
	case wmposEndCodecAcquisition:
        break;
	case wmposBeginLicenseAcquisition:
        break;
	case wmposEndLicenseAcquisition:
        break;
	case wmposBeginIndividualization:
        break;
	case wmposEndIndividualization:
        break;
	case wmposMediaWaiting:
        break;
	case wmposOpeningUnknownURL:
        break;
    default:
        break;
    }
}

void CWmpLyrics::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
	case wmppsStopped:
        break;
	case wmppsPaused:
        break;
	case wmppsPlaying:
        break;
	case wmppsScanForward:
        break;
	case wmppsScanReverse:
        break;
	case wmppsBuffering:
        break;
	case wmppsWaiting:
        break;
	case wmppsMediaEnded:
        break;
	case wmppsTransitioning:
        break;
	case wmppsReady:
        break;
	case wmppsReconnecting:
        break;
	case wmppsLast:
        break;
    default:
        break;
    }
}

void CWmpLyrics::AudioLanguageChange( long LangID )
{
}

void CWmpLyrics::StatusChange()
{
}

void CWmpLyrics::ScriptCommand( BSTR scType, BSTR Param )
{
}

void CWmpLyrics::NewStream()
{
}

void CWmpLyrics::Disconnect( long Result )
{
}

void CWmpLyrics::Buffering( VARIANT_BOOL Start )
{
}

void CWmpLyrics::Error()
{
    CComPtr<IWMPError>      spError;
    CComPtr<IWMPErrorItem>  spErrorItem;
    HRESULT                 dwError = S_OK;
    HRESULT                 hr = S_OK;

    if (m_spCore)
    {
        hr = m_spCore->get_error(&spError);

        if (SUCCEEDED(hr))
        {
            hr = spError->get_item(0, &spErrorItem);
        }

        if (SUCCEEDED(hr))
        {
            hr = spErrorItem->get_errorCode( (long *) &dwError );
        }
    }
}

void CWmpLyrics::Warning( long WarningType, long Param, BSTR Description )
{
}

void CWmpLyrics::EndOfStream( long Result )
{
}

void CWmpLyrics::PositionChange( double oldPosition, double newPosition)
{
}

void CWmpLyrics::MarkerHit( long MarkerNum )
{
}

void CWmpLyrics::DurationUnitChange( long NewDurationUnit )
{
}

void CWmpLyrics::CdromMediaChange( long CdromNum )
{
}

void CWmpLyrics::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CWmpLyrics::CurrentPlaylistChange( WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CWmpLyrics::CurrentPlaylistItemAvailable( BSTR bstrItemName )
{
}

void CWmpLyrics::MediaChange( IDispatch * Item )
{
	g_Player.OnMediaChanged();
}

void CWmpLyrics::CurrentMediaItemAvailable( BSTR bstrItemName )
{
}

void CWmpLyrics::CurrentItemChange( IDispatch *pdispMedia)
{
}

void CWmpLyrics::MediaCollectionChange()
{
}

void CWmpLyrics::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CWmpLyrics::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CWmpLyrics::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void CWmpLyrics::PlaylistCollectionChange()
{
}

void CWmpLyrics::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName)
{
}

void CWmpLyrics::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName)
{
}

void CWmpLyrics::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted)
{
}

void CWmpLyrics::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CWmpLyrics::MediaError( IDispatch * pMediaObject)
{
}

void CWmpLyrics::OpenPlaylistSwitch( IDispatch *pItem )
{
}

void CWmpLyrics::DomainChange( BSTR strDomain)
{
}

void CWmpLyrics::SwitchedToPlayerApplication()
{
}

void CWmpLyrics::SwitchedToControl()
{
}

void CWmpLyrics::PlayerDockedStateChange()
{
}

void CWmpLyrics::PlayerReconnect()
{
}

void CWmpLyrics::Click( short nButton, short nShiftState, long fX, long fY )
{
}

void CWmpLyrics::DoubleClick( short nButton, short nShiftState, long fX, long fY )
{
}

void CWmpLyrics::KeyDown( short nKeyCode, short nShiftState )
{
}

void CWmpLyrics::KeyPress( short nKeyAscii )
{
}

void CWmpLyrics::KeyUp( short nKeyCode, short nShiftState )
{
}

void CWmpLyrics::MouseDown( short nButton, short nShiftState, long fX, long fY )
{
}

void CWmpLyrics::MouseMove( short nButton, short nShiftState, long fX, long fY )
{
}

void CWmpLyrics::MouseUp( short nButton, short nShiftState, long fX, long fY )
{
}
