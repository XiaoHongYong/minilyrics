// MOSoundCard.cpp: implementation of the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMPlayer.h"
#include "MOSoundCard.h"

// Optimized for pocketpc.
bool		g_bGaplessOutput = true;
#define		FADE_DURATION		500 / 1000	// ms
#define		DOUBLECONVT			int
// #define		DOUBLECONVT		double

void DBGMMError(MMRESULT nRet)
{
	TCHAR szError[256];
	if (MMSYSERR_NOERROR == waveOutGetErrorText(nRet, szError, CountOf(szError)))
	{
		DBGLOG0(szError);
	}
	else
	{
		DBGLOG0(_T("unknown"));
	}
}

void CALLBACK
CMOSoundCard::MCICallBack(HWAVEOUT hwo, UINT msg, DWORD dwInstance,
            DWORD dwParam1, DWORD dwParam2)
{
	switch (msg)
	{
	case WOM_DONE:
		{
			LPWAVEHDR		lpWaveHdr = (LPWAVEHDR) dwParam1;
			CMOSoundCard	*pSoundCard= (CMOSoundCard *)dwInstance;
			IFBuffer		*pBuf = (IFBuffer*)lpWaveHdr->dwUser;
			pBuf->Release();
			pSoundCard->m_dwTotolBytesOffset += lpWaveHdr->dwBufferLength;

			if (pSoundCard->m_fadeMode == FADE_OUT && pSoundCard->m_pFadeOutEndHdr == lpWaveHdr)
			{
				waveOutPause(hwo);
				pSoundCard->m_pFadeOutEndHdr = NULL;
			}

			pSoundCard->m_mutex.Acquire();
			pSoundCard->m_listFree.push_back(lpWaveHdr);
			pSoundCard->m_nBuffered -= lpWaveHdr->dwBufferLength;
			if (pSoundCard->m_nBuffered < pSoundCard->m_nBufferedMax)
				pSoundCard->m_eventCanWrite.Set();
			// DBGLOG1(_T("Mci callback: %d"), pSoundCard->m_nBuffered);
			// if (pSoundCard->m_nBuffered <= 1024 * 10)
			{
			//	ERRLOG1(_T("Buffered: %d"), pSoundCard->m_nBuffered);
			}
			ASSERT(pSoundCard->m_listPrepared.front() == lpWaveHdr);
			pSoundCard->m_listPrepared.pop_front();
			pSoundCard->m_mutex.Release();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOSoundCard::CMOSoundCard() : m_eventCanWrite(false, true)
{
	OBJ_REFERENCE_INIT;

	m_dwTotolBytesOffset = 0;

	m_hwo = NULL;

	m_fadeMode = FADE_NONE;
	m_pFadeOutEndHdr = NULL;
	m_bWrittingPausedData = false;
}

CMOSoundCard::~CMOSoundCard()
{
	if (m_hwo)
	{
		DoStop();
	}
}

LPCXSTR CMOSoundCard::GetDescription()
{
	return _T("MPlayer soundcard output 1.0");
}

MLRESULT CMOSoundCard::Open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
	DBGLOG1(_T("Open: %d"), m_nBuffered);

	m_fadeMode = FADE_NONE;

	if (m_hwo && g_bGaplessOutput)
	{
		if (m_nChannels != nNumChannels || nSampleRate != m_nSamplerate
			|| m_nBitsPerSamp != nBitsPerSamp)
			DoStop();
		else
		{
			m_eventCanWrite.Set();
			return ERR_OK;
		}
	}
	assert(m_hwo == NULL);

	m_nChannels = nNumChannels;
	m_nSamplerate = nSampleRate;
	m_nBitsPerSamp = nBitsPerSamp;

	return DoOpen();
}

WAVEHDR* CMOSoundCard::NewWaveHdr()
{
	WAVEHDR		*waveHdr;
	waveHdr = new WAVEHDR;
	memset(waveHdr, 0, sizeof(WAVEHDR));

	return waveHdr;
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
	assert(m_hwo);

	WAVEHDR		*pWaveHdr;
	MMRESULT	result;

	m_eventCanWrite.Acquire();

	while (m_bWrittingPausedData)
		Sleep(20);

	m_mutex.Acquire();

	// get a free WaveHdr
	if (m_listFree.size())
	{
		pWaveHdr = m_listFree.front();
		m_listFree.pop_front();
	}
	else
		pWaveHdr = NewWaveHdr();

	if (m_fadeMode == FADE_OUT && m_nFadeDoneSamples == 0)
		m_listPausedHdr.push_back(pWaveHdr);
	else
		m_listPrepared.push_back(pWaveHdr);
	m_nBuffered += pBuf->size();
	if (m_nBuffered < m_nBufferedMax)
		m_eventCanWrite.Set();

	m_mutex.Release();

	if (pWaveHdr->dwFlags & WHDR_PREPARED)
	{
		waveOutUnprepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
		pWaveHdr->dwFlags = 0;
	}
	pWaveHdr->lpData = pBuf->data();
	pWaveHdr->dwBufferLength = pBuf->size();
	pWaveHdr->dwUser = (DWORD)pBuf;

	if (m_fadeMode == FADE_IN)
	{
		// fade in sound data.
		FadeInSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
		if (m_nFadeDoneSamples == m_nFadeMaxSamples)
		{
			m_fadeMode = FADE_NONE;
		}
	}
	else if (m_fadeMode == FADE_OUT)
	{
		if (m_nFadeDoneSamples != 0)
		{
			FadeOutSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
			if (m_nFadeDoneSamples == 0)
				m_pFadeOutEndHdr = pWaveHdr;
		}
		else
		{
			// alread added data to m_listPausedHdr
			return ERR_OK;
		}
	}

	result = waveOutPrepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
	if (result != MMSYSERR_NOERROR)
	{
		DBGMMError(result);
		return ERR_SOUND_DEVICE_WRITE;
	}

	result = waveOutWrite(m_hwo, pWaveHdr, sizeof(WAVEHDR));
	if (result != MMSYSERR_NOERROR)
	{
		DBGMMError(result);
		return ERR_SOUND_DEVICE_WRITE;
	}

	return ERR_OK;
}

MLRESULT CMOSoundCard::Flush()
{
	LIST_WAVHDR::iterator	it, itEnd;

	m_mutex.Acquire();
	itEnd = m_listPausedHdr.end();
	for(it = m_listPausedHdr.begin(); it != itEnd; ++it)
	{
		WAVEHDR		*pWaveHdr = *it;
		IFBuffer	*pBuf = (IFBuffer*)pWaveHdr->dwUser;
		pBuf->Release();
		m_nBuffered -= pWaveHdr->dwBufferLength;
		delete pWaveHdr;
	}
	m_listPausedHdr.clear();
	m_mutex.Release();

	waveOutReset(m_hwo);

	m_eventCanWrite.Set();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Pause(bool bPause)
{
	assert(m_hwo);

	if (bPause)
	{
		if (m_fadeMode != FADE_OUT)
		{
			// fade out sound data.
			m_fadeMode = FADE_OUT;
			m_nFadeMaxSamples = m_nSamplerate * FADE_DURATION;	// fade time 0.5 sec
			m_nFadeDoneSamples = m_nFadeMaxSamples;
			m_pFadeOutEndHdr = NULL;

			m_mutex.Acquire();
			LIST_WAVHDR::iterator	it, itEnd;
			itEnd = m_listPrepared.end();
			for(it = m_listPrepared.begin(); it != itEnd; ++it)
			{
				WAVEHDR		*pWaveHdr = *it;
				ASSERT(pWaveHdr->dwFlags & WHDR_PREPARED);

				FadeOutSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
				if (m_nFadeDoneSamples == m_nFadeMaxSamples)
				{
					m_pFadeOutEndHdr = pWaveHdr;
					break;
				}
			}
			m_mutex.Release();
		}
	}
	else
	{
		if (m_fadeMode != FADE_IN)
		{
			LIST_WAVHDR::iterator	it, itEnd;
			LIST_WAVHDR				listTemp;

			// fade in sound data.
			m_fadeMode = FADE_IN;
			m_nFadeDoneSamples = 0;
			m_nFadeMaxSamples = m_nSamplerate * FADE_DURATION;	// fade time 0.5 sec

			m_bWrittingPausedData = true;

			m_mutex.Acquire();
			itEnd = m_listPausedHdr.end();
			for (it = m_listPausedHdr.begin(); it != itEnd; ++it)
			{
				WAVEHDR		*pWaveHdr = *it;
				m_listPrepared.push_back(pWaveHdr);
			}
			listTemp = m_listPausedHdr;
			m_listPausedHdr.clear();
			m_mutex.Release();

			itEnd = listTemp.end();
			for (it = listTemp.begin(); it != itEnd; ++it)
			{
				WAVEHDR		*pWaveHdr = *it;
				MMRESULT	result;

				if (m_nFadeDoneSamples != m_nFadeMaxSamples)
				{
					FadeInSoundData(pWaveHdr->lpData, pWaveHdr->dwBufferLength);
					if (m_nFadeDoneSamples == m_nFadeMaxSamples)
						m_fadeMode = FADE_NONE;
				}

				result = waveOutPrepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
				if (result != MMSYSERR_NOERROR)
				{
					DBGMMError(result);
				}

				result = waveOutWrite(m_hwo, pWaveHdr, sizeof(WAVEHDR));
				if (result != MMSYSERR_NOERROR)
				{
					DBGMMError(result);
				}
			}
			m_bWrittingPausedData = false;

			m_eventCanWrite.Set();

			waveOutRestart(m_hwo);
		}
	}

	return ERR_OK;
}

bool CMOSoundCard::IsPlaying()
{
	if (m_listPrepared.empty())
		return false;
	else
		return true;
}

MLRESULT CMOSoundCard::Stop()
{
	LIST_WAVHDR::iterator	it, itEnd;

	m_fadeMode = FADE_NONE;

	m_mutex.Acquire();
	itEnd = m_listPausedHdr.end();
	for(it = m_listPausedHdr.begin(); it != itEnd; ++it)
	{
		WAVEHDR		*pWaveHdr = *it;
		IFBuffer	*pBuf = (IFBuffer*)pWaveHdr->dwUser;
		pBuf->Release();
		m_nBuffered -= pWaveHdr->dwBufferLength;
		delete pWaveHdr;
	}
	m_listPausedHdr.clear();
	m_mutex.Release();

	if (g_bGaplessOutput)
	{
		if (m_hwo)
			waveOutReset(m_hwo);

		m_eventCanWrite.Set();
		return ERR_OK;
	}
	else
		return DoStop();
}

bool CMOSoundCard::IsOpened()
{
	return m_hwo != NULL;
}

// volume
MLRESULT CMOSoundCard::SetVolume(int nVolume, int nBanlance)
{
	ASSERT(nBanlance >= -100 && nBanlance <= 100);
	MMRESULT	ret;
	int			nLeft, nRight;

	nLeft = (int)((double)nVolume * (100 - nBanlance) * 0xFFFF / 10000);
	nRight = (int)((double)nVolume * (100 + nBanlance) * 0xFFFF / 10000);

	if (m_hwo)
		ret = waveOutSetVolume(m_hwo, MAKELONG(nLeft, nRight));
	else
		return ERR_FALSE;
	if (ret == MMSYSERR_NOERROR)
		return ERR_OK;
	else if (ret == MMSYSERR_NOTSUPPORTED)
		return ERR_NOT_SUPPORT;
	else if (ret == MMSYSERR_NOMEM)
		return ERR_NO_MEM;
	else
		return ERR_FALSE;
}

#define MAXINT32 0x7FFFFFFF

uint32 CMOSoundCard::GetPos()
{
	if (!m_hwo)
		return 0;

	MMTIME              sTime;
	uint32				nTime;

	sTime.wType = TIME_BYTES;
	if (waveOutGetPosition(m_hwo, &sTime, sizeof(sTime)) != MMSYSERR_NOERROR)
		return 0;

	nTime = (uint32)((double)sTime.u.cb * 1000 / (m_iBytesPerSample * m_nSamplerate));

	return nTime;
}


MLRESULT CMOSoundCard::DoOpen()
{
	MMRESULT        mmresult = 0;

	m_iBytesPerSample = m_nChannels * (m_nBitsPerSamp / 8);

	// cache 1 second of sound data
	m_nBufferedMax = m_iBytesPerSample * m_nSamplerate;
	m_nBuffered = 0;

	m_bWrittingPausedData = false;

	m_wfex.wBitsPerSample = m_nBitsPerSamp;
	m_wfex.wFormatTag = WAVE_FORMAT_PCM;
	m_wfex.nChannels = (WORD) m_nChannels;
	m_wfex.nSamplesPerSec = m_nSamplerate;
	m_wfex.nBlockAlign = (m_nBitsPerSamp / 8) * m_nChannels;
	m_wfex.nAvgBytesPerSec = m_nSamplerate * m_wfex.nBlockAlign;
	m_wfex.cbSize = 0;

	mmresult = waveOutOpen(&m_hwo,
		WAVE_MAPPER,
		&m_wfex,
		(DWORD) MCICallBack,
		(DWORD)this,
		WAVE_ALLOWSYNC | CALLBACK_FUNCTION);

	if (mmresult != MMSYSERR_NOERROR)
	{
		DBGMMError(mmresult);
		return ERR_FALSE;
	}

/*	ASSERT(!m_wavehdr_array);
	m_nBegFreeHeader = 0;
	m_nEndFreeHeader = m_nNumHeaders - 1;*/

	return ERR_OK;
}


MLRESULT CMOSoundCard::DoStop()
{
	if (m_hwo)
		waveOutReset(m_hwo);

	LIST_WAVHDR::iterator	it, itEnd;
	LIST_WAVHDR				listTemp;

	m_mutex.Acquire();
	listTemp = m_listFree;
	m_listFree.clear();

	itEnd = m_listPausedHdr.end();
	for(it = m_listPausedHdr.begin(); it != itEnd; ++it)
	{
		WAVEHDR		*pWaveHdr = *it;
		IFBuffer	*pBuf = (IFBuffer*)pWaveHdr->dwUser;
		m_nBuffered -= pWaveHdr->dwBufferLength;
		pBuf->Release();
		delete pWaveHdr;
	}
	m_listPausedHdr.clear();
	m_mutex.Release();

	itEnd = listTemp.end();
	for(it = listTemp.begin(); it != itEnd; ++it)
	{
		WAVEHDR		*pWaveHdr = *it;
		if (pWaveHdr->dwFlags & WHDR_PREPARED)
		{
			waveOutUnprepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
			pWaveHdr->dwFlags = 0;
		}
		delete pWaveHdr;
	}

	if (m_hwo)
	{
		waveOutClose(m_hwo);
		m_hwo = NULL;
	}

	return ERR_OK;
}

void CMOSoundCard::FadeInSoundData(char *data, int nLen)
{
	//
	// Fade in sound data.
	//
	ASSERT(m_nFadeDoneSamples < m_nFadeMaxSamples);

	int		n = nLen / (m_nBitsPerSamp / 8);
	int		nSamples = n / m_nChannels;
	int		i, nTo;

	if (m_nBitsPerSamp == 8)
	{
		char	*buf = data;

		if (m_nFadeDoneSamples + nSamples > m_nFadeMaxSamples)
			nTo = (m_nFadeMaxSamples - m_nFadeDoneSamples) * m_nChannels;
		else
			nTo = n;
		if (m_nChannels == 1)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples ++;
			}
		}
		else if (m_nChannels == 2)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				i++;
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples ++;
			}
		}
	}
	else if (m_nBitsPerSamp == 16)
	{
		short	*buf = (short *)data;

		if (m_nFadeDoneSamples + nSamples > m_nFadeMaxSamples)
			nTo = (m_nFadeMaxSamples - m_nFadeDoneSamples) * m_nChannels;
		else
			nTo = n;
		if (m_nChannels == 1)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples ++;
			}
		}
		else if (m_nChannels == 2)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				i++;
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples++;
			}
		}
	}
	else
		m_nFadeDoneSamples = m_nFadeMaxSamples;
}

