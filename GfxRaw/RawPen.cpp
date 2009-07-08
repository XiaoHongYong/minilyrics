// RawPen.cpp: implementation of the CRawPen class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RawPen.h"

//////////////////////////////////////////////////////////////////////////

CRawPen::CRawPen()
{
	m_nWidth = 1;
}

CRawPen::~CRawPen()
{
}

bool CRawPen::CreateSolidPen(int nWidth, MLColor &color)
{
	m_nWidth = nWidth;
	m_clrPen = color;

	return true;
}
