// Region.cpp: implementation of the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "base.h"
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
		gdk_region_destroy(m_rgn);
		m_rgn = NULL;
	}
}

bool CRegion::LoadFromFile(LPCTSTR szFile)
{
	GdkRectangle	rc;
	FILE			*fp;
	char			buffer[256];

	Destroy();

	fp = fopen(szFile, "r");
	if (!fp)
	{
		MLDebug("CRegion::LoadFromFile, Open file: %s FAILED!", szFile);
		return false;
	}

	if (!fgets(buffer, 255, fp))
	{
		fclose(fp);
		MLDebug("CRegion::LoadFromFile, Can't read file tag!");
		return false;
	}

	if (strncmp(buffer, "txtregion", 9) != 0)
	{
		MLDebug("CRegion::LoadFromFile, file tag not equal: %s!", buffer);
		fclose(fp);
		return false;
	}

	// next line is bound rect, here it's useless.
	fgets(buffer, 256, fp);

	m_rgn = gdk_region_new();

	while (fgets(buffer, 256, fp))
	{
		if (Scan4IntX(buffer, rc.x, rc.y, rc.width, rc.height))
			gdk_region_union_with_rect(m_rgn, &rc);
	}

	fclose(fp);
	return true;
}

bool CRegion::LoadTxtRgnFile(LPCTSTR szFile)
{
	FILE		*fp;
	char		buffer[256];
	GdkRectangle	rc;

	Destroy();

	fp = _tfopen(szFile, _T("r"));
	if (!fp)
	{
		// ERRLOG1(_T("CRegion::LoadFromFile, Open file: %s FAILED!"), szFile);
		return false;
	}

	if (!fgets(buffer, 255, fp))
	{
		fclose(fp);
		// ERRLOG0(_T("CRegion::LoadFromFile, Can't read file tag!"));
		return false;
	}

	if (strncmp(buffer, "txtregion", 9) != 0)
	{
		// ERRLOG0(_T("CRegion::LoadFromFile, file tag not equal!"));
		fclose(fp);
		return false;
	}


	m_rgn = gdk_region_new();

	// ignore bound box
	fgets(buffer, 256, fp);

	while (fgets(buffer, 256, fp))
	{
		if (Scan4IntX(buffer, rc.x, rc.y, rc.width, rc.height))
			gdk_region_union_with_rect(m_rgn, &rc);
	}

	fclose(fp);
	return true;
}

bool CRegion::CreateFromPolygon(POINT *points, int nPoints)
{
	m_rgn = gdk_region_polygon(points, nPoints, GDK_EVEN_ODD_RULE);
	return m_rgn != NULL;
}

bool CRegion::CreateRectRgn(int x1, int y1, int x2, int y2)
{
	GdkRectangle		rect;
	rect.x = x1;
	rect.y = y1;
	rect.width = x2 - x1;
	rect.height = y2 - y1;
	m_rgn = gdk_region_rectangle(&rect);
	return m_rgn != NULL;
}

bool CRegion::IsPointIn(int x, int y) const
{
	return gdk_region_point_in(m_rgn, x, y);
}

void CRegion::GetBoundsRect(CRect &rc)
{
	GdkRectangle	grc;
	gdk_region_get_clipbox(m_rgn, &grc);
	rc.SetRect(grc.x, grc.y, grc.x + grc.width, grc.y + grc.height);
}
