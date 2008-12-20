#ifndef _MEMGDI_GRAPHICS_INC_
#define _MEMGDI_GRAPHICS_INC_

#pragma once

class CMemGraphics :
	public CGdiGraphics
{
public:
	CMemGraphics(void);
	virtual ~CMemGraphics(void);

public:
	bool Create(int cx, int cy, IMLGraphicsBase *pGraph);
	bool Create(int cx, int cy, HDC hdc);

	void Destroy();

	int Width() const { return m_memdc.Width(); }
	int Height() const { return m_memdc.Height(); }

protected:
	CMemDC		m_memdc;

};

#ifndef _WINCE_
class CMemGdiplusGraphics :
	public CGdiplusGraphics
{
public:
	CMemGdiplusGraphics(void);
	virtual ~CMemGdiplusGraphics(void);

public:
	bool Create(int cx, int cy, IMLGraphicsBase *pGraph);

	int Width() const { return m_memdc.Width(); }
	int Height() const { return m_memdc.Height(); }

protected:
	CMemDC		m_memdc;

};
#endif

#endif // _MEMGDI_GRAPHICS_INC_
