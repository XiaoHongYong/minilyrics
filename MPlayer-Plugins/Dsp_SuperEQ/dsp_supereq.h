#ifndef DSP_SUPEREQ_H
#define DSP_SUPEREQ_H
#pragma once

#include "mlstdafx.h"
#include "Error.h"
#include "IMPlayer.h"


class CDspSuperEQ : public IDSP  
{
protected:
	CDspSuperEQ();
	virtual ~CDspSuperEQ();


public:
	static CDspSuperEQ * GetInstance();
	virtual void AddRef() {};
	virtual void Release() {};

	virtual MLRESULT Init(IMPlayer *pPlayer);
	virtual MLRESULT Quit();

	virtual void Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

protected:
	int						m_nLastSRate;
	int						m_nLastChannel;
	int						m_nLastBps;
	CMPAutoPtr<IMPlayer>	m_pPlayer;
};

#endif
