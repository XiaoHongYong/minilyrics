// MDRow.h: interface for the CMDRow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_)
#define AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CMDRow : public IMediaDecode
{
OBJ_REFERENCE_DECL
public:
	CMDRow();
	virtual ~CMDRow();

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
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();

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
	uint32			m_nLengthMs;

	CThread			m_threadDecode;

};

#endif // !defined(AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_)
