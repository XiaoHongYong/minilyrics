#include "StdAfx.h"
#include "MDLrc.h"

CMDLrc::CMDLrc(void)
{
}

CMDLrc::~CMDLrc(void)
{
}

LPCXSTR CMDLrc::GetDescription()
{

}

LPCXSTR CMDLrc::GetFileExtentions()
{

}

MLRESULT CMDLrc::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	CMLData data;
	int nRet = data.OpenLyrics(pInput->GetSource(), 0, pInput->GetSource());

	m_nLength = data.Properties().GetMediaLengthInt();

	return nRet;
}

//
// decode media file related methods
//

bool CMDLrc::IsSeekable()
{
	return true;
}

bool CMDLrc::IsUseOutputPlug()
{
	return false;
}

// ERR_PLAYER_INVALID_FILE: decoder can't decode the file
// ERR_DECODER_INNER_ERROR: inner error occurs at decoder
// ERR_DECODER_UNSUPPORTED_FEATURE:
// ERR_DECODER_INIT_FAILED:
MLRESULT CMDLrc::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	m_dwBeginTime = GetTickCount();
}

MLRESULT CMDLrc::Pause()
{
	if (m_state == PS_PLAYING)
	{
		m_state = PS_PAUSED;
		m_nPausedPos = GetTickCount() - m_dwBeginTime;
	}

	return ERR_OK;
}

MLRESULT CMDLrc::Unpause()
{
	if (m_state == PS_PAUSED)
	{
		m_state = PS_PLAYING;
		m_dwBeginTime = GetTickCount() - m_nPausedPos;
	}

	return ERR_OK;
}

MLRESULT CMDLrc::Stop()
{
	m_dwBeginTime = 0;
	m_nPausedPos = 0;
	m_state = PS_STOPED;
}

// media length, pos related functions, unit: ms
uint32 CMDLrc::GetLength()
{
	return m_nLength;
}

MLRESULT CMDLrc::Seek(uint32 nPos)
{
	if (m_state == PS_STOPED)
		return ERR_OK;

	if (nPos < 0)
		nPos = 0;
	else if (nPos > m_nLength)
		nPos = m_nLength;

	m_dwBeginTime = GetTickCount() - nPos;
	m_nPausedPos = nPos;

	return ERR_OK;
}

uint32 CMDLrc::GetPos()
{
	return GetTickCount() - m_dwBeginTime;
}

// volume
MLRESULT CMDLrc::SetVolume(int nVolume, int nBanlance)
{
	return ERR_OK;
}

bool CMDLrc::Init()
{
	return true;
}

MLRESULT CMDLrc::Init(CMPlayer *pPlayer)
{
	return CMDAgent::Init(pPlayer);
}

MLRESULT CMDLrc::DoDecode(IMedia *pMedia)
{
	MLRESULT					nRet;
	CXStr						strMedia;

	nRet = pMedia->GetSourceUrl(&strMedia);
	if (nRet != ERR_OK)
		return nRet;

	CMLData data;
	int nRet = data.OpenLyrics(strMedia.c_str(), 0, strMedia.c_str());

	m_nLength = data.Properties().GetMediaLengthInt();
	m_dwBeginTime = GetTickCount();
	m_state = PS_PLAYING;
	m_nPausedPos = 0;

	return ERR_OK;
}

void CMDLrc::NotifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
	m_state = PS_STOPED;
}

bool CMDLrc::IsOK()
{
	return true;
}
