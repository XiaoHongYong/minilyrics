// MLBrush.cpp: implementation of the CMLPen class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLColor.h"
#include "MLPen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLPen::CMLPen()
{
	m_pen = NULL;
}

CMLPen::~CMLPen()
{
	if (m_pen)
		DeleteObject(m_pen);
}

bool CMLPen::CreateSolidPen(int nWidth, const MLColor &color)
{
	if (m_pen)
		Destroy();

	m_pen = ::CreatePen(PS_SOLID, nWidth, color.get());

	return m_pen != NULL;
}
