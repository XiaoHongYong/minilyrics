// MOSoundCard.cpp: implementation of the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMPlayer.h"
#include "MOSoundCard.h"

// Optimized for pocketpc.

CMOSoundCard::CMOSoundCard() : m_eventCanWrite(false, true)
{
	OBJ_REFERENCE_INIT;

	m_dwTotolBytesOffset = 0;

	m_fadeMode = FADE_NONE;
	m_bWrittingPausedData = false;
}

CMOSoundCard::~CMOSoundCard()
{
}

OBJ_REFERENCE_IMP(CMOSoundCard)

LPCXSTR CMOSoundCard::GetDescription()
{
	return _T("MPlayer soundcard output 1.0");
}

MLRESULT CMOSoundCard::Open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
	DBGLOG1(_T("Open: %d"), m_nBuffered);

	m_fadeMode = FADE_NONE;

	m_nChannels = nNumChannels;
	m_nSamplerate = nSampleRate;
	m_nBitsPerSamp = nBitsPerSamp;

	return ERR_OK;
}

MLRESULT CMOSoundCard::WaitForWrite()
{
	if (!m_eventCanWrite.Acquire())
		return ERR_FALSE;

	m_eventCanWrite.Set();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Write(IFBuffer *pBuf)
{
	m_eventCanWrite.Acquire();

	while (m_bWrittingPausedData)
		Sleep(20);

	m_mutex.Acquire();

	m_mutex.Release();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Flush()
{
	m_eventCanWrite.Set();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Pause(bool bPause)
{
	return ERR_OK;
}

bool CMOSoundCard::IsPlaying()
{
	return false;
}

MLRESULT CMOSoundCard::Stop()
{	return ERR_OK;
}

bool CMOSoundCard::IsOpened()
{
	return false;
}

// volume
MLRESULT CMOSoundCard::SetVolume(int nVolume, int nBanlance)
{
	return ERR_FALSE;
}

#define MAXINT32 0x7FFFFFFF

uint32 CMOSoundCard::GetPos()
{
	return 0;
}

