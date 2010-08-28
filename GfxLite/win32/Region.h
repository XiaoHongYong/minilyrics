// Region.h: interface for the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_)
#define AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRegion  
{
public:
	CRegion();
	virtual ~CRegion();

	bool IsValid() const { return m_rgn != NULL; }

	void Destroy();

	//
	// Load a region from a region define file.
	// the region file is a text file, which includes the rects compose the region.
	//
	virtual bool LoadFromFile(LPCTSTR szFile);

	bool LoadTxtRgnFile(LPCTSTR szFile);

	virtual bool CreateFromPolygon(POINT *points, int nPoints);

	virtual bool IsPointIn(int x, int y) const;

	virtual void GetBoundsRect(CRect &rc) const;

	bool CreateRectRgn(int x1, int y1, int x2, int y2)
		{ m_rgn = ::CreateRectRgn(x1, y1, x2, y2); return m_rgn != NULL; }


	HRGN GetHandle() const { return m_rgn; }

public:
	HRGN			m_rgn;

};

#endif // !defined(AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_)
