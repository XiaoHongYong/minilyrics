// MPlayer.cpp: implementation of the CMPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayer.h"
#include "MILocalFile.h"
// #include "MDWave.h"

#ifdef _WIN32
#include "win32/MOSoundCard.h"
#endif

#ifdef _LINUX
#include "linux/MOSoundCard.h"
#endif

// #include "MOAnalysis.h"
#include "MDRow.h"
#include "MDWave.h"
#include "MDLibmad.h"
// #include "VISDemo.h"
// #include "DspDemo.h"
#include "MORaw.h"
#include "../MPlayer-Plugins/Dsp_SuperEQ/dsp_supereq.h"

#define SIZE_INC (1024 * 4) 

#ifdef _WIN32_WCE
// in WCE, use static lib to link all the decoders.
IMediaDecode *MPlayerGetMDOgg();
IMediaDecode *MPlayerGetMDMP4();
IMediaDecode *MPlayerGetMDWMA();
IMediaDecode *MPlayerGetMDFlac();
#endif

// MLRESULT GetPlayerInstance(IMPlayer **ppPlayer);
// {
// 	return CMPlayer::GetInstance(ppPlayer);
// }

//////////////////////////////////////////////////////////////////////////
// CFBuffer

CFBuffer::CFBuffer(CMemAllocator *pMemAllocator)
{
	OBJ_REFERENCE_INIT
	m_buf = NULL;
	m_pMemAllocator = pMemAllocator;
	m_nCapacity = 0;
	m_nSize = 0;

	m_pMemAllocator->AddRef();
}

CFBuffer::~CFBuffer()
{
	if (m_buf)
		free(m_buf);

	m_pMemAllocator->Release();
}

void CFBuffer::AddRef()
{
	InterlockedIncrement(&m_nReference);
}

void CFBuffer::Release()
{
	if (InterlockedDecrement(&m_nReference) <= 0)
	{
		m_pMemAllocator->OnRelease(this);
	}
}

char *CFBuffer::data()
{
	return m_buf;
}

uint32 CFBuffer::size()
{
	return m_nSize;
}

uint32 CFBuffer::capacity()
{
	return m_nCapacity;
}

void CFBuffer::resize(uint32 nSize)
{
	ASSERT(nSize <= m_nCapacity);
	if (nSize > m_nCapacity)
		nSize = m_nCapacity;
	m_nSize = nSize;
}

MLRESULT CFBuffer::reserve(uint32 nCapacity)
{
	// ASSERT(m_nRef == 1);
	if (nCapacity > m_nCapacity)
	{
		m_buf = (char *)realloc(m_buf, nCapacity);
		ASSERT(m_buf);
		if (!m_buf)
			return ERR_NO_MEM;

		m_nCapacity = nCapacity;
	}

	return ERR_OK;
}

bool CFBuffer::New(uint32 nCapacity)
{
	m_buf = (char *)malloc(nCapacity);
	if (!m_buf)
		return false;

	m_nCapacity = nCapacity;
	m_nSize = 0;

	return true;
}

CMemAllocator::CMemAllocator()
{
	OBJ_REFERENCE_INIT;
}

CMemAllocator::~CMemAllocator()
{
	ASSERT(m_listFree.size() == 0);
}

OBJ_REFERENCE_IMP(CMemAllocator)

void CMemAllocator::Quit()
{
	LIST_BUF::iterator	it;

	for (it = m_listFree.begin(); it != m_listFree.end(); ++it)
	{
		CFBuffer		*pBuffer = (CFBuffer*)(IFBuffer*)(*it);

		delete pBuffer;
		// pBuffer->Release();
	}
	m_listFree.clear();
}

IFBuffer *CMemAllocator::AllocFBuffer(uint32 nCapacity)
{
	LIST_BUF::iterator	it;
	CMutexAutolock	autolock(m_mutex);

	for (it = m_listFree.begin(); it != m_listFree.end(); ++it)
	{
		IFBuffer		*pBuffer = *it;
		if (pBuffer->capacity() >= nCapacity)
		{
			pBuffer->AddRef();
			m_listFree.erase(it);
			return pBuffer;
		}
	}

	CFBuffer	*pBuffer;
	pBuffer = new CFBuffer(this);
	if (!pBuffer->New(nCapacity))
	{
		delete pBuffer;
		return NULL;
	}

	pBuffer->AddRef();

	static int	nCount = 0;
	nCount++;
	DBGLOG1(_T("mallocated mem count: %d"), nCount);

	return pBuffer;
}

IXStr *CMemAllocator::AllocStr()
{
	return new CXStr();
}

void CMemAllocator::OnRelease(IFBuffer *pBuf)
{
	pBuf->resize(0);
	CMutexAutolock	autolock(m_mutex);
	m_listFree.push_front(pBuf);
}

//////////////////////////////////////////////////////////////////////////
CMOAgent::CMOAgent()
{
	OBJ_REFERENCE_INIT;
}

CMOAgent::~CMOAgent()
{
}

OBJ_REFERENCE_IMP(CMOAgent)

LPCXSTR CMOAgent::GetDescription()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->GetDescription();
	else
		return _T("No output device is available.");
}

MLRESULT CMOAgent::Init(IMPlayer *pPlayer)
{
	return ERR_OK;
}

MLRESULT CMOAgent::Quit()
{
	return ERR_OK;
}

MLRESULT CMOAgent::Open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
	if (m_pMediaOutput)
		return m_pMediaOutput->Open(nSampleRate, nNumChannels, nBitsPerSamp);
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::WaitForWrite()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->WaitForWrite();
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::Write(IFBuffer *pBuf)
{
	if (m_pMediaOutput)
		return m_pMediaOutput->Write(pBuf);
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::Flush()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->Flush();
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::Pause(bool bPause)
{
	if (m_pMediaOutput)
		return m_pMediaOutput->Pause(bPause);
	else
		return ERR_NO_DEVICE;
}

bool CMOAgent::IsPlaying()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->IsPlaying();
	else
		return false;
}

