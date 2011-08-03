// MDRow.cpp: implementation of the CMDRow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMPlayer.h"
#include "MDRow.h"


// raw configuration
#define NCH 2
#define SAMPLERATE 44100
#define BPS 16
//#define NCH 1
//#define SAMPLERATE 22050
//#define BPS 8

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMDRow::CMDRow()
{
	OBJ_REFERENCE_INIT;

	m_pInput = NULL;
	m_pPlayer = NULL;
	m_pOutput = NULL;
	m_bPaused = false;
	m_bKillThread = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;
	m_state = PS_STOPED;
}

CMDRow::~CMDRow()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

LPCXSTR CMDRow::GetDescription()
{
	return _T("Raw File decoder");
}

LPCXSTR CMDRow::GetFileExtentions()
{
	return _T(".raw|raw files");
}

MLRESULT CMDRow::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	uint32		nFileSize;
	pInput->GetSize(nFileSize);

	m_nLengthMs = (uint32)((int64)nFileSize * 1000 / (SAMPLERATE * NCH * (BPS / 8)));

	pMedia->SetAttribute(MA_FILESIZE, nFileSize);
 	pMedia->SetAttribute(MA_DURATION, m_nLengthMs);
 	pMedia->SetAttribute(MA_SAMPLE_RATE, SAMPLERATE);
 	pMedia->SetAttribute(MA_CHANNELS, NCH);
 	pMedia->SetAttribute(MA_BPS, BPS);

	return ERR_OK;
}

bool CMDRow::IsSeekable()
{
	return true;
}

bool CMDRow::IsUseOutputPlug()
{
	return true;
}

MLRESULT CMDRow::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ASSERT(pPlayer && pInput);
	if (!pPlayer || !pInput)
		return ERR_INVALID_HANDLE;

	MLRESULT		nRet;
	uint32		nFileSize;

	Stop();

	m_state = PS_STOPED;
	m_bPaused = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;

	m_pPlayer = pPlayer;
	m_pPlayer->AddRef();

	m_pInput = pInput;
	m_pInput->AddRef();

	nRet = m_pPlayer->QueryInterface(MPIT_OUTPUT, (LPVOID *)&m_pOutput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pPlayer->QueryInterface(MPIT_MEMALLOCATOR, (LPVOID *)&m_pMemAllocator);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_state = PS_PLAYING;
	m_bKillThread = false;

	pInput->GetSize(nFileSize);

	m_nLengthMs = (uint32)((int64)nFileSize * 1000 / (SAMPLERATE * NCH * (BPS / 8)));

	nRet = m_pOutput->Open(SAMPLERATE, NCH, BPS);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_threadDecode.Create(DecodeThread, this);
	m_threadDecode.SetPriority(THREAD_PRIORITY_HIGHEST);
// #ifdef _WIN32_WCE
// 	HMODULE hModule = LoadLibrary(_T("Coredll.dll"));
// 	if (hModule) {
// 		BOOL (WINAPI *pCeSetThreadPriority)(HANDLE hThread, int nPriority); 
// 		(FARPROC&)pCeSetThreadPriority = GetProcAddress(hModule, _T("CeSetThreadPriority"));
// 		if (pCeSetThreadPriority) 
// 			pCeSetThreadPriority(GetCurrentThread(), 160);
// 		FreeLibrary(hModule);
// 	}
// #endif

	return ERR_OK;

R_FAILED:
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	if (m_pInput)
	{
		m_pInput->Release();
		m_pInput = NULL;
	}
	if (m_pOutput)
	{
		m_pOutput->Release();
		m_pOutput = NULL;
	}
	if (m_pMemAllocator)
	{
		m_pMemAllocator->Release();
		m_pMemAllocator = NULL;
	}
	return nRet;
}

MLRESULT CMDRow::Pause()
{
	m_state = PS_PAUSED;
	return m_pOutput->Pause(true);
}

MLRESULT CMDRow::Unpause()
{
	m_state = PS_PLAYING;
	return m_pOutput->Pause(false);
}

MLRESULT CMDRow::Stop()
{
	m_bKillThread = true;

	if (m_state == PS_PAUSED)
	{
		m_pOutput->Flush();
	}

	m_threadDecode.Join();

	return ERR_OK;
}

uint32 CMDRow::GetLength()
{
	return m_nLengthMs;
}

MLRESULT CMDRow::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDRow::GetPos()
{
	if (m_pOutput)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

MLRESULT CMDRow::SetVolume(int nVolume, int nBanlance)
{
	return m_pOutput->SetVolume(nVolume, nBanlance);
}

void CMDRow::DecodeThread(LPVOID lpParam)
{
	CMDRow		*pMDRow;

	pMDRow = (CMDRow *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}

void CMDRow::DecodeThreadProc()
{
	IFBuffer		*pBuf;
	uint32			bufSize, nReaded;

	// bufSize = SAMPLERATE * NCH * (BPS / 8);
	bufSize = 512 *4;

	while (!m_bKillThread)
	{
		pBuf = m_pMemAllocator->AllocFBuffer(bufSize + 4);
		nReaded = m_pInput->Read(pBuf->data(), bufSize);
		if (nReaded == 0)
		{
			pBuf->Release();
			pBuf = NULL;
			break;
		}
		pBuf->resize(nReaded);
		m_pOutput->WaitForWrite();
		if (m_bKillThread)
		{
			pBuf->Release();
			break;
		}
		if (m_bSeekFlag)
			pBuf->Release();
		else
		{
			m_pPlayer->OutputWrite(pBuf, SAMPLERATE, NCH, BPS);
		}

		if (m_bSeekFlag)
		{
			uint32	nSeekTo = (uint32)((double)m_nSeekPos * SAMPLERATE * NCH * (BPS / 8) / 1000);
			m_pInput->Seek(nSeekTo, SEEK_SET);
			m_pOutput->Flush();
			m_bSeekFlag = false;
		}
	}

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	m_state = PS_STOPED;

	m_pOutput->Stop();
	m_pOutput->Release();
	m_pOutput = NULL;

	m_pInput->Release();
	m_pInput = NULL;

	m_pMemAllocator->Release();
	m_pMemAllocator = NULL;

	m_pPlayer->NotifyEod(this, ERR_OK);
	m_pPlayer->Release();
	m_pPlayer = NULL;
}
