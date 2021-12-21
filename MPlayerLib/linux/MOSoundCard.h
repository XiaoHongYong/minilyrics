// MOSoundCard.h: interface for the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)
#define AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_

#pragma once

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

private:
	Event			m_eventCanWrite;
	DWORD			m_dwTotolBytesOffset;		// 输入到MOSoundCard中的字节偏移总量

    int				m_iBytesPerSample;
	DWORD			m_dwDataSize;

	Mutex			m_mutex;
	int				m_nBuffered;
	int				m_nBufferedMax;

	FADE_MODE		m_fadeMode;
	long			m_nFadeDoneSamples;
	long			m_nFadeMaxSamples;

	bool			m_bWrittingPausedData;
	
};

#endif // !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)

