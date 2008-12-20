// SkinRgn.cpp: implementation of the CSkinRgn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../SkinRgn.h"

#define MAX_RGN_FILE_SIZE (1024 * 20)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinRgn::CSkinRgn()
{
	m_lpRgnDataOrg = NULL;
	m_nRgnDataSize = 0;

	m_nStretchStartX = 0;
	m_nStretchEndX = 0;
	m_nStretchStartY = 0;
	m_nStretchEndY = 0;

	m_nWidthOrg = 0;
	m_nHeightOrg = 0;
}

CSkinRgn::~CSkinRgn()
{
	if (m_lpRgnDataOrg != NULL)
		delete m_lpRgnDataOrg;
}

BOOL CSkinRgn::StretchRgn(double dbScaleRadio, int nWidth, int nHeight)
{
	LPRGNDATA	lpRgnData;
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


	if (m_rgn == NULL)
		return FALSE;

	lpRgnData = (LPRGNDATA)new BYTE[m_nRgnDataSize];
	assert(lpRgnData);

	memcpy(lpRgnData, m_lpRgnDataOrg, m_nRgnDataSize);

	if (dbScaleRadio != 1.0)
	{
		// scale all region data.
		pRc = (RECT *)lpRgnData->Buffer;
		for (i = 0; i < (int)lpRgnData->rdh.nCount; i++)
		{
			pRc[i].left = (long)(pRc[i].left * dbScaleRadio);
			pRc[i].right = (long)(pRc[i].right * dbScaleRadio);
			pRc[i].top = (long)(pRc[i].top * dbScaleRadio);
			pRc[i].bottom = (long)(pRc[i].bottom * dbScaleRadio);
		}
	}

	nNewStartX = (nWidth - (nStretchEndX - nStretchStartX)) / 2;
	nNewEndX = nWidth - (nWidthOrg - nStretchEndX);

	nNewStartY = (nHeight - (nStretchEndY - nStretchStartY)) / 2;
	nNewEndY = nHeight - (nHeightOrg - nStretchEndY);

	// recalculate set region's rectangle.
	pRc = (RECT *)lpRgnData->Buffer;
	for (i = 0; i < (int)lpRgnData->rdh.nCount; i++)
	{
		if (pRc[i].left > nStretchStartX && pRc[i].left <= nStretchEndX)
			pRc[i].left = nNewStartX + pRc[i].left - nStretchStartX;
		else if (pRc[i].left > nStretchEndX)
			pRc[i].left = nNewEndX + pRc[i].left - nStretchEndX;

		if (pRc[i].right > nStretchStartX && pRc[i].right <= nStretchEndX)
			pRc[i].right = nNewStartX + pRc[i].right - nStretchStartX;
		else if (pRc[i].right > nStretchEndX)
			pRc[i].right = nNewEndX + pRc[i].right - nStretchEndX;

		if (pRc[i].top > nStretchStartY && pRc[i].top <= nStretchEndY)
			pRc[i].top = nNewStartY + pRc[i].top - nStretchStartY;
		else if (pRc[i].top > nStretchEndY)
			pRc[i].top = nNewEndY + pRc[i].top - nStretchEndY;

		if (pRc[i].bottom > nStretchStartY && pRc[i].bottom <= nStretchEndY)
			pRc[i].bottom = nNewStartY + pRc[i].bottom - nStretchStartY;
		else if (pRc[i].bottom > nStretchEndY)
			pRc[i].bottom = nNewEndY + pRc[i].bottom - nStretchEndY;
	}

	// release old.
	Destroy();

	m_rgn = ::ExtCreateRegion(NULL, m_nRgnDataSize, (RGNDATA *)(lpRgnData));

	delete lpRgnData;

	return true;
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
	FILE		*fp;
	char		buffer[256];

	fp = _tfopen(szFile, _T("r"));
	if (!fp)
		return false;

	if (!fgets(buffer, 255, fp))
	{
		fclose(fp);
		return false;
	}

	if (strncmp(buffer, "txtrgn2", 7) != 0)
	{
		ERRLOG0(_T("CSkinRg::LoadTxtRgn2File, file tag should be: txtrgn2"));
		fclose(fp);
		return false;
	}

	vector<RECT>		vRc;
	RECT				rc;
	long				w, h;

	if (!fgets(buffer, CountOf(buffer), fp))
		return false;
	if (!Scan4IntX(buffer, m_nStretchStartX, m_nStretchEndX, m_nStretchStartY, m_nStretchEndY))
		return false;

	while (fgets(buffer, 256, fp))
	{
		if (Scan4IntX(buffer, rc.left, rc.top, w, h))
		{
			rc.right = rc.left + w;
			rc.bottom = rc.top + h;
			vRc.push_back(rc);
		}
	}

	if (vRc.size() < 2)
	{
		ERRLOG1(_T("less than 2 rect in region data: %s!"), szFile);
		fclose(fp);
		return false;
	}

	// vRc[0] : region stretch x and y
	// stretch start x, stretch end x, stretch start y, stretch end y
	m_nWidthOrg = vRc[0].right;
	m_nHeightOrg = vRc[0].bottom;

	int				nSize;
	RGNDATA			*prgnData;
	RECT			*prc;

	nSize = sizeof(RGNDATA) + sizeof(RECT) * vRc.size() - 1;
	prgnData = (RGNDATA*)new char[nSize];

	RGNDATAHEADER	&rdh = prgnData->rdh;
	rdh.dwSize = sizeof(prgnData->rdh);
	rdh.iType = RDH_RECTANGLES;
	rdh.nCount = vRc.size() - 1;
	rdh.nRgnSize = sizeof(RECT) * rdh.nCount;
	rdh.rcBound = vRc[0];

	prc = (RECT*)prgnData->Buffer;
	for (int i = 1; i < (int)vRc.size(); i++)
	{
		*prc = vRc[i];
		prc++;
	}

	m_rgn = ExtCreateRegion(NULL, nSize, prgnData);

	delete prgnData;

	fclose(fp);
	return true;
}

bool CSkinRgn::LoadRgn(LPCTSTR szFile)
{
	int			nRet;

	if (FileIsExtSame(szFile, _T(".tr2")))
	{
		if (!LoadTxtRgn2File(szFile))
			return false;
	}
	else if (!CRegion::LoadFromFile(szFile))
		return false;

	// Get the region's data first
	m_nRgnDataSize = ::GetRegionData(m_rgn, 0, NULL);
	if ((DWORD)m_nRgnDataSize > MAX_RGN_FILE_SIZE)
		return false;

	if (m_lpRgnDataOrg)
	{
		delete[] m_lpRgnDataOrg;
		m_lpRgnDataOrg = NULL;
	}
	m_lpRgnDataOrg = (LPRGNDATA )new BYTE[m_nRgnDataSize];
	assert(m_lpRgnDataOrg);

	nRet = ::GetRegionData(m_rgn, m_nRgnDataSize, m_lpRgnDataOrg);
	if (nRet == ERROR || nRet == NULLREGION)
	{
		// if get region data failed, delete the object
		delete m_lpRgnDataOrg;

		m_lpRgnDataOrg = NULL;
		m_nRgnDataSize = 0;

		Destroy();

		return false;
	}

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
