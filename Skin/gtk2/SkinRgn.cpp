// SkinRgn.cpp: implementation of the CSkinRgn class.
//
//////////////////////////////////////////////////////////////////////

#include ".h"
#include "SkinRgn.h"

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
/*
	assert(m_rgn);

	LPRGNDATA	lpRgnData;

	if (m_rgn == NULL)
	{
		ERRLOG0(_T("Null Rgn handle! can't save to file!"));
		return FALSE;
	}

	lpRgnData = (LPRGNDATA)new BYTE[m_nRgnDataSize];
	assert(lpRgnData);

	memcpy(lpRgnData, m_lpRgnDataOrg, m_nRgnDataSize);
	
	int			nNewStartX1;
	int			nNewStartX2;
	int			nNewStartY1;
	int			nNewStartY2;
	RECT		*pRc;

	pRc = (RECT *)lpRgnData->Buffer;

	nNewStartX1 = (nWidth - (m_nScaleX2 - m_nScaleX1)) / 2;
	nNewStartX2 = nWidth - (m_nWidthOrg - m_nScaleX2);

	nNewStartY1 = (nHeight - (m_nScaleY2 - m_nScaleY1)) / 2;
	nNewStartY2 = nHeight - (m_nHeightOrg - m_nScaleY2);

	// recalculate set region's rectangle.
	for (int i = 0; i < (int)lpRgnData->rdh.nCount; i++)
	{
		if (pRc[i].left > m_nScaleX1 && pRc[i].left <= m_nScaleX2)
			pRc[i].left = nNewStartX1 + pRc[i].left - m_nScaleX1;
		else if (pRc[i].left > m_nScaleX2)
			pRc[i].left = nNewStartX2 + pRc[i].left - m_nScaleX2;

		if (pRc[i].right > m_nScaleX1 && pRc[i].right <= m_nScaleX2)
			pRc[i].right = nNewStartX1 + pRc[i].right - m_nScaleX1;
		else if (pRc[i].right > m_nScaleX2)
			pRc[i].right = nNewStartX2 + pRc[i].right - m_nScaleX2;

		if (pRc[i].top > m_nScaleY1 && pRc[i].top <= m_nScaleY2)
			pRc[i].top = nNewStartY1 + pRc[i].top - m_nScaleY1;
		else if (pRc[i].top > m_nScaleY2)
			pRc[i].top = nNewStartY2 + pRc[i].top - m_nScaleY2;

		if (pRc[i].bottom > m_nScaleY1 && pRc[i].bottom <= m_nScaleY2)
			pRc[i].bottom = nNewStartY1 + pRc[i].bottom - m_nScaleY1;
		else if (pRc[i].bottom > m_nScaleY2)
			pRc[i].bottom = nNewStartY2 + pRc[i].bottom - m_nScaleY2;
	}

	// release old.
	Destroy();

	m_rgn = ::ExtCreateRegion(NULL, m_nRgnDataSize, (RGNDATA *)(lpRgnData));

	delete lpRgnData;

	return true;*/

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

	Destroy();

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

	GdkRectangle	rc;
	int				w, h;

	if (!fgets(buffer, CountOf(buffer), fp))
		return false;
	if (!Scan4IntX(buffer, m_nStretchStartX, m_nStretchEndX, m_nStretchStartY, m_nStretchEndY))
		return false;

	m_rgn = gdk_region_new();

	// next line is bound rect, here it's useless.
	fgets(buffer, 256, fp);

	while (fgets(buffer, 256, fp))
	{
		if (Scan4IntX(buffer, rc.x, rc.y, rc.width, rc.height))
			gdk_region_union_with_rect(m_rgn, &rc);
	}

	while (fgets(buffer, 256, fp))
	{
		if (Scan4IntX(buffer, rc.x, rc.y, w, h))
			gdk_region_union_with_rect(m_rgn, &rc);
	}

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

	return false;
}

void CSkinRgn::SetScaleWidthOrg(int nWidthOrg)
{
	m_nWidthOrg = nWidthOrg;
}

void CSkinRgn::SetScaleHeightOrg(int nHeightOrg)
{
	m_nHeightOrg = nHeightOrg;
}
