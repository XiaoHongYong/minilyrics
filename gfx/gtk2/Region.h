// Region.h: interface for the CRegion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_)
#define AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_

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

	virtual void GetBoundsRect(CRect &rc);

	bool CreateRectRgn(int x1, int y1, int x2, int y2);


	GdkRegion *GetHandle() const { return m_rgn; }

public:
	GdkRegion		*m_rgn;

};

#endif // !defined(AFX_REGION_H__844FB04B_9388_4936_B811_2298EF0E7388__INCLUDED_)