MLRESULT CMOAgent::Stop()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->Stop();
	else
		return ERR_NO_DEVICE;
}

bool CMOAgent::IsOpened()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->IsOpened();
	else
		return false;
}

uint32 CMOAgent::GetPos()
{
	if (m_pMediaOutput)
		return m_pMediaOutput->GetPos();
	else
		return 0;
}

// volume
MLRESULT CMOAgent::SetVolume(int nVolume, int nBanlance)
{
	if (m_pMediaOutput)
		return m_pMediaOutput->SetVolume(nVolume, nBanlance);
	else
		return ERR_NO_DEVICE;
}

//////////////////////////////////////////////////////////////////////////

MLRESULT CMPluginManagerAgent::DetectPlugins()
{
	if (m_pluginMgr)
		m_pluginMgr->DetectPlugins();

	//
	// Register internal plugins
	//
	IMediaDecode	*pDecoder;

	pDecoder = new CMDLibmad;
	OnInternalDecoderRegister(pDecoder);
	pDecoder->Release();

	pDecoder = new CMDWave;
	OnInternalDecoderRegister(pDecoder);
	pDecoder->Release();

	pDecoder = new CMDRow;
	OnInternalDecoderRegister(pDecoder);
	pDecoder->Release();

#ifdef _WIN32_WCE // in WCE, use static lib to link all the decoders.
	pDecoder = MPlayerGetMDOgg();
	if (pDecoder)
	{
		OnInternalDecoderRegister(pDecoder);
		pDecoder->Release();
	}

	pDecoder = MPlayerGetMDMP4();
	if (pDecoder)
	{
		OnInternalDecoderRegister(pDecoder);
		pDecoder->Release();
	}

	pDecoder = MPlayerGetMDWMA();
	if (pDecoder)
	{
		OnInternalDecoderRegister(pDecoder);
		pDecoder->Release();
	}

	pDecoder = MPlayerGetMDFlac();
	if (pDecoder)
	{
		OnInternalDecoderRegister(pDecoder);
		pDecoder->Release();
	}
#endif // _WIN32_WCE

	return ERR_OK;
}


MLRESULT CMPluginManagerAgent::NewInput(LPCXSTR szMediaUrl, IMediaInput **ppInput)
{
	if (m_pluginMgr && m_pluginMgr->NewInput(szMediaUrl, ppInput) == ERR_OK)
		return ERR_OK;

	MLRESULT				nRet;

	*ppInput = new CMILocalFile;

	nRet = (*ppInput)->Open(szMediaUrl);
	if (nRet != ERR_OK)
	{
		delete *ppInput;
		*ppInput = NULL;
		return nRet;
	}

	(*ppInput)->AddRef();

	return ERR_OK;
}


MLRESULT CMPluginManagerAgent::NewDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder)
{
	ASSERT(pInput);

	LPCXSTR szMediaUrl = pInput->GetSource();

	if (FileIsExtSame(szMediaUrl, _T(".mp3")))
		*ppDecoder = new CMDLibmad;
	else if (FileIsExtSame(szMediaUrl, _T(".wav")))
		*ppDecoder = new CMDWave;
	else if (FileIsExtSame(szMediaUrl, _T(".raw")))
		*ppDecoder = new CMDRow;
#ifdef _WIN32_WCE // in WCE, use static lib to link all the decoders.
	else if (FileIsExtSame(szMediaUrl, _T(".flac")))
		*ppDecoder = MPlayerGetMDFlac();
	else if (FileIsExtSame(szMediaUrl, _T(".wma")))
		*ppDecoder = MPlayerGetMDWMA();
	else if (FileIsExtSame(szMediaUrl, _T(".mp4"))
		|| FileIsExtSame(szMediaUrl, _T(".m4a"))
		|| FileIsExtSame(szMediaUrl, _T(".aac")))
		*ppDecoder = MPlayerGetMDMP4();
	else if (FileIsExtSame(szMediaUrl, _T(".ogg")))
		*ppDecoder = MPlayerGetMDOgg();
	else if (FileIsExtSame(szMediaUrl, _T(".raw")))
		*ppDecoder = new CMDRow;
#endif // _WIN32_WCE
	else if (m_pluginMgr)
		return m_pluginMgr->NewDecoder(pInput, ppDecoder);
	else
	{
		*ppDecoder = NULL;
		return ERR_NOT_SUPPORT_FILE_FORMAT;
	}

	if (*ppDecoder)
	{
		(*ppDecoder)->AddRef();
		return ERR_OK;
	}
	else
		return ERR_NOT_SUPPORT_FILE_FORMAT;
}


MLRESULT CMPluginManagerAgent::NewOutput(IMediaOutput **ppOutput)
{
	if (m_pluginMgr && m_pluginMgr->NewOutput(ppOutput) == ERR_OK)
		return ERR_OK;

	*ppOutput = new CMOSoundCard;

	return ERR_OK;
}


MLRESULT CMPluginManagerAgent::GetActiveDSP(IDSP **ppDSP)
{
	MLRESULT mlRes = ERR_NOT_SUPPORT;
	if (m_pluginMgr)
		mlRes = m_pluginMgr->GetActiveDSP(ppDSP);
	
	if (mlRes == ERR_NOT_SUPPORT)
	{
		// *ppDSP = CDspSuperEQ::GetInstance();
		// return ERR_OK;

		//*ppDSP = new CDspDemo;
		//(*ppDSP)->AddRef();

		return ERR_FALSE;
	}

	return mlRes;
}


