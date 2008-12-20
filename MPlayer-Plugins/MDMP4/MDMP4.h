// MDMP4.h: interface for the CMDMP4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
#define AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMDMP4 : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
	CMDMP4();
	virtual ~CMDMP4();

	//
	// individual methods
	//

	virtual LPCXSTR GetDescription();
	virtual LPCXSTR GetFileExtentions();	// get supported file's extentions.
	virtual MLRESULT GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia);

	//
	// decode media file related methods
	//

	virtual bool IsSeekable();
	virtual bool IsUseOutputPlug();

	virtual MLRESULT Play(IMPlayer *pPlayer, IMediaInput *pInput);
	virtual MLRESULT Pause();
	virtual MLRESULT Unpause();
	virtual bool IsPaused();
	virtual MLRESULT Stop();

	// time length
	virtual uint32 GetLength();
	virtual MLRESULT Seek(uint32 dwPos);
	virtual uint32 GetPos();

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance);

	struct AUDIO_INFO
	{
		int				nMediaLength;	// ms
		int				nBitRate;		// 
		uint32			nMediaFileSize;	// byte
		int				Bps;
		unsigned long	nSampleRate;
		unsigned char	nChannels;

		bool			m_bSeekable;
	};

protected:
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();

public:
	IMediaOutput	*m_pOutput;
	IMediaInput		*m_pInput;
	IMPlayer		*m_pPlayer;
	IMemAllocator	*m_pMemAllocator;
	PLAYER_STATE	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	bool			m_bSeekFlag;

	AUDIO_INFO		m_audioInfo;

	CThread			m_threadDecode;

protected:
	MLRESULT PreDecode();
	MLRESULT AACPlayThreadFun();
	MLRESULT MP4PlayThreadFun();
	bool			m_bIsSeekable;

};

#endif // !defined(AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
