/////////////////////////////////////////////////////////////////////////////
//
// PluginWMPVisEvents.cpp : Implementation of CPluginWMPVis events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PluginWMPVis.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

void CPluginWMPVis::OpenStateChange( long NewState )
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

void CPluginWMPVis::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
	case wmppsStopped:
		g_MLService.OnPlayStateChanged(PS_STOPED);
        break;
	case wmppsPaused:
		g_MLService.OnPlayStateChanged(PS_PAUSED);
        break;
	case wmppsPlaying:
		g_MLService.OnPlayStateChanged(PS_PLAYING);
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

void CPluginWMPVis::AudioLanguageChange( long LangID )
{
}

void CPluginWMPVis::StatusChange()
{
}

void CPluginWMPVis::ScriptCommand( BSTR scType, BSTR Param )
{
}

void CPluginWMPVis::NewStream()
{
}

void CPluginWMPVis::Disconnect( long Result )
{
}

void CPluginWMPVis::Buffering( VARIANT_BOOL Start )
{
}

void CPluginWMPVis::Error()
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

void CPluginWMPVis::Warning( long WarningType, long Param, BSTR Description )
{
}

void CPluginWMPVis::EndOfStream( long Result )
{
}

void CPluginWMPVis::PositionChange( double oldPosition, double newPosition)
{
}

void CPluginWMPVis::MarkerHit( long MarkerNum )
{
}

void CPluginWMPVis::DurationUnitChange( long NewDurationUnit )
{
}

void CPluginWMPVis::CdromMediaChange( long CdromNum )
{
}

void CPluginWMPVis::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change )
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

void CPluginWMPVis::CurrentPlaylistChange( WMPPlaylistChangeEventType change )
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

void CPluginWMPVis::CurrentPlaylistItemAvailable( BSTR bstrItemName )
{
}

void CPluginWMPVis::MediaChange( IDispatch * Item )
{
}

void CPluginWMPVis::CurrentMediaItemAvailable( BSTR bstrItemName )
{
}

void CPluginWMPVis::CurrentItemChange( IDispatch *pdispMedia)
{
}

void CPluginWMPVis::MediaCollectionChange()
{
}

void CPluginWMPVis::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CPluginWMPVis::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CPluginWMPVis::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void CPluginWMPVis::PlaylistCollectionChange()
{
}

void CPluginWMPVis::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName)
{
}

void CPluginWMPVis::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName)
{
}

void CPluginWMPVis::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted)
{
}

void CPluginWMPVis::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CPluginWMPVis::MediaError( IDispatch * pMediaObject)
{
}

void CPluginWMPVis::OpenPlaylistSwitch( IDispatch *pItem )
{
}

void CPluginWMPVis::DomainChange( BSTR strDomain)
{
}

void CPluginWMPVis::SwitchedToPlayerApplication()
{
}

void CPluginWMPVis::SwitchedToControl()
{
}

void CPluginWMPVis::PlayerDockedStateChange()
{
}

void CPluginWMPVis::PlayerReconnect()
{
}

void CPluginWMPVis::Click( short nButton, short nShiftState, long fX, long fY )
{
}

void CPluginWMPVis::DoubleClick( short nButton, short nShiftState, long fX, long fY )
{
}

void CPluginWMPVis::KeyDown( short nKeyCode, short nShiftState )
{
}

void CPluginWMPVis::KeyPress( short nKeyAscii )
{
}

void CPluginWMPVis::KeyUp( short nKeyCode, short nShiftState )
{
}

void CPluginWMPVis::MouseDown( short nButton, short nShiftState, long fX, long fY )
{
}

void CPluginWMPVis::MouseMove( short nButton, short nShiftState, long fX, long fY )
{
}

void CPluginWMPVis::MouseUp( short nButton, short nShiftState, long fX, long fY )
{
}