MLRESULT CMPluginManagerAgent::GetActiveVis(IVector *pvVis)
{
	if (m_pluginMgr)
		return m_pluginMgr->GetActiveVis(pvVis);
	else
		return ERR_FALSE;
}

//////////////////////////////////////////////////////////////////////

CMPlayer		*CMPlayer::m_spPlayer = NULL;

OBJ_REFERENCE_IMP(CMPlayer)

CMPlayer::CMPlayer()
{
	OBJ_REFERENCE_INIT;

	m_state = PS_STOPED;

	m_pMDAgent = new CMDAgent;
	m_pMDAgent->Init(this);

	m_pMOAgent = new CMOAgent;
	m_pMOAgent->Init(this);

	m_pMemAllocator = new CMemAllocator;

	m_pMediaLib = new CMediaLibrary;
	m_pMediaLib->Init(this);

	m_bShuffle = false;
	m_bMute = false;
	m_loopMode = MP_LOOP_OFF;
	m_nVolume = -1;
	m_nBalance = 0;

	memset(&m_Equalizer, 0, sizeof(m_Equalizer));

	m_currentPlaylist = new CPlaylist(this);
	m_currentPlaylist->AddRef();

	m_nCurrentMedia = 0;
	m_pCurrentMedia = NULL;
	m_bCurMediaPlayed = false;

	m_bAutoAddToMediaLib = true;

	m_nBufferedVisMs = 0;
	m_nMODelay = 1000;

	srand(GetTickCount());
}

CMPlayer::~CMPlayer()
{
	StopVis();

	{
		m_mutexVisdataAccess.Acquire();
		V_VIS::iterator	it, itEnd;
		itEnd = m_vVis.end();
		for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
		{
			IVis	*pVis = *it;
			pVis->Quit();
			pVis->Release();
		}
		m_mutexVisdataAccess.Release();
	}

	m_mutexDSP.Acquire();
	if (m_pDsp)
	{
		m_pDsp->Quit();
		m_pDsp.Release();
	}
	m_mutexDSP.Release();

	m_pMDAgent.Release();

	m_pMOAgent.Release();

	if (m_pCurrentMedia)
		m_pCurrentMedia->Release();

	if (m_currentPlaylist)
		m_currentPlaylist->Release();

	if (m_pMediaLib)
	{
		m_pMediaLib->Close();
		m_pMediaLib.Release();
	}

	if (m_pMemAllocator)
	{
		m_pMemAllocator->Quit();
		m_pMemAllocator.Release();
	}
}

LPCXSTR CMPlayer::MediaAttributeName(MediaAttribute mediaAttri)
{
	static LPCXSTR		_szMediaAttrName[] = 
	{
		_T("Artist"),
		_T("Album"),
		_T("Title"),
		_T("Track number"),
		_T("Year"),
		_T("Genre"),
		_T("Comment"),
		_T("Length"),
		_T("File size"),
		_T("Time added"),
		_T("Last played"),
		_T("Rating"),
		_T("Auto rating"),
		_T("Times played"),
		_T("Times play skipped"),
		_T("Lyrics file"),
		_T("Format"),
		_T("Is Vbr"),
		_T("Bitrate"),
		_T("BPS"),
		_T("Channels"),
		_T("Sample rate"),
		_T("Extra Info"),
	};

	if (mediaAttri >= 0 && mediaAttri < CountOf(_szMediaAttrName))
		return _szMediaAttrName[mediaAttri];
	else
		return _T("Undefined Name");
}

MLRESULT CMPlayer::SetPluginManager(IMPluginManager *pPluginMgr)
{
	if (m_pluginMgrAgent.m_pluginMgr)
		m_pluginMgrAgent.m_pluginMgr.Release();

	m_pluginMgrAgent.m_pluginMgr = pPluginMgr;
	m_pluginMgrAgent.DetectPlugins();

	//
	// Set DSP
	//
	m_mutexDSP.Acquire();
	if (m_pDsp)
		m_pDsp.Release();
	if (m_pluginMgrAgent.GetActiveDSP(&m_pDsp) == ERR_OK)
		m_pDsp->Init(this);
	m_mutexDSP.Release();

	//
	// Set Vis
	//

	// Stop and unregister all Vis
	StopVis();

	m_mutexVisdataAccess.Acquire();
	V_VIS::iterator	it, itEnd;
	itEnd = m_vVis.end();
	for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
	{
		IVis	*pVis = *it;
		pVis->Quit();
		pVis->Release();
	}
	m_mutexVisdataAccess.Release();

	// Register new vis
	CVector		vVis;
	m_pluginMgrAgent.GetActiveVis(&vVis);
	for (int i = 0; i < (int)vVis.size(); i++)
	{
		IVis	*pVis = (IVis*)vVis.at(i);

		pVis->Init(this);
		RegisterVis(pVis);
	}

	return ERR_OK;
}

MLRESULT CMPlayer::QueryInterface(MPInterfaceType interfaceType, LPVOID *lpInterface)
{
	switch (interfaceType)
	{
	case MPIT_OUTPUT:
		*lpInterface = (IMediaOutput*)m_pMOAgent;
		m_pMOAgent->AddRef();
		break;
	case MPIT_DECODE:
		*lpInterface = (IMediaDecode*)m_pMDAgent;
		m_pMDAgent->AddRef();
		break;
	case MPIT_DSP:
		{
			CMutexAutolock		lock(m_mutexDSP);
			if (m_pDsp)
			{
				*lpInterface = m_pDsp;
				m_pDsp->AddRef();
			}
			else
				return ERR_NOT_FOUND;
		}
		break;
	case MPIT_MEMALLOCATOR:
		*lpInterface = (IMemAllocator*)m_pMemAllocator;
		m_pMemAllocator->AddRef();
		break;
	case MPIT_MEDIA_LIB:
		*lpInterface = (IMediaLibrary*)m_pMediaLib;
		m_pMediaLib->AddRef();
		break;
	default:
		return ERR_NOT_FOUND;
		break;
	}

	return ERR_OK;
}

