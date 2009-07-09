// MLBrush.cpp: implementation of the CMLPen class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLColor.h"
#include "MLPen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLPen::CMLPen()
{
	m_nWidth = 1;
	m_LineStyle = GDK_LINE_SOLID;
	m_CapStyle = GDK_CAP_BUTT;
	m_JionStyle = GDK_JOIN_BEVEL;
}

CMLPen::~CMLPen()
{
}

bool CMLPen::CreateSolidPen(int nWidth, MLColor &color)
{
	m_nWidth = nWidth;
	m_LineStyle = GDK_LINE_SOLID;

	m_clr.red = color.red;
	m_clr.blue = color.blue;
	m_clr.green = color.green;

	return true;
}
