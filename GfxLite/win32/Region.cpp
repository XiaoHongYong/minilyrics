// Region.cpp: implementation of the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Region.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegion::CRegion()
{
	m_rgn = NULL;
}

CRegion::~CRegion()
{
	Destroy();
}

void CRegion::Destroy()
{
	if (m_rgn)
	{
		DeleteObject(m_rgn);
		m_rgn = NULL;
	}
}

bool CRegion::LoadFromFile(LPCTSTR szFile)
{
	LPPOINT		lppt;
	DWORD		dwSize;
	DWORD		dwBytesRead;
	char		*lpsBuff = NULL;
	BOOL		bRet = FALSE;
	HANDLE		hFile;

	if (FileIsExtSame(szFile, _T(".trg")))
		return LoadTxtRgnFile(szFile);

	hFile = CreateFile(szFile, GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		ERRLOG1(_T("Load Region Data, Open file: %s FAILED!"), szFile);
		return false;
	}

	dwSize = GetFileSize(hFile, NULL);

	lpsBuff = new char[dwSize];

	if (!ReadFile(hFile, lpsBuff, dwSize, &dwBytesRead, NULL))
	{
		ERRLOG1(_T("Load Region Data, Read file: %s FAILED!"), szFile);

		goto EXIT_OUT;
	}

	if (lpsBuff[0] == 'R' && lpsBuff[1] == 'G')
	{
		m_rgn = ::ExtCreateRegion(NULL, dwSize - 2, (RGNDATA *)(lpsBuff + 2));
		bRet = m_rgn != NULL;

		goto EXIT_OUT;
	}
	else if (lpsBuff[0] == 'P' && lpsBuff[1] == 'L')
	{
		// The beginning point...
		lppt = (LPPOINT)(lpsBuff + 2);

		bRet = CreateFromPolygon(lppt,(dwSize - 2) / sizeof(POINT));

		goto EXIT_OUT;
	}
	else
	{
		ERRLOG1(_T("Load Region Data, file: %s is not a region Data File!"), szFile);
		goto EXIT_OUT;
	}
EXIT_OUT:
	if (lpsBuff)
		delete[] lpsBuff;

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return Tobool(bRet);
}

bool CRegion::LoadTxtRgnFile(LPCTSTR szFile)
{
	FILE		*fp;
	char		buffer[256];

	fp = _tfopen(szFile, _T("r"));
	if (!fp)
	{
		ERRLOG1(_T("CRegion::LoadFromFile, Open file: %s FAILED!"), szFile);
		return false;
	}

	if (!fgets(buffer, 255, fp))
	{
		fclose(fp);
		ERRLOG0(_T("CRegion::LoadFromFile, Can't read file tag!"));
		return false;
	}

	if (strncmp(buffer, "txtregion", 9) != 0)
	{
		ERRLOG0(_T("CRegion::LoadFromFile, file tag not equal!"));
		fclose(fp);
		return false;
	}

	vector<RECT>		vRc;
	RECT				rc;
	long				w, h;

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
		ERRLOG1(_T("CRegion::LoadFromFile, less than 2 rect in region data: %s!"), szFile);
		fclose(fp);
		return false;
	}

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

	delete[] prgnData;

	fclose(fp);
	return true;
}

bool CRegion::CreateFromPolygon(POINT *points, int nPoints)
{
#ifdef _WINCE_
	m_rgn = NULL;
#else
	m_rgn = ::CreatePolygonRgn(points, nPoints, ALTERNATE);
#endif
	return m_rgn != NULL;
}

bool CRegion::IsPointIn(int x, int y) const
{
	return Tobool(::PtInRegion(m_rgn, x, y));
}

void CRegion::GetBoundsRect(CRect &rc) const
{
	::GetRgnBox(m_rgn, &rc);
}
