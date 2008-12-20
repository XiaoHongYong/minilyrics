#include "../StdAfx.h"
#include "../gfx.h"
#include "MemGraphics.h"

CMemGraphics::CMemGraphics(void)
{
}

CMemGraphics::~CMemGraphics(void)
{
}

bool CMemGraphics::Create(int cx, int cy, GdkDrawable *pDrawable)
{
	return CreateMemDC(pDrawable, cx, cy);
}

void CMemGraphics::Destroy()
{
	DeleteDC();
}
