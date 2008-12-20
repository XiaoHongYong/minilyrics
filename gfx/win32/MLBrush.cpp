// MLBrush.cpp: implementation of the CMLBrush class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLColor.h"
#include "MLBrush.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLBrush::CMLBrush()
{
	m_brush = NULL;
}

CMLBrush::~CMLBrush()
{
	if (m_brush)
		DeleteObject(m_brush);
}

bool CMLBrush::CreateSolidBrush(const MLColor &color)
{
	if (m_brush)
		Destroy();

	m_brush = ::CreateSolidBrush(color.get());
	m_color = color;

	return m_brush != NULL;
}

void CMLBrush::Destroy()
{
	if (m_brush)
		DeleteObject(m_brush);
	m_brush = NULL;
	m_color.setAlpha(255);
}
