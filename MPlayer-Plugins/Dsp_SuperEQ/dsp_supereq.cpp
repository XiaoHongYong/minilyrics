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


int lslpos[19],rslpos[19],last_lslpos[19],last_rslpos[19];

//void load_from(char *filename,int igerr)
//{
//	CFile f;
//	CFileException e;
//
//	if( !f.Open( filename, CFile::modeRead, &e ) )
//	{
//		if (!igerr)
//			::MessageBox(NULL,"Couldn't open file","Error",MB_OK | MB_ICONEXCLAMATION);
//		return;
//	}
//
//	char buf[256];
//
//	my_fgets(buf,256,f);
//
//	if (sscanf(buf,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
//		"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
//		&lslpos[0],&lslpos[1],&lslpos[2],&lslpos[3],&lslpos[4],&lslpos[5],
//		&lslpos[6],&lslpos[7],&lslpos[8],&lslpos[9],&lslpos[10],&lslpos[11],
//		&lslpos[12],&lslpos[13],&lslpos[14],&lslpos[15],&lslpos[16],&lslpos[17],
//		&lslpos[18],
//		&rslpos[0],&rslpos[1],&rslpos[2],&rslpos[3],&rslpos[4],&rslpos[5],
//		&rslpos[6],&rslpos[7],&rslpos[8],&rslpos[9],&rslpos[10],&rslpos[11],
//		&rslpos[12],&rslpos[13],&rslpos[14],&rslpos[15],&rslpos[16],&rslpos[17],
//		&rslpos[18]) != 38) {
//			::MessageBox(NULL,"Format error","Error",MB_OK | MB_ICONEXCLAMATION);
//			// error
//	}
//
//	delete paramroot.elm;
//	paramroot.elm = NULL;
//	paramlistelm **p;
//
//	for(p = &paramroot.elm;;p = &(*p)->next)
//	{
//		float lower,upper,gain;
//		int left,right,r;
//
//		my_fgets(buf,256,f);
//		r = sscanf(buf,"%g %g %g %d %d",&upper,&lower,&gain,&left,&right);
//		if (r == 0 || r == EOF) break;
//		if (r != 5) {
//			sprintf(buf,"Format error 2 (%d)",r);
//			::MessageBox(NULL,buf,"Error",MB_OK | MB_ICONEXCLAMATION);
//			break;
//		}
//
//		*p = new paramlistelm;
//		(*p)->lower = lower;
//		(*p)->upper = upper;
//		(*p)->gain  = gain;
//		(*p)->left  = left;
//		(*p)->right = right;
//	}
//
//	f.Close();
//}

void setBandsFromSlpos(void)
{
	int i;
	float lpreamp = lslpos[0] == 96 ? 0 : pow(10,lslpos[0]/-20.0);
	float rpreamp = rslpos[0] == 96 ? 0 : pow(10,rslpos[0]/-20.0);

	for(i=0;i<18;i++)
	{
		lbands[i] = lslpos[i+1] == 96 ? 0 : lpreamp*pow(10,lslpos[i+1]/-20.0);
		rbands[i] = rslpos[i+1] == 96 ? 0 : rpreamp*pow(10,rslpos[i+1]/-20.0);
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
		if (lslpos[i] < 0 ) lslpos[i] = 0;
		if (lslpos[i] > 96) lslpos[i] = 96;
		if (rslpos[i] < 0 ) rslpos[i] = 0;
		if (rslpos[i] > 96) rslpos[i] = 96;
	}

#if 0
	lslpos[0] = rslpos[0] = 0;
	lslpos[1] = rslpos[1] = 0;
	lslpos[2] = rslpos[2] = 0;
	lslpos[3] = rslpos[3] = 0;
	lslpos[4] = rslpos[4] = 0;
	lslpos[5] = rslpos[5] = 0;
	lslpos[6] = rslpos[6] = 0;
	lslpos[7] = rslpos[7] = 0;
	lslpos[8] = rslpos[8] = 0;
	lslpos[9] = rslpos[9] = 0;
	lslpos[10] = rslpos[10] = 50;
	lslpos[11] = rslpos[11] = 50;
	lslpos[12] = rslpos[12] = 50;
	lslpos[13] = rslpos[13] = 50;
	lslpos[14] = rslpos[14] = 50;
	lslpos[15] = rslpos[15] = 50;
	lslpos[16] = rslpos[16] = 50;
	lslpos[17] = rslpos[17] = 50;
	lslpos[18] = rslpos[18] = 50;
#endif

	//load_from(autoloadfile,1);

	setBandsFromSlpos();

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