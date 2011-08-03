// MDWave.cpp: implementation of the CMDWave class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MDWave.h"

#define	WAVE_FORMAT_UNKNOWN		(0x0000)
#define	WAVE_FORMAT_PCM			(0x0001)
#define	WAVE_FORMAT_ADPCM		(0x0002)
#define	WAVE_FORMAT_ALAW		(0x0006)
#define	WAVE_FORMAT_MULAW		(0x0007)
#define	WAVE_FORMAT_OKI_ADPCM		(0x0010)
#define	WAVE_FORMAT_DIGISTD		(0x0015)
#define	WAVE_FORMAT_DIGIFIX		(0x0016)
#define	IBM_FORMAT_MULAW         	(0x0101)
#define	IBM_FORMAT_ALAW			(0x0102)
#define	IBM_FORMAT_ADPCM         	(0x0103)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDWave::CMDWave()
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
	m_bufInput.reserve(1024 * 16);
}

CMDWave::~CMDWave()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

LPCXSTR CMDWave::GetDescription()
{
	return _T("Wave File decoder");
}

LPCXSTR CMDWave::GetFileExtentions()
{
	return _T(".wav|wave files");
}

MLRESULT CMDWave::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	MLRESULT		nRet;

	nRet = GetHeadInfo(pInput);
	if (nRet != ERR_OK)
		return nRet;

	pMedia->SetAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
 	pMedia->SetAttribute(MA_DURATION, m_audioInfo.nMediaLength);
 	pMedia->SetAttribute(MA_SAMPLE_RATE, m_audioInfo.samples_per_sec);
 	pMedia->SetAttribute(MA_CHANNELS, m_audioInfo.channels);
 	pMedia->SetAttribute(MA_BPS, m_audioInfo.bits_per_sample);
	pMedia->SetAttribute(MA_BITRATE, m_audioInfo.avg_bytes_per_sec);

	return nRet;
}

bool CMDWave::IsSeekable()
{
	return true;
}

bool CMDWave::IsUseOutputPlug()
{
	return true;
}

