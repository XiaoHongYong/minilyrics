#include "StdAfx.h"
#include "../MPlayerLib/IMPlayer.h"
#include "MDWmpCore.h"
#include "MPTime.h"
#include "MPlayer.h"

class CWmpEventsSink : 
	public IWMPEvents
{
public:
	CWmpEventsSink(CMDWmpCore *pMDWmpCore)
	{
		m_mdWmpCore = pMDWmpCore;
		m_nReference = 0;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [annotation][iid_is][out] */ 
		__RPC__deref_out  void **ppvObject)
	{
		if (memcmp(&riid, &__uuidof(IWMPEvents), sizeof(UUID)) == 0)
			*ppvObject = (IWMPEvents *)this;
		else if (memcmp(&riid, &__uuidof(IUnknown), sizeof(UUID)) == 0)
			*ppvObject = (IUnknown *)this;
		else
		{
			*ppvObject = NULL;
			return S_FALSE;
		}
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return InterlockedIncrement(&m_nReference);
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		long n = InterlockedDecrement(&m_nReference);
		if (n == 0)
			delete this;
		return n;
	}

	long			m_nReference;

public:
	void STDMETHODCALLTYPE OpenStateChange( long NewState ) { }
	void STDMETHODCALLTYPE PlayStateChange( long NewState );
	void STDMETHODCALLTYPE AudioLanguageChange( long LangID ) { }
	void STDMETHODCALLTYPE StatusChange() { }
	void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param ) { }
	void STDMETHODCALLTYPE NewStream() { }
	void STDMETHODCALLTYPE Disconnect( long Result ) { }
	void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start ) { }
	void STDMETHODCALLTYPE Error() { }
	void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description ) { }
	void STDMETHODCALLTYPE EndOfStream( long Result );
	void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition) { }
	void STDMETHODCALLTYPE MarkerHit( long MarkerNum ) { }
	void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit ) { }
	void STDMETHODCALLTYPE CdromMediaChange( long CdromNum ) { }
	void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change ) { }
	void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change ) { }
	void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName ) { }
	void STDMETHODCALLTYPE MediaChange( IDispatch * Item ) { }
	void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName ) { }
	void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia) { }
	void STDMETHODCALLTYPE MediaCollectionChange() { }
	void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal ) { }
	void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal ) { }
	void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal) { }
	void STDMETHODCALLTYPE PlaylistCollectionChange() { }
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName) { }
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName) { }
	void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted) { }
	void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue) { }
	void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject) { }
	void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem ) { }
	void STDMETHODCALLTYPE DomainChange( BSTR strDomain) { }
	void STDMETHODCALLTYPE SwitchedToPlayerApplication() { }
	void STDMETHODCALLTYPE SwitchedToControl() { }
	void STDMETHODCALLTYPE PlayerDockedStateChange() { }
	void STDMETHODCALLTYPE PlayerReconnect() { }
	void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY ) { }
	void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY ) { }
	void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState ) { }
	void STDMETHODCALLTYPE KeyPress( short nKeyAscii ) { }
	void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState ) { }
	void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY ) { }
	void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY ) { }
	void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY ) { }

protected:
	CMPAutoPtr<CMDWmpCore>		m_mdWmpCore;

};

