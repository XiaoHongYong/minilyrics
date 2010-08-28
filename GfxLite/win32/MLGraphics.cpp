#include "stdafx.h"
#include "MLGraphics.h"

CMLGraphics::CMLGraphics()
{
	m_hdc = NULL;
}

CMLGraphics::~CMLGraphics()
{
}

void CMLGraphics::Attach(HDC hdc)
{
	m_hdc = hdc;
}

HDC CMLGraphics::Detach()
{
	HDC hdc = m_hdc;
	m_hdc = NULL;
	return m_hdc;
}
