// MDAPE.h: interface for the CMDAPE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_)
#define AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../base/base.h"
#include "../../MPlayerLib/IMPlayer.h"

class CMDAPE : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
	CMDAPE();
	virtual ~CMDAPE();

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

protected:
	struct AUDIO_INFO
	{
		int			nMediaLength;	// ms
		int			nBitRate;		// 
		uint32		nMediaFileSize;	// byte
		int			nBps;
		int			nSampleRate;
		int			nChannels;
	};

	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();
	bool OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

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

	LPVOID			m_lpAPEDecoder;

};

#endif // !defined(AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_)
