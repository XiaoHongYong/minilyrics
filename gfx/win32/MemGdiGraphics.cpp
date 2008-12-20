#include "StdAfx.h"
#include "gfx.h"
#include "MemGdiGraphics.h"

CMemGraphics::CMemGraphics(void)
{
}

CMemGraphics::~CMemGraphics(void)
{
}

bool CMemGraphics::Create(int cx, int cy, IMLGraphicsBase *pGraph)
{
	if (!m_memdc.Create(cx, cy, pGraph->GetHandle()))
		return false;

	if (!CGdiGraphics::Create(m_memdc.GetDC()))
		return false;

	return true;
}

bool CMemGraphics::Create(int cx, int cy, HDC hdc)
{
	if (!m_memdc.Create(cx, cy, hdc))
		return false;

	if (!CGdiGraphics::Create(m_memdc.GetDC()))
		return false;

	return true;
}

void CMemGraphics::Destroy()
{
	CGdiGraphics::Destroy();

	m_memdc.DeleteDC();
}

//////////////////////////////////////////////////////////////////////////
#ifndef _WINCE_
CMemGdiplusGraphics::CMemGdiplusGraphics(void)
{
}

CMemGdiplusGraphics::~CMemGdiplusGraphics(void)
{
}

bool CMemGdiplusGraphics::Create(int cx, int cy, IMLGraphicsBase *pGraph)
{
	if (!m_memdc.Create(cx, cy, pGraph->GetHandle()))
		return false;

	if (!CGdiplusGraphics::Create(m_memdc.GetDC()))
		return false;

	return true;
}
#endif
