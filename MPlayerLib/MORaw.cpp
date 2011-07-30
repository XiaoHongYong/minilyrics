// MORaw.cpp: implementation of the CMORaw class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MORaw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMORaw::CMORaw() : m_eventCanWrite(false, true)
{
	OBJ_REFERENCE_INIT;

	m_dwTotolBytesOffset = 0;
	m_fp = NULL;
	m_bPaused = false;
	m_nChannels = 0;
	m_nSamplerate = 0;
	m_nBps = 0;
}

CMORaw::~CMORaw()
{
}

LPCXSTR CMORaw::GetDescription()
{
	return _T("MPlayer Raw file output 1.0");
}

MLRESULT CMORaw::Open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
	m_bPaused = false;
	m_nChannels = nNumChannels;
	m_nSamplerate = nSampleRate;
	m_nBps = nBitsPerSamp;

	m_nBytesPerSample = nNumChannels * (nBitsPerSamp / 8);

// #ifndef _WIN32_WCE
	TCHAR		szFile[MAX_PATH];
	GetWorkingFolder(szFile);
	_tcscat_safe(szFile, CountOf(szFile), _T("mout.raw"));
	m_fp = _tfopen(szFile, _T("wb"));
	if (!m_fp)
		return ERR_OPEN_FILE;
// #endif

	return ERR_OK;
}

MLRESULT CMORaw::WaitForWrite()
{
	Sleep(10);

	m_eventCanWrite.Acquire();
	m_eventCanWrite.Set();

	return ERR_OK;
}

MLRESULT CMORaw::Write(IFBuffer *pBuf)
{
	int		nRet = ERR_OK;

	m_dwTotolBytesOffset += pBuf->size();

	static	int		iii = 0;
	ERRLOG2(_T("write: %d, %d"), iii, pBuf->size());
	iii++;

	if (m_fp && pBuf->size())
	{
		if (fwrite(pBuf->data(), 1, pBuf->size(), m_fp) > 0)
			nRet = ERR_OK;
		else
			nRet = ERR_WRITE_FILE;
	}

	pBuf->Release();

	return nRet;
}

MLRESULT CMORaw::Flush()
{
	if (m_bPaused)
	{
		m_bPaused = false;
		m_eventCanWrite.Set();
	}
	return ERR_OK;
}

MLRESULT CMORaw::Pause(bool bPause)
{
	m_bPaused = bPause;
	if (bPause)
		m_eventCanWrite.Reset();
	else
		m_eventCanWrite.Set();

	return ERR_OK;
}

bool CMORaw::IsPlaying()
{
	return false;
}

MLRESULT CMORaw::Stop()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	if (m_bPaused)
	{
		m_bPaused = false;
		m_eventCanWrite.Set();
	}

	return ERR_OK;
}

bool CMORaw::IsOpened()
{
	return m_nBps != 0;
}

// volume
MLRESULT CMORaw::SetVolume(int nVolume, int nBanlance)
{
	return ERR_OK;
}

uint32 CMORaw::GetPos()
{
	uint32 nTime = (uint32)((double)m_dwTotolBytesOffset * 1000 / (m_nBytesPerSample * m_nSamplerate));

	return nTime;
}