//
// Player control
//

MLRESULT CMPlayer::Play()
{
	if (m_state == PS_PLAYING)
	{
		return Seek(0);
	}
	else if (m_state == PS_STOPED)
	{
		if (m_pCurrentMedia)
		{
			int		nRet;
			m_bAutoPlayNext = true;
			m_bCurMediaPlayed = true;
			nRet = m_pMDAgent->DoDecode(m_pCurrentMedia);

			if (nRet == ERR_OK)
			{
				m_state = PS_PLAYING;
				NotifyPlayStateChanged();
			}

			return nRet;
			// return DecodeEx(pMedia);
		}
		else
			return ERR_EMPTY_PLAYLIST;
	}
	else
	{
		return Unpause();
	}
}

MLRESULT CMPlayer::Pause()
{
	MLRESULT		ret;
	if (m_state != PS_PLAYING)
		return ERR_OK;

	ret = m_pMDAgent->Pause();
	if (ret == ERR_OK)
	{
		m_state = PS_PAUSED;
		NotifyPlayStateChanged();
	}

	return ret;
}

MLRESULT CMPlayer::Unpause()
{
	MLRESULT		ret;
	if (m_state != PS_PAUSED)
		return ERR_OK;

	ret = m_pMDAgent->Unpause();
	if (ret == ERR_OK)
	{
		m_state = PS_PLAYING;
		NotifyPlayStateChanged();
	}

	return ret;
}

MLRESULT CMPlayer::Stop()
{
	m_bAutoPlayNext = false;

	return DoStop();
}

MLRESULT CMPlayer::Prev()
{
	m_mutexDataAccess.Acquire();
	long			nOld = m_nCurrentMedia;


	if (m_bShuffle)
	{
		if (m_nCurrentShuffleMedia <= 0)
		{
			// in shuffle mode, only can back to 0
			return ERR_END_OF_PLAYLIST;
		}

		ASSERT(m_vShuffleMedia.size() == m_currentPlaylist->GetCount());
		m_nCurrentShuffleMedia--;
		m_nCurrentMedia = m_vShuffleMedia[m_nCurrentShuffleMedia];
		// m_nCurrentMedia = rand() % m_currentPlaylist->GetCount();
	}
	else
	{
		if (m_nCurrentMedia > 0)
			m_nCurrentMedia--;
		else if (m_loopMode == MP_LOOP_ALL)
			m_nCurrentMedia = m_currentPlaylist->GetCount() - 1;
	}

	m_mutexDataAccess.Release();

	if (nOld != m_nCurrentMedia)
	{
		// Notify changed
		CurrentMediaChanged();
	}

	return ERR_OK;
}

MLRESULT CMPlayer::Next()
{
	if (m_state != PS_STOPED)
	{
		// No rating consideration will be done at all if the song is skipped within these seconds. 
		if (GetPos() < 20 * 1000)
			m_pMediaLib->MarkPlaySkipped(m_pCurrentMedia);
	}

	return DoNext(true);
}

MLRESULT CMPlayer::Seek(uint32 dwPos)
{
	if (m_pMDAgent)
	{
		MLRESULT	nRet;
		if ((int)dwPos < -1)
			dwPos = 0;
		nRet = m_pMDAgent->Seek(dwPos);
		if (nRet == ERR_OK)
			NotifySeek();

		return nRet;
	}
	else
		return ERR_PLAYER_INVALID_STATE;
}

MLRESULT CMPlayer::NewMedia(IMedia **ppMedia, LPCXSTR szString)
{
	ASSERT(ppMedia);

	int			nRet;

	nRet = m_pMediaLib->GetMediaByUrl(szString, ppMedia);
	if (nRet != ERR_OK)
	{
		*ppMedia = new CMedia;
		if (*ppMedia == NULL)
			return ERR_NO_MEM;

		(*ppMedia)->AddRef();
		(*ppMedia)->SetSourceUrl(szString);
	}

	return ERR_OK;
}

MLRESULT CMPlayer::NewPlaylist(IPlaylist **ppPlaylist)
{
	*ppPlaylist = new CPlaylist(this);
	if (!*ppPlaylist)
		return ERR_NO_MEM;

	(*ppPlaylist)->AddRef();

	return ERR_OK;
}

MLRESULT CMPlayer::GetMediaLibrary(IMediaLibrary **ppMediaLib)
{
	ASSERT(m_pMediaLib);
	*ppMediaLib = m_pMediaLib;
	m_pMediaLib->AddRef();

	return ERR_OK;
}

MLRESULT CMPlayer::GetCurrentPlaylist(IPlaylist **ppPlaylist)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	m_currentPlaylist->AddRef();
	*ppPlaylist = m_currentPlaylist;

	return ERR_OK;
}

MLRESULT CMPlayer::GetCurrentMedia(IMedia **ppMedia)
{
	if (m_pCurrentMedia)
	{
		m_pCurrentMedia->AddRef();
		*ppMedia = m_pCurrentMedia;

		return ERR_OK;
	}
	else
		return ERR_NOT_FOUND;
	// return m_currentPlaylist->GetItem(m_nCurrentMedia, ppMedia);
}

long CMPlayer::GetCurrentMediaInPlaylist()
{
	return m_nCurrentMedia;
}

