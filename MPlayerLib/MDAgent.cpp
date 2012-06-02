// MDAgent.cpp: implementation of the CMDAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayer.h"
#include "MDAgent.h"
#include "MPTime.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDAgent::CMDAgent()
{
	OBJ_REFERENCE_INIT
}

CMDAgent::~CMDAgent()
{

}

LPCXSTR CMDAgent::GetDescription()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->GetDescription();
	else
		return _T("MDAgen: no decoder was specified.");
}

LPCXSTR CMDAgent::GetFileExtentions()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->GetFileExtentions();
	else
		return _T("MDAgen: no decoder was specified.");
}

MLRESULT CMDAgent::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	MLRESULT				nRet;

	CMPAutoPtr<IMediaDecode>	pDecode;
	nRet = m_pPlayer->m_pluginMgrAgent.NewDecoder(pInput, &pDecode);
	if (nRet != ERR_OK)
		return nRet;

	nRet = pDecode->GetMediaInfo(m_pPlayer, pInput, pMedia);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}


//
// decode media file related methods
//

bool CMDAgent::IsSeekable()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->IsSeekable();
	else
		return false;
}

bool CMDAgent::IsUseOutputPlug()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->IsUseOutputPlug();
	else
		return false;
}

//	ERR_PLAYER_INVALID_FILE:	decoder can't decode the file
//  ERR_DECODER_INNER_ERROR:	inner error occurs at decoder
//  ERR_DECODER_UNSUPPORTED_FEATURE:
//  ERR_DECODER_INIT_FAILED:
MLRESULT CMDAgent::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->Play(pPlayer, pInput);
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::Pause()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->Pause();
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::Unpause()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->Unpause();
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::Stop()
{
	MLRESULT	nRet;

	IMediaDecode *pMediaDecode = m_pMediaDecode;
	pMediaDecode->AddRef();
	nRet = pMediaDecode->Stop();
	pMediaDecode->Release();

	return nRet;
}
// media length, pos related functions, unit: ms
uint32 CMDAgent::GetLength()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->GetLength();
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::Seek(uint32 nPos)
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->Seek(nPos);
	else
		return ERR_NO_DEVICE;
}

uint32 CMDAgent::GetPos()
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->GetPos();
	else
		return 0;
}

// volume
MLRESULT CMDAgent::SetVolume(int nVolume, int nBanlance)
{
	if (m_pMediaDecode.p)
		return m_pMediaDecode->SetVolume(nVolume, nBanlance);
	else
		return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::Init(CMPlayer *pPlayer)
{
	m_pPlayer = pPlayer;
	return ERR_OK;
}

MLRESULT CMDAgent::DoDecode(IMedia *pMedia)
{
	MLRESULT					nRet;
	CXStr						strMedia;
	CMPAutoPtr<IMediaDecode>	pDecoder;
	CMPAutoPtr<IMediaInput>		pInput;
	CMPTime		timePlayed;

	nRet = pMedia->GetSourceUrl(&strMedia);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pPlayer->m_pluginMgrAgent.NewInput(strMedia.c_str(), &pInput);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pPlayer->m_pluginMgrAgent.NewDecoder(pInput, &pDecoder);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pPlayer->MakeOutputReadyForDecode();
	if (nRet != ERR_OK)
		return nRet;

	m_pMediaInput = pInput;
	m_pMediaDecode = pDecoder;
	m_pMedia = pMedia;

	m_pMediaDecode->GetMediaInfo(m_pPlayer, m_pMediaInput, m_pMedia);

	nRet = m_pMediaDecode->Play(m_pPlayer, m_pMediaInput);
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
			m_pMediaDecode->SetVolume(0, 0);
		else
			m_pMediaDecode->SetVolume(m_pPlayer->m_nVolume, m_pPlayer->m_nBalance);
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
	m_pMediaInput.Release();

	return nRet;
}

void CMDAgent::NotifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
	if (m_pMediaDecode.p == pDecoder)
		m_pMediaDecode.Release();
}