void CMOSoundCard::FadeOutSoundData(char *data, int nLen)
{
	//
	// Fade out sound data.
	//

	int		n = nLen / (m_nBitsPerSamp / 8);
	int		nSamples = n / m_nChannels;
	int		i, nTo;

	if (m_nBitsPerSamp == 8)
	{
		char	*buf = data;

		if (m_nFadeDoneSamples < nSamples)
			nTo = m_nFadeDoneSamples * m_nChannels;
		else
			nTo = n;
		if (m_nChannels == 1)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples --;
			}
			for (; i < n; i++)
				buf[i] = 0;
		}
		else if (m_nChannels == 2)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				i++;
				buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples --;
			}
			for (; i < n; i++)
				buf[i] = 0;
		}
	}
	else if (m_nBitsPerSamp == 16)
	{
		short	*buf = (short *)data;

		if (m_nFadeDoneSamples < nSamples)
			nTo = m_nFadeDoneSamples * m_nChannels;
		else
			nTo = n;
		if (m_nChannels == 1)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples --;
			}
			for (; i < n; i++)
				buf[i] = 0;
		}
		else if (m_nChannels == 2)
		{
			for (i = 0; i < nTo; i++)
			{
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				i++;
				buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
				m_nFadeDoneSamples --;
			}
			for (; i < n; i++)
				buf[i] = 0;
		}
	}
	else
		m_nFadeDoneSamples = 0;
}
