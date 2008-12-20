// MDWMA.h: interface for the CMDWMA class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDWMA_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_)
#define AFX_MDWMA_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../base/mlstdafx.h"
#include "../../base/base.h"
#include "../../MPlayerLib/IMPlayer.h"

class CMDWMA : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
	CMDWMA();
	virtual ~CMDWMA();

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
		int			nMediaLength;	// ms
		int			nBitRate;		// 
		uint32		nMediaFileSize;	// byte
		int			nBps;
		int			nSampleRate;
		int			nChannels;

		bool		m_bSeekable;
	};

protected:
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();

	void wma_playbuff(int out_size);

	void WMADecode();

	// bool OutputWrite(IFBuffer *pBuf);

protected:
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

	// wma codec data
	AVCodecContext	*m_CodecContext;
	AVFormatContext *m_FormatContext;
	int				m_nWmaIndex;
    AVCodec			*m_Codec;

};

#endif // !defined(AFX_MDWMA_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_)