MLRESULT CMPlayer::SetCurrentMediaInPlaylist(long nIndex)
{
	m_mutexDataAccess.Acquire();

	if (nIndex >= 0 && nIndex < (int)m_currentPlaylist->GetCount())
	{
		m_nCurrentMedia = nIndex;
		m_mutexDataAccess.Release();
		CurrentMediaChanged();
		return ERR_OK;
	}
	else
	{
		m_mutexDataAccess.Release();
		return ERR_NOT_FOUND;
	}
}

MLRESULT CMPlayer::SetCurrentPlaylist(IPlaylist *pPlaylist)
{
	m_mutexDataAccess.Acquire();
	if (m_currentPlaylist)
		m_currentPlaylist->Release();
	m_currentPlaylist = (CPlaylist *)pPlaylist;
	m_currentPlaylist->AddRef();
	m_nCurrentMedia = 0;
	m_mutexDataAccess.Release();

	GenerateShuffleMediaQueue();

	NotifyCurrentPlaylistChanged(IMPEvent::PCA_FULL_UPDATE, 0, 0);

	CurrentMediaChanged();

	return ERR_OK;
}

MLRESULT CMPlayer::SetCurrentMedia(IMedia *pMedia)
{
	MLRESULT		nRet;

	if (m_state != PS_STOPED)
	{
		nRet = DoStop();
		if (nRet != ERR_OK && m_state != PS_STOPED)
			return nRet;
	}

	m_mutexDataAccess.Acquire();
	if (m_currentPlaylist)
		m_currentPlaylist->Release();

	m_currentPlaylist = new CPlaylist(this);
	m_currentPlaylist->AddRef();
	nRet = m_currentPlaylist->InsertItem(-1, pMedia);
	m_mutexDataAccess.Release();
	if (nRet != ERR_OK)
		return nRet;

	m_nCurrentMedia = 0;

	GenerateShuffleMediaQueue();
	NotifyCurrentPlaylistChanged(IMPEvent::PCA_FULL_UPDATE, 0, 0);

	CurrentMediaChanged();

	return Play();
}

MLRESULT CMPlayer::SetCurrentMedia(LPCXSTR szSourceMedia)
{
	MLRESULT			nRet;
	CMPAutoPtr<IMedia>	pMedia;

	nRet = NewMedia(&pMedia, szSourceMedia);
	if (nRet != ERR_OK)
		return nRet;

	nRet = SetCurrentMedia(pMedia);
	if (m_state != PS_PLAYING)
		Play();

	return nRet;
}

//
// Current playing Media state
//
uint32 CMPlayer::GetLength()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	if (m_pMDAgent)
		return m_pMDAgent->GetLength();
	else
	{
		IMedia		*pMedia;
		if (m_currentPlaylist->GetItem(m_nCurrentMedia, &pMedia) == ERR_OK)
		{
			int			nValue = 0;
			if (pMedia->GetAttribute(MA_DURATION, &nValue) == ERR_OK)
				return nValue;
			else
				return 0;
		}
		else
			return 0;
	}
}

uint32 CMPlayer::GetPos()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	if (m_pMDAgent)
		return m_pMDAgent->GetPos();
	else
		return 0;
}

PLAYER_STATE CMPlayer::GetState()
{
	return m_state;
}

void CMPlayer::OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
	m_mutexDSP.Acquire();
	if (m_pDsp)
		m_pDsp->Process(pBuf, nBps, nChannels, nSampleRate);
	m_mutexDSP.Release();

	if (IsVisActive())
		AddVisData(pBuf, nBps, nChannels, nSampleRate);

	m_pMOAgent->Write(pBuf);
}
//
// Player settings
//
void CMPlayer::SetShuffle(bool bShuffle)
{
	m_bShuffle = bShuffle;
	NotifySettingsChanged(IMPEvent::MPS_SHUFFLE, m_bShuffle);
}

void CMPlayer::SetLoop(MP_LOOP_MODE loopMode)
{
	m_loopMode = loopMode;
	NotifySettingsChanged(IMPEvent::MPS_LOOP, loopMode);
}

bool CMPlayer::GetShuffle()
{
	return m_bShuffle;
}

MP_LOOP_MODE CMPlayer::GetLoop()
{
	return m_loopMode;
}

void CMPlayer::SetMute(bool bMute)
{
	m_bMute = bMute;

	if (m_pMDAgent)
	{
		if (bMute)
			m_pMDAgent->SetVolume(0, 0);
		else if (m_nVolume != -1)
			m_pMDAgent->SetVolume(m_nVolume, m_nBalance);
	}

	NotifySettingsChanged(IMPEvent::MPS_MUTE, m_bMute);
}

bool CMPlayer::GetMute()
{
	return m_bMute;
}

// 0 ~ 100
MLRESULT CMPlayer::SetVolume(long nVolume)
{
	ASSERT(nVolume >= 0 && nVolume <= 100);
	if (nVolume < 0)
		nVolume = 0;
	else if (nVolume > 100)
		nVolume = 100;
	m_nVolume = nVolume;
	NotifySettingsChanged(IMPEvent::MPS_VOLUME, m_nVolume);
	if (m_pMDAgent)
		return m_pMDAgent->SetVolume(nVolume, m_nBalance);
	else
		return ERR_OK;
}

long CMPlayer::GetVolume()
{
	return m_nVolume;
}

// -100 ~ 100
MLRESULT CMPlayer::SetBalance(long nBalance)
{
	ASSERT(nBalance >= -100 && nBalance <= 100);
	if (nBalance < -100)
		nBalance = -100;
	else if (nBalance > 100)
		nBalance = 100;
	m_nBalance = nBalance;
	NotifySettingsChanged(IMPEvent::MPS_BALANCE, m_nBalance);
	if (m_pMDAgent && m_nVolume != -1)
		return m_pMDAgent->SetVolume(m_nVolume, nBalance);
	else
		return ERR_OK;
}

