// MOSoundCard.h: interface for the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)
#define AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

class CMOSoundCard : public IMediaOutput  
{
OBJ_REFERENCE_DECL
public:
	enum FADE_MODE
	{
		FADE_NONE,
		FADE_IN,
		FADE_OUT,
	};

	CMOSoundCard();
	virtual ~CMOSoundCard();

	LPCXSTR GetDescription();

	virtual MLRESULT Init(IMPlayer *pPlayer) { return ERR_OK; }
	virtual MLRESULT Quit() { return ERR_OK; }

	virtual MLRESULT Open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

	virtual MLRESULT WaitForWrite();

	virtual MLRESULT Write(IFBuffer *pBuf);
	virtual MLRESULT Flush();

	virtual MLRESULT Pause(bool bPause);
	virtual bool IsPlaying();
	virtual MLRESULT Stop();

	virtual bool IsOpened();

	// volume
	MLRESULT SetVolume(int nVolume, int nBanlance);

	virtual uint32 GetPos();

protected:
	MLRESULT DoOpen();
	MLRESULT DoStop();

	void FadeInSoundData(char *buf, int nLen);
	void FadeOutSoundData(char *buf, int nLen);

	WAVEHDR* NewWaveHdr();

public:
	static void CALLBACK MCICallBack(HWAVEOUT hwo, UINT msg, DWORD dwInstance,
				DWORD dwParam1, DWORD dwParam2);

private:
	Event			m_eventCanWrite;
	DWORD			m_dwTotolBytesOffset;		// 输入到MOSoundCard中的字节偏移总量

    WAVEFORMATEX	m_wfex;
    HWAVEOUT        m_hwo;

    int				m_iBytesPerSample;
	DWORD			m_dwDataSize;

	typedef	list<WAVEHDR *>		LIST_WAVHDR;

	Mutex			m_mutex;
	list<WAVEHDR *>	m_listFree, m_listPrepared, m_listPausedHdr;
	int				m_nBuffered;
	int				m_nBufferedMax;

	FADE_MODE		m_fadeMode;
	long			m_nFadeDoneSamples;
	long			m_nFadeMaxSamples;

	bool			m_bWrittingPausedData;
	
	WAVEHDR			*m_pFadeOutEndHdr;
	
};

#endif // !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)

