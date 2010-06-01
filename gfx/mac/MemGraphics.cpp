#include "StdAfx.h"
#include "gfx.h"
#include "MemGraphics.h"

CMemGraphics::CMemGraphics(void)
{
}

CMemGraphics::~CMemGraphics(void)
{
}

bool CMemGraphics::Create(int cx, int cy, IMLGraphicsBase *pGraph)
{
}

void CMemGraphics::Destroy()
{
	DeleteDC();
}
