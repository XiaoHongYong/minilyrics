// DspDemo.h: interface for the CDspDemo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_)
#define AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CDspDemo : public IDSP  
{
	OBJ_REFERENCE_DECL

public:
	CDspDemo();
	virtual ~CDspDemo();

	virtual MLRESULT Init(IMPlayer *pPlayer);
	virtual MLRESULT Quit();

	virtual void Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);
	virtual void ProcessEcho(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);
	virtual void ProcessPitch(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

	virtual void ProcessFadeInout(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

	virtual void ProcessVoice(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

protected:
	CMPAutoPtr<IMPlayer>	m_pPlayer;
	CCharBuffer				m_buffPitch;

};

#endif // !defined(AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_)