void CWmpEventsSink::PlayStateChange( long NewState )
{
	switch (NewState)
	{
	case wmppsUndefined:
		break;
	case wmppsStopped:
		if (m_mdWmpCore->m_mplayer)
			m_mdWmpCore->m_mplayer->NotifyEod(m_mdWmpCore, ERR_OK);
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
		if (m_mdWmpCore->m_mplayer)
			m_mdWmpCore->m_mplayer->NotifyEod(m_mdWmpCore, ERR_OK);
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

void CWmpEventsSink::EndOfStream( long Result )
{
	if (m_mdWmpCore->m_mplayer)
		m_mdWmpCore->m_mplayer->NotifyEod(m_mdWmpCore, ERR_OK);
}

//////////////////////////////////////////////////////////////////////////

CMDWmpCore::CMDWmpCore(void)
{
	OBJ_REFERENCE_INIT;

	m_dwAdviseCookie = 0;
	Init();
}

CMDWmpCore::~CMDWmpCore(void)
{
	if (m_pConnectionPoint)
	{
		if (0 != m_dwAdviseCookie)
		{
			m_pConnectionPoint->Unadvise(m_dwAdviseCookie);
			m_dwAdviseCookie = 0;
		}
		m_pConnectionPoint = NULL;
	}
}

MLRESULT HResultToMLResult(HRESULT hr)
{
	if (FAILED(hr))
		return ERR_FALSE;
	return ERR_OK;
}

LPCXSTR CMDWmpCore::GetDescription()
{
	return _T("Windows Media Player Core");
}

LPCXSTR CMDWmpCore::GetFileExtentions()
{
	return _T(".mp3|MP3 files|.wma|WMA files|.mp2|MP2 files");
}

MLRESULT CMDWmpCore::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	CMPAutoPtr<IWMPMedia>	media;
	HRESULT hr = m_collection->add((BSTR)pInput->GetSource(), &media);
	if (FAILED(hr))
		return HResultToMLResult(hr);

	double duration;
	hr = media->get_duration(&duration);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_DURATION, (int)(duration * 1000));

	bstr_s	artist, album, title;

	BSTR	pbstrItemName = SysAllocString(L"Artist");
	hr = media->getItemInfo(pbstrItemName, &artist);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_ARTIST, artist.c_str());

	pbstrItemName = SysAllocString(L"Album");
	hr = media->getItemInfo(pbstrItemName, &album);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_ALBUM, album.c_str());

	pbstrItemName = SysAllocString(L"Title");
	hr = media->getItemInfo(pbstrItemName, &title);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_TITLE, title.c_str());

	return ERR_OK;
}

//
// decode media file related methods
//

bool CMDWmpCore::IsSeekable()
{
	return true;
}

bool CMDWmpCore::IsUseOutputPlug()
{
	return false;
}

MLRESULT CMDWmpCore::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	m_mplayer = pPlayer;

	CMPAutoPtr<IWMPMedia>	media;
	HRESULT hr = m_collection->add((BSTR)pInput->GetSource(), &media);
	if (FAILED(hr))
		return HResultToMLResult(hr);

	m_controls->stop();
	m_playlist->clear();
	m_playlist->appendItem(media);
	m_controls->put_currentItem(media);

	m_controls->playItem(media);

	return ERR_OK;
}

MLRESULT CMDWmpCore::Pause()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	HRESULT hr = m_controls->pause();

	return HResultToMLResult(hr);
}

MLRESULT CMDWmpCore::Unpause()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	WMPPlayState	state;
	HRESULT hr = m_player->get_playState(&state);
	if (FAILED(hr))
		return HResultToMLResult(hr);

	if (state == wmppsPaused)
		m_controls->play();

	return ERR_OK;
}

MLRESULT CMDWmpCore::Stop()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	HRESULT hr = m_controls->stop();
	return HResultToMLResult(hr);
}

uint32 CMDWmpCore::GetLength()
{
	if (!IsOK())
		return 0;

	CMPAutoPtr<IWMPMedia>	media;
	HRESULT hr = m_controls->get_currentItem(&media);
	if (FAILED(hr))
		return 0;

	double duration = 0;
	hr = media->get_duration(&duration);
	if (FAILED(hr))
		return 0;

	return uint32(duration * 1000);
}

MLRESULT CMDWmpCore::Seek(uint32 nPos)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	double pos = (double)nPos / 1000;
	HRESULT hr = m_controls->put_currentPosition(pos);
	return HResultToMLResult(hr);
}

uint32 CMDWmpCore::GetPos()
{
	if (!IsOK())
		return 0;

	double pos = 0;
	HRESULT hr = m_controls->get_currentPosition(&pos);
	if (FAILED(hr))
		return 0;

	return (uint32)(pos * 1000);
}