MLRESULT CMDWave::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ASSERT(pPlayer && pInput);
	if (!pPlayer || !pInput)
		return ERR_INVALID_HANDLE;

	Stop();

	MLRESULT		nRet;

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

	nRet = GetHeadInfo(m_pInput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pOutput->Open(m_audioInfo.samples_per_sec, m_audioInfo.channels, m_audioInfo.bits_per_sample);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_state = PS_PLAYING;
	m_bKillThread = false;

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

MLRESULT CMDWave::Pause()
{
	m_state = PS_PAUSED;
	return m_pOutput->Pause(true);
}

MLRESULT CMDWave::Unpause()
{
	m_state = PS_PLAYING;
	return m_pOutput->Pause(false);
}

MLRESULT CMDWave::Stop()
{
	m_bKillThread = true;

	if (m_state == PS_PAUSED)
	{
		m_pOutput->Flush();
	}

	m_threadDecode.Join();

	return ERR_OK;
}

uint32 CMDWave::GetLength()
{
	return m_audioInfo.nMediaLength;
}

MLRESULT CMDWave::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDWave::GetPos()
{
	if (m_pOutput)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

MLRESULT CMDWave::SetVolume(int nVolume, int nBanlance)
{
	return m_pOutput->SetVolume(nVolume, nBanlance);
}

void CMDWave::DecodeThread(LPVOID lpParam)
{
	CMDWave		*pMDRow;

	pMDRow = (CMDWave *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}


int read_le_long(IMediaInput *pInput, long *ret)
{
	unsigned char buf[4];

	if (pInput->Read(buf, 4) != 4)
		return 0;

	*ret = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
	return 4;
}

int read_le_short(IMediaInput *pInput, short *ret)
{
	unsigned char buf[2];

	if (pInput->Read(buf, 2) != 2)
		return 0;

	*ret = (buf[1] << 8) | buf[0];
	return TRUE;
}

void CMDWave::DecodeThreadProc()
{
	int bytes, blk_size, size_per_second;
	int actual_read;

	size_per_second = m_audioInfo.samples_per_sec * m_audioInfo.channels *
		(m_audioInfo.bits_per_sample / 8);
	blk_size = size_per_second / 8;

	IFBuffer	*pBuf;

	while (!m_bKillThread)
	{
		bytes = blk_size;
		if (int(m_audioInfo.length - m_audioInfo.position) < bytes)
			bytes = m_audioInfo.length - m_audioInfo.position;
		if (bytes > 0)
		{
			pBuf = m_pMemAllocator->AllocFBuffer(blk_size);
			actual_read = m_pInput->Read(pBuf->data(), bytes);
			if (actual_read == 0)
			{
				pBuf->Release();
				break;
			}
			else
			{
				pBuf->resize(actual_read);
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
					m_pPlayer->OutputWrite(pBuf, m_audioInfo.bits_per_sample, m_audioInfo.channels, m_audioInfo.samples_per_sec);
				}
				m_audioInfo.position += actual_read;
			}
		}
		else
		{
			break;
		}
		if (m_bSeekFlag)
		{
			m_audioInfo.position = m_nSeekPos * (size_per_second / 1000);
			m_pInput->Seek(m_audioInfo.position + m_audioInfo.data_offset, SEEK_SET);
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

	ERRLOG0(_T("End of decode thread! Quit."));
}

MLRESULT CMDWave::GetHeadInfo(IMediaInput *pInput)
{
	m_bufInput.clear();
	pInput->Seek(0, SEEK_SET);

	char			szMagic[4];
	char			magic[4];
	long			len;

	memset(&m_audioInfo, 0, sizeof(m_audioInfo));

	pInput->GetSize(m_audioInfo.nMediaFileSize);

	if (pInput->Read(szMagic, 4) != 4)
		return ERR_BAD_FILE_FORMAT;
	if (strncmp(szMagic, "RIFF", 4))
		return ERR_BAD_FILE_FORMAT;
	if (!read_le_long(pInput, &len))
		return ERR_BAD_FILE_FORMAT;
	if (pInput->Read(szMagic, 4) != 4)
		return ERR_BAD_FILE_FORMAT;
	if (strncmp(szMagic, "WAVE", 4) != 0)
		return ERR_BAD_FILE_FORMAT;

	for (;;)
	{
		if (pInput->Read(magic, 4) != 4)
			return ERR_BAD_FILE_FORMAT;
		if (!read_le_long(pInput, &len))
			return ERR_BAD_FILE_FORMAT;
		if (!strncmp("fmt ", magic, 4))
			break;
		pInput->Seek(len, SEEK_CUR);
	}
	if (len < 16)
		return ERR_BAD_FILE_FORMAT;
	if (!read_le_short(pInput, &m_audioInfo.format_tag))
		return ERR_BAD_FILE_FORMAT;

	switch (m_audioInfo.format_tag)
	{
		case WAVE_FORMAT_UNKNOWN:
		case WAVE_FORMAT_ALAW:
		case WAVE_FORMAT_MULAW:
		case WAVE_FORMAT_ADPCM:
		case WAVE_FORMAT_OKI_ADPCM:
		case WAVE_FORMAT_DIGISTD:
		case WAVE_FORMAT_DIGIFIX:
		case IBM_FORMAT_MULAW:
		case IBM_FORMAT_ALAW:
		case IBM_FORMAT_ADPCM:
			return ERR_BAD_FILE_FORMAT;
	}
	read_le_short(pInput, &m_audioInfo.channels);
	read_le_long(pInput, &m_audioInfo.samples_per_sec);
	read_le_long(pInput, &m_audioInfo.avg_bytes_per_sec);
	read_le_short(pInput, &m_audioInfo.block_align);
	read_le_short(pInput, &m_audioInfo.bits_per_sample);
	if (m_audioInfo.bits_per_sample != 8 && m_audioInfo.bits_per_sample != 16)
		return ERR_BAD_FILE_FORMAT;
	len -= 16;
	if (len)
		pInput->Seek(len, SEEK_CUR);

	for (;;)
	{
		if (pInput->Read(magic, 4) != 4)
			return ERR_BAD_FILE_FORMAT;

		if (!read_le_long(pInput, &len))
			return ERR_BAD_FILE_FORMAT;
		if (!strncmp("data", magic, 4))
			break;
		pInput->Seek(len, SEEK_CUR);
	}
	m_audioInfo.data_offset = pInput->GetPos();
	m_audioInfo.length = len;
	m_audioInfo.position = 0;
	m_audioInfo.nMediaLength = (int)((double)len / (m_audioInfo.channels * m_audioInfo.samples_per_sec * (m_audioInfo.bits_per_sample / 8)) * 1000);

	return ERR_OK;
}
