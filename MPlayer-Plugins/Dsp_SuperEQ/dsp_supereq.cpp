#include "dsp_supereq.h"
#include "paramlist.hpp"
#include <math.h>

extern void equ_init(int wb);
extern void equ_makeTable(float *lbc,float *rbc,paramlist *,float fs);
extern void equ_quit(void);
extern int equ_modifySamples(char *buf,int nsamples,int nch,int bps);
extern void equ_clearbuf(int,int);

float	lbands[18] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
float	rbands[18] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
paramlist	paramroot;

// The range of lslpos, rslpos is [-20, 20](dB).
int lslpos[19],rslpos[19],last_lslpos[19],last_rslpos[19];


void SetBandsFromSlpos()
{
	int i;
	float lpreamp = pow(10.0f, lslpos[0]/20.0f);
	float rpreamp = pow(10.0f, rslpos[0]/20.0f);

	for(i=0;i<18;i++)
	{
		lbands[i] = lpreamp * pow(10.0f,lslpos[i+1]/20.0f);
		rbands[i] = rpreamp * pow(10.0f,rslpos[i+1]/20.0f);
	}

	equ_makeTable(lbands,rbands,&paramroot, 0);
}

CDspSuperEQ::CDspSuperEQ()
{
	m_pPlayer = NULL;
	m_nLastSRate = 0;
	m_nLastChannel = 0;
	m_nLastBps = 0;
	m_pPlayer = 0;
}


CDspSuperEQ::~CDspSuperEQ()
{
}

CDspSuperEQ * CDspSuperEQ::GetInstance()
{
	static CDspSuperEQ instance;
	return &instance;
}

MLRESULT CDspSuperEQ::Init(IMPlayer *pPlayer)
{
	m_pPlayer = pPlayer;

	equ_init(14);

	for(int i=0;i<=18;i++)
	{
		if (lslpos[i] < -20 ) lslpos[i] = -20;
		if (lslpos[i] > 20) lslpos[i] = 20;
		if (rslpos[i] < -20 ) rslpos[i] = -20;
		if (rslpos[i] > 20) rslpos[i] = 20;
	}

#if 0
	lslpos[0] = rslpos[0] = 0;  // Preamp
	lslpos[1] = rslpos[1] = 10;
	lslpos[2] = rslpos[2] = 10;
	lslpos[3] = rslpos[3] = 10;
	lslpos[4] = rslpos[4] = 10;
	lslpos[5] = rslpos[5] = 10;
	lslpos[6] = rslpos[6] = 0;
	lslpos[7] = rslpos[7] = 0;
	lslpos[8] = rslpos[8] = 0;
	lslpos[9] = rslpos[9] = 0;
	lslpos[10] = rslpos[10] = 0;
	lslpos[11] = rslpos[11] = 0;
	lslpos[12] = rslpos[12] = 0;
	lslpos[13] = rslpos[13] = 0;
	lslpos[14] = rslpos[14] = 0;
	lslpos[15] = rslpos[15] = 0;
	lslpos[16] = rslpos[16] = 0;
	lslpos[17] = rslpos[17] = 0;
	lslpos[18] = rslpos[18] = 0;
#endif

	SetBandsFromSlpos();

	return ERR_OK;
}


MLRESULT CDspSuperEQ::Quit()
{
	ASSERT(m_pPlayer);

	m_pPlayer.Release();
	m_pPlayer = NULL;
	equ_quit();

	return ERR_OK;
}


void CDspSuperEQ::Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
	if ((nChannels != 1 && nChannels != 2) || (nBps != 8 && nBps != 16 && nBps != 24)) 
		return;

	if (nSampleRate == 0)
		return;

	if (m_nLastSRate != nSampleRate)
	{
		equ_makeTable(lbands, rbands, &paramroot, (float)nSampleRate);
		m_nLastSRate = nSampleRate;
		m_nLastChannel = nChannels;
		m_nLastBps = nBps;
		equ_clearbuf(nBps, nSampleRate);
	}
	else if (m_nLastChannel != nChannels || m_nLastBps != nBps)
	{
		m_nLastChannel = nChannels;
		m_nLastBps = nBps;
		equ_clearbuf(nBps, nSampleRate);
	}

	equ_modifySamples((char *)pBuf->data(), pBuf->size() / (nChannels * nBps / 8), nChannels, nBps);
}