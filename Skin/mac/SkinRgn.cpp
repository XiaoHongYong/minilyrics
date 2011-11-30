// SkinRgn.cpp: implementation of the CSkinRgn class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../SkinRgn.h"

#define MAX_RGN_FILE_SIZE (1024 * 20)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinRgn::CSkinRgn()
{
	m_nStretchStartX = 0;
	m_nStretchEndX = 0;
	m_nStretchStartY = 0;
	m_nStretchEndY = 0;

	m_nWidthOrg = 0;
	m_nHeightOrg = 0;
}

CSkinRgn::~CSkinRgn()
{
}

BOOL CSkinRgn::StretchRgn(double dbScaleRadio, int nWidth, int nHeight)
{
	int			nStretchStartX = (int)(m_nStretchStartX * dbScaleRadio);
	int			nStretchEndX = (int)(m_nStretchEndX * dbScaleRadio);
	int			nStretchStartY = (int)(m_nStretchStartY * dbScaleRadio);
	int			nStretchEndY = (int)(m_nStretchEndY * dbScaleRadio);
	int			nWidthOrg = (int)(m_nWidthOrg * dbScaleRadio);
	int			nHeightOrg = (int)(m_nHeightOrg * dbScaleRadio);
	int			nNewStartX;
	int			nNewEndX;
	int			nNewStartY;
	int			nNewEndY;
	RECT		*pRc;
	int			i;


	return FALSE;
}

void CSkinRgn::SetScaleX1(int nScaleX1)
{
	m_nStretchStartX = nScaleX1;
}

void CSkinRgn::SetScaleX2(int nScaleX2)
{
	m_nStretchEndX = nScaleX2;
}

void CSkinRgn::SetScaleY1(int nScaleY1)
{
	m_nStretchStartY = nScaleY1;
}

void CSkinRgn::SetScaleY2(int nScaleY2)
{
	m_nStretchEndY = nScaleY2;
}


bool CSkinRgn::LoadTxtRgn2File(LPCTSTR szFile)
{

	return true;
}

bool CSkinRgn::LoadRgn(LPCTSTR szFile)
{


	return true;
}

void CSkinRgn::SetScaleWidthOrg(int nWidthOrg)
{
	m_nWidthOrg = nWidthOrg;
}

void CSkinRgn::SetScaleHeightOrg(int nHeightOrg)
{
	m_nHeightOrg = nHeightOrg;
}