MLRESULT CMDWmpCore::SetVolume(int nVolume, int nBanlance)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	CMPAutoPtr<IWMPSettings>	settings;

	HRESULT hr = m_player->get_settings(&settings);
	if (FAILED(hr))
		return HResultToMLResult(hr);

	hr = settings->put_volume(nVolume);

	return ERR_OK;
}

bool CMDWmpCore::Init()
{
	HRESULT hr = CoCreateInstance(__uuidof(WindowsMediaPlayer), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWMPPlayer), (LPVOID*)&m_player);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to create IWMPPlayer instance: %s"), Error2Str(hr).c_str());
		return false;
	}

	CMPAutoPtr<IConnectionPointContainer> connectionPointContainer;
	hr = m_player->QueryInterface(__uuidof(IConnectionPointContainer), (LPVOID*)&connectionPointContainer);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to Query IConnectionPointContainer: %s"), Error2Str(hr).c_str());
		return false;
	}

	hr = connectionPointContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_pConnectionPoint);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to FindConnectionPoint IWMPEvents: %s"), Error2Str(hr).c_str());
		return false;
	}

	CWmpEventsSink *pSink = new CWmpEventsSink(this);
	pSink->AddRef();
	hr = m_pConnectionPoint->Advise(pSink, &m_dwAdviseCookie);
	if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
		m_pConnectionPoint = NULL;

	hr = m_player->get_controls(&m_controls);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to get_controls : %s"), Error2Str(hr).c_str());
		return false;
	}

	hr = m_player->get_currentPlaylist(&m_playlist);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to get_currentPlaylist : %s"), Error2Str(hr).c_str());
		return false;
	}

	hr = m_player->get_mediaCollection(&m_collection);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to get_mediaCollection : %s"), Error2Str(hr).c_str());
		return false;
	}

	return true;
}

MLRESULT CMDWmpCore::DoDecode(IMedia *pMedia)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	MLRESULT					nRet;
	CXStr						strMedia;
	CMPAutoPtr<IMediaInput>		pInput;
	CMPTime		timePlayed;

	nRet = pMedia->GetSourceUrl(&strMedia);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pPlayer->m_pluginMgrAgent.NewInput(strMedia.c_str(), &pInput);
	if (nRet != ERR_OK)
		return nRet;

	GetMediaInfo(m_pPlayer, pInput, pMedia);

	nRet = Play(m_pPlayer, pInput);
	if (nRet != ERR_OK)
	{
		if (nRet == ERR_MI_NOT_FOUND && pMedia->GetID() != MEDIA_ID_INVALID)
		{
			// the source can't be opened, set it as deleted.
			m_pPlayer->m_pMediaLib->SetDeleted(&pMedia);
		}
		goto R_FAILED;
	}

	if (m_pPlayer->m_nVolume != -1)
	{
		if (m_pPlayer->m_bMute)
			SetVolume(0, 0);
		else
			SetVolume(m_pPlayer->m_nVolume, m_pPlayer->m_nBalance);
	}

	// set the play time
	timePlayed.GetCurrentTime();
	pMedia->SetAttribute(MA_TIME_PLAYED, timePlayed.m_time);

	if (pMedia->GetID() == MEDIA_ID_INVALID && m_pPlayer->IsAutoAddToMediaLib())
		m_pPlayer->m_pMediaLib->Add(pMedia);

	// If media info in media library isn't up to date,
	// update to media library.
	if (pMedia->GetID() != MEDIA_ID_INVALID && !pMedia->IsInfoUpdatedToMediaLib())
	{
		m_pPlayer->m_pMediaLib->UpdateMediaInfo(pMedia);
	}

	return ERR_OK;

R_FAILED:
	return nRet;
}

bool CMDWmpCore::IsOK()
{
	return m_player != NULL && m_playlist != NULL && m_controls != NULL && m_collection != NULL;
}