long CMPlayer::GetBalance()
{
	return m_nBalance;
}

MLRESULT CMPlayer::SetEQ(const EQualizer *eq)
{
	m_Equalizer = *eq;
	NotifyEQSettingsChanged(&m_Equalizer);
	return ERR_OK;
}

MLRESULT CMPlayer::GetEQ(EQualizer *eq)
{
	*eq = m_Equalizer;
	return ERR_OK;
}

//
// Event tracer
//
void CMPlayer::RegisterEvent(IMPEvent *pEventHandler)
{
	CMutexAutolock	autolock(m_mutexEventHandler);

	pEventHandler->AddRef();
	m_listEventHandler.push_back(pEventHandler);
}

void CMPlayer::UnregisterEvent(IMPEvent *pEventHandler)
{
	CMutexAutolock	autolock(m_mutexEventHandler);

	list<IMPEvent *>::iterator it, itEnd;
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		if (pEventHandler == *it)
		{
			m_listEventHandler.erase(it);
			pEventHandler->Release();
			return;
		}
	}
}

void CMPlayer::NotifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
	m_state = PS_STOPED;

	m_pMDAgent->NotifyEod(pDecoder, nError);

	if (m_bAutoPlayNext)
	{
		if (m_loopMode == MP_LOOP_TRACK)
		{
			Play();
			return;
		}

		MLRESULT	nRet;
		nRet = DoNext();
		if (nRet == ERR_OK)
		{
			nRet = Play();
			if (nRet == ERR_MI_OPEN_SRC && nError == ERR_OK)
			{
				// if (ERR_OPEN_FILE && last_file_can be played)
				//		try the other medias.
				int			i, nTryMax;
				nTryMax = m_currentPlaylist->GetCount();
				for (i = 1; m_bAutoPlayNext && i < nTryMax 
					&& nRet == ERR_MI_OPEN_SRC && DoNext() == ERR_OK; i++)
				{
					nRet = Play();
					Sleep(500);
				}
			}
		}
		else
		{
			if (m_bCurMediaPlayed)
				m_pMediaLib->MarkPlayFinished(m_pCurrentMedia);
		}
	}

	NotifyPlayStateChanged();
}


MLRESULT CMPlayer::MakeOutputReadyForDecode()
{
	MLRESULT		nRet;

	if (!m_pMOAgent->m_pMediaOutput)
	{
		nRet = m_pluginMgrAgent.NewOutput(&(m_pMOAgent->m_pMediaOutput));
		if (nRet != ERR_OK)
			return nRet;

		m_pMOAgent->m_pMediaOutput->Init(this);
	}

	return ERR_OK;
}

MLRESULT CMPlayer::LoadMediaTagInfo(IMedia *pMedia, bool bForceReload)
{
	MLRESULT				nRet;

// 	if (pMedia->GetAttributeCount() > 0 && !bForceReload)
// 		return ERR_OK;

	CXStr		str;

	nRet = pMedia->GetSourceUrl(&str);
	if (nRet != ERR_OK)
		return ERR_OK;

	CMPAutoPtr<IMediaDecode>	pDecode;
	CMPAutoPtr<IMediaInput>		pInput;

	nRet = m_pluginMgrAgent.NewInput(str.c_str(), &pInput);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pluginMgrAgent.NewDecoder(pInput, &pDecode);
	if (nRet != ERR_OK)
		return nRet;

	return pDecode->GetMediaInfo(this, pInput, pMedia);
}

// Vis:
MLRESULT CMPlayer::RegisterVis(IVis *pVis)
{
	CMutexAutolock	autolock(m_mutexVisdataAccess);
	V_VIS::iterator	it, itEnd;

	itEnd = m_vVis.end();
	for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
	{
		if (*it == pVis)
			return ERR_EXIST;
	}

	pVis->AddRef();
	m_vVis.push_back(pVis);

	if (m_vVis.size() == 1)
	{
		autolock.Unlock();
		StartVis();
	}

	return ERR_OK;
}

