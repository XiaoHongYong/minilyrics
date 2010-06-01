#include "StdAfx.h"
#include "gfx.h"
#include "MLGraphicsFactory.h"

MLGRAPHICS_TYPE		CMLGraphicsFactory::m_graphicsType = MGT_WIN32GDI;

CMLGraphicsFactory::CMLGraphicsFactory(void)
{
}

CMLGraphicsFactory::~CMLGraphicsFactory(void)
{
}

IMLGraphicsBase *CMLGraphicsFactory::CreateMLGraphics(HDC hdc, MLGRAPHICS_TYPE graphType)
{
#ifdef _WIN32
#ifdef _WINCE_
	CGdiGraphics		*pGraph;

	pGraph = new CGdiGraphics();
	pGraph->Create(hdc);
	return pGraph;
#else
	if ((graphType == MGT_AUTO && m_graphicsType == MGT_GDIPLUS)
		|| graphType == MGT_GDIPLUS)
	{
		CGdiplusGraphics		*pGraph;

		pGraph = new CGdiplusGraphics();
		pGraph->Create(hdc);
		return pGraph;
	}
	else
	{
		CGdiGraphics		*pGraph;

		pGraph = new CGdiGraphics();
		pGraph->Create(hdc);
		return pGraph;
	}
#endif
#endif

#ifdef _LINUX_GTK2
	return new CMLGraphics();
#endif

#ifdef _MAC
	return new CMLGraphics();
#endif

	return NULL;
}

void CMLGraphicsFactory::ReleaseMLGraphics(IMLGraphicsBase *pGraph)
{
	delete pGraph;
}
