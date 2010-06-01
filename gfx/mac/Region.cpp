// Region.cpp: implementation of the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "base.h"
#include "Region.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegion::CRegion()
{
}

CRegion::~CRegion()
{
	Destroy();
}

void CRegion::Destroy()
{
}

bool CRegion::LoadFromFile(LPCTSTR szFile)
{
	return false;
}

bool CRegion::LoadTxtRgnFile(LPCTSTR szFile)
{
	return false;
}

bool CRegion::CreateFromPolygon(POINT *points, int nPoints)
{
	return false;
}

bool CRegion::CreateRectRgn(int x1, int y1, int x2, int y2)
{
	return false;
}

bool CRegion::IsPointIn(int x, int y) const
{
	return false;
}

void CRegion::GetBoundsRect(CRect &rc)
{
}
