// MDWave.h: interface for the CMDWave class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDWAVE_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
#define AFX_MDWAVE_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CMDWave : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
	CMDWave();
	virtual ~CMDWave();

	//
	// individual methods
	//

	virtual LPCXSTR GetDescription();
	virtual LPCXSTR GetFileExtentions();
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
	struct WaveFileInfo
	{
		short	format_tag, channels, block_align, bits_per_sample;
		long	samples_per_sec, avg_bytes_per_sec;
		unsigned long position, length;
		uint32	nMediaFileSize;
		int		nMediaLength;	// ms
		long	data_offset;
	};

	MLRESULT GetHeadInfo(IMediaInput *pInput);

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

	WaveFileInfo	m_audioInfo;

	CCharBuffer		m_bufInput;
	CThread			m_threadDecode;

};

#endif // !defined(AFX_MDWAVE_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