MLRESULT CMPlayer::UnregisterVis(IVis *pVis)
{
	CMutexAutolock	autolock(m_mutexVisdataAccess);
	V_VIS::iterator	it, itEnd;

	itEnd = m_vVis.end();
	for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
	{
		if (*it == pVis)
		{
			pVis->Quit();
			pVis->Release();
			m_vVis.erase(it);
			if (m_vVis.empty())
			{
				autolock.Unlock();
				StopVis();
			}
			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}

// Dsp:
MLRESULT CMPlayer::RegisterDsp(IDSP *pVis)
{
	return ERR_OK;
}

MLRESULT CMPlayer::UnregisterDsp(IDSP *pVis)
{
	return ERR_OK;
}

MLRESULT CMPlayer::DoStop()
{
	if (m_state == PS_STOPED)
		return ERR_OK;

	// use a temp MediaDecode with AddRef for thread safe
	return m_pMDAgent->Stop();
}

MLRESULT CMPlayer::DoNext(bool bLoop)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	if (m_currentPlaylist->GetCount() == 0)
		return ERR_EMPTY_PLAYLIST;

	long			nOld = m_nCurrentMedia;

	if (m_bShuffle)
	{
		if (m_nCurrentShuffleMedia >= (int)m_vShuffleMedia.size() - 1)
		{
			if (m_loopMode != MP_LOOP_ALL && !bLoop)
				return ERR_END_OF_PLAYLIST;

			// all media has been played, regenerate radom playlist.
			GenerateShuffleMediaQueue();
		}

		ASSERT(m_vShuffleMedia.size() == m_currentPlaylist->GetCount());
		m_nCurrentMedia = m_vShuffleMedia[m_nCurrentShuffleMedia];
		m_nCurrentShuffleMedia++;
		// m_nCurrentMedia = rand() % m_currentPlaylist->GetCount();
	}
	else
	{
		if (m_nCurrentMedia < (int)m_currentPlaylist->GetCount() - 1)
			m_nCurrentMedia++;
		else if (m_loopMode != MP_LOOP_ALL && !bLoop)
			return ERR_END_OF_PLAYLIST;
		else
			m_nCurrentMedia = 0;
	}

	autolock.Unlock();

	if (nOld != m_nCurrentMedia)
	{
		// Notify changed
		CurrentMediaChanged();
	}
	return ERR_OK;
}

void CMPlayer::CurrentMediaChanged()
{
	list<IMPEvent *>::iterator	it, itEnd;
	MLRESULT				nRet;
	bool					bPlay = false;

	if (m_state != PS_STOPED)
	{
		if (m_state == PS_PLAYING)
			bPlay = true;
		m_bAutoPlayNext = false;
		nRet = DoStop();
		if (nRet != ERR_OK && m_state != PS_STOPED)
			return;
	}

	m_mutexDataAccess.Acquire();

	if (m_pCurrentMedia)
	{
		if (m_bAutoPlayNext && m_bCurMediaPlayed)
			m_pMediaLib->MarkPlayFinished(m_pCurrentMedia);

		m_pCurrentMedia->Release();
		m_pCurrentMedia = NULL;
		m_bCurMediaPlayed = false;
	}

	if (m_currentPlaylist->GetCount() > 0)
	{
		nRet = m_currentPlaylist->GetItem(m_nCurrentMedia, &m_pCurrentMedia);
		if (nRet == ERR_OK)
		{
			nRet = LoadMediaTagInfo(m_pCurrentMedia, false);
		}
	}

	m_mutexDataAccess.Release();

	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnCurrentMediaChanged();
	}
	m_mutexEventHandler.Release();

	if (bPlay)
		Play();
}

MLRESULT CMPlayer::GetInstance(IMPlayer **ppPlayer)
{
	if (!m_spPlayer)
	{
		m_spPlayer = new CMPlayer;
	}
	m_spPlayer->AddRef();

	*ppPlayer = m_spPlayer;

	return ERR_OK;
}

MLRESULT CMPlayer::QuitInstance(IMPlayer **ppPlayer)
{
	*ppPlayer = NULL;

	if (m_spPlayer)
	{
		delete m_spPlayer;
		m_spPlayer = NULL;
	}

	return ERR_OK;
}

void CMPlayer::NotifyPlaylistChanged(CPlaylist *playlist, IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld)
{
	if (m_currentPlaylist == playlist)
	{
		// first notify the message
		NotifyCurrentPlaylistChanged(action, nIndex, nIndexOld);

		//
		// is current media changed?
		//
		CMPAutoPtr<IMedia>	pMedia;
		long				nNewCurrentIndex;
		CMutexAutolock		autolock(m_mutexDataAccess);

		if (m_nCurrentMedia >= (int)m_currentPlaylist->GetCount())
			m_nCurrentMedia = 0;

		GenerateShuffleMediaQueue();

		if (m_currentPlaylist->GetCount() == 0)
		{
			autolock.Unlock();
			if (m_pCurrentMedia)
				CurrentMediaChanged();
			return;
		}

		// is current media?
		if (m_currentPlaylist->GetItem(m_nCurrentMedia, &pMedia) == ERR_OK)
		{
			if (m_pCurrentMedia == pMedia)
				return;
		}

		// Get new index of current media.
		if (m_currentPlaylist->GetItemIndex(m_pCurrentMedia, nNewCurrentIndex) == ERR_OK)
		{
			m_nCurrentMedia = nNewCurrentIndex;

			// unlock before notify
			autolock.Unlock();

			list<IMPEvent *>::iterator	it, itEnd;

			m_mutexEventHandler.Acquire();
			itEnd = m_listEventHandler.end();
			for (it = m_listEventHandler.begin(); it != itEnd; ++it)
			{
				IMPEvent	*pEventHandler = *it;
				pEventHandler->OnCurrentMediaChanged();
			}
			m_mutexEventHandler.Release();
			return;
		}

		// unlock before CurrentMediaChanged
		autolock.Unlock();

		CurrentMediaChanged();
	}
}

void CMPlayer::NotifyPlayStateChanged()
{
	list<IMPEvent *>::iterator	it, itEnd;

	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnStateChanged(m_state);
	}
	m_mutexEventHandler.Release();
}

void CMPlayer::NotifyCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld)
{
	list<IMPEvent *>::iterator	it, itEnd;

	// notify current playlist changed
	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnCurrentPlaylistChanged(action, nIndex, nIndexOld);
	}
	m_mutexEventHandler.Release();
}

void CMPlayer::NotifySettingsChanged(IMPEvent::MP_SETTING_TYPE settingType, long nValue)
{
	list<IMPEvent *>::iterator	it, itEnd;

	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnSettingChanged(settingType, nValue);
	}
	m_mutexEventHandler.Release();
}

void CMPlayer::NotifyEQSettingsChanged(const EQualizer *peq)
{
	list<IMPEvent *>::iterator	it, itEnd;

	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnEQSettingChanged(peq);
	}
	m_mutexEventHandler.Release();
}

void CMPlayer::NotifySeek()
{
	list<IMPEvent *>::iterator	it, itEnd;

	// notify current playlist changed
	m_mutexEventHandler.Acquire();
	itEnd = m_listEventHandler.end();
	for (it = m_listEventHandler.begin(); it != itEnd; ++it)
	{
		IMPEvent	*pEventHandler = *it;
		pEventHandler->OnSeek();
	}
	m_mutexEventHandler.Release();
}

