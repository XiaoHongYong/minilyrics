// MORaw.h: interface for the CMORaw class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MORAW_H__3B18E176_AD97_407E_90A2_80B04178519B__INCLUDED_)
#define AFX_MORAW_H__3B18E176_AD97_407E_90A2_80B04178519B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CMORaw : public IMediaOutput  
{
OBJ_REFERENCE_DECL
public:
	CMORaw();
	virtual ~CMORaw();

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
	DWORD			m_dwTotolBytesOffset;		// 输入到MOSoundCard中的字节偏移总量
	int				m_nBps, m_nSampleRate, m_nChannels;
	int				m_nBytesPerSample;
	FILE			*m_fp;
	bool			m_bPaused;
	Event			m_eventCanWrite;

};

#endif // !defined(AFX_MORAW_H__3B18E176_AD97_407E_90A2_80B04178519B__INCLUDED_)
