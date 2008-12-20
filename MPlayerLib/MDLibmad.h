// MDLibmad.h: interface for the CMDLibmad class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_)
#define AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"
#include "../MPlayer-Plugins/libmad-0.15.1b/mad.h"
#include "audio.h"

struct MPEG_AUDIO_INFO
{
	mad_header	header;

	int			nSamplesPerFrame;
	int			nFrameSize;
	int			nFrameCount;
	int			nMediaLength;	// ms

	bool		bVbr;
	int			nBitRate;		// same as header.bitrate if not vbr.

	uint32		nMediaFileSize;	// byte
	uint32		nFirstFramePos;
};

class CMDLibmad : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
	CMDLibmad();
	virtual ~CMDLibmad();

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
	virtual MLRESULT Stop();

	// time length
	virtual uint32 GetLength();
	virtual MLRESULT Seek(uint32 dwPos);
	virtual uint32 GetPos();

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance);

protected:
	MLRESULT GetHeadInfo(IMediaInput *pInput);

	static void DecodeThread(LPVOID lpParam);
	int mad_decoder_run(struct mad_decoder *decoder);
	int run_sync(struct mad_decoder *decoder);

	DWORD DecodeThreadProc();

	// bool OutputWrite(IFBuffer *pBuf, int nChannels, int nSampleRate);

public:
	IMediaOutput	*m_pOutput;
	IMediaInput		*m_pInput;
	IMPlayer		*m_pPlayer;
	IMemAllocator	*m_pMemAllocator;
	PLAYER_STATE	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	uint32			m_nWritePosByte;
	bool			m_bSeekFlag;

	CCharBuffer		m_bufInput;
	CThread			m_threadDecode;

	MPEG_AUDIO_INFO	m_audioInfo;

	mad_decoder 	m_decoder;
	audio_stats		m_audioStats;

};

#endif // !defined(AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_)