// For internal using, do NOT lock.
void CMPlayer::GenerateShuffleMediaQueue()
{
	long			nCount;

	nCount = m_currentPlaylist->GetCount();
	m_nCurrentShuffleMedia = 0;

	vector<long>	vMediaSequence;
	long			i, nNext;

	for (i = 0; i < nCount; i++)
		vMediaSequence.push_back(i);

	m_vShuffleMedia.clear();
	for (i = 0; i < nCount; i++)
	{
		nNext = rand() % vMediaSequence.size();
		m_vShuffleMedia.push_back(vMediaSequence[nNext]);
		vMediaSequence.erase(vMediaSequence.begin() + nNext);
	}
}

MLRESULT CMPlayer::StartVis()
{
	m_bQuitVis = false;
	if (m_threadVis.Create(ThreadVisProc, this))
		return ERR_OK;
	else
		return ERR_FALSE;
}

MLRESULT CMPlayer::StopVis()
{
	m_bQuitVis = true;
	m_threadVis.Join();

	CMutexAutolock autolock(m_mutexVisdataAccess);

	while (!m_queVisDataPre.empty())
	{
		VisDataPre	&temp = m_queVisDataPre.front();
		temp.pBuf->Release();
		m_queVisDataPre.pop_front();
	}

	m_nBufferedVisMs = 0;

	return ERR_OK;
}

void CMPlayer::CalcVisParam(VisParam &param)
{
	int					nSamples;

	// get pcm data
	m_mutexVisdataAccess.Acquire();
	if (m_queVisDataPre.size() == 0)
	{
		m_mutexVisdataAccess.Release();
		return;
	}
	VisDataPre	temp = m_queVisDataPre.front();
	m_queVisDataPre.pop_front();
	m_mutexVisdataAccess.Release();

	m_nBufferedVisMs -= temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels / temp.nSampleRate;

	nSamples = temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels;
	if (nSamples >= VIS_N_WAVE_SAMPLE)
		nSamples = VIS_N_WAVE_SAMPLE;
	param.nChannels = temp.nChannels;

	//
	// clear not setted data 
	//
	if (temp.nChannels == 2)
	{
		for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++)
		{
			param.waveformData[0][i] = 0;
			param.waveformData[1][i] = 0;
		}
	}
	else
	{
		for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++)
		{
			param.waveformData[0][i] = 0;
		}
	}

	// analysis pcm data
	if (temp.nBps == 16)
	{
		// 16 bps wave data
		const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

		int		k = 0;
		if (temp.nChannels == 2)
		{
			// stereo
			for (int i = 0; i < nSamples; i++)
			{
				param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
				param.waveformData[1][i] = (unsigned char)(buf[k++] / 0xFF);
			}
			calc_freq(param.spectrumData[0], param.waveformData[0]);
			calc_freq(param.spectrumData[1], param.waveformData[1]);
		}
		else
		{
			// mono
			for (int i = 0; i < nSamples; i++)
			{
				param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
			}
			calc_freq(param.spectrumData[0], param.waveformData[0]);
		}
	}
	else if (temp.nBps == 8)
	{
		const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

		int		k = 0;
		if (temp.nChannels == 2)
		{
			// stereo
			for (int i = 0; i < nSamples; i++)
			{
				param.waveformData[0][i] = (unsigned char)(buf[k++]);
				param.waveformData[1][i] = (unsigned char)(buf[k++]);
			}
			calc_freq(param.spectrumData[0], param.waveformData[0]);
			calc_freq(param.spectrumData[1], param.waveformData[1]);
		}
		else
		{
			// mono
			for (int i = 0; i < nSamples; i++)
			{
				param.waveformData[0][i] = (unsigned char)(buf[k++]);
			}
			calc_freq(param.spectrumData[0], param.waveformData[0]);
		}
	}

	temp.pBuf->Release();
}

void CMPlayer::ThreadVisProc(LPVOID lpData)
{
	CMPlayer	*pPlayer = (CMPlayer *)lpData;

	pPlayer->ThreadVis();
}

void CMPlayer::ThreadVis()
{
	static VisParam		param;

	while (!m_bQuitVis)
	{
		Sleep(30);

		if (m_state == PS_PLAYING)
		{
			CalcVisParam(param);
		}
		else if (m_state == PS_STOPED)
		{
			memset(param.spectrumData, 0, sizeof(param.spectrumData));
			memset(param.waveformData, 0, sizeof(param.waveformData));
		}

		// call vis to render
		V_VIS::iterator	it, itEnd;

		m_mutexVisdataAccess.Acquire();
		itEnd = m_vVis.end();
		for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
		{
			IVis	*pVis = *it;
			pVis->Render(&param);
		}
		m_mutexVisdataAccess.Release();
	}
}

bool CMPlayer::IsVisActive()
{
	return !m_vVis.empty() && !m_bQuitVis;
}

void CMPlayer::AddVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
	VisDataPre		data;

	data.pBuf = pBuf;
	data.pBuf->AddRef();
	data.nBps = nBps;
	data.nChannels = nChannels;
	data.nPlayingPos = 0;
	data.nSampleRate = nSampleRate;

	CMutexAutolock	autolock(m_mutexVisdataAccess);

	if (m_nBufferedVisMs > m_nMODelay && m_queVisDataPre.size() >= 9 || m_queVisDataPre.size() > 40)
	{
		VisDataPre	&temp = m_queVisDataPre.front();
		m_nBufferedVisMs -= temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels / temp.nSampleRate;
		temp.pBuf->Release();
		m_queVisDataPre.pop_front();
	}

	m_nBufferedVisMs += pBuf->size() / (nBps / 8) / nChannels / nSampleRate;

	m_queVisDataPre.push_back(data);
}
