// MLBrush.cpp: implementation of the CMLBrush class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLColor.h"
#include "MLBrush.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLBrush::CMLBrush()
{

}

CMLBrush::~CMLBrush()
{

}

bool CMLBrush::CreateSolidBrush(const MLColor &color)
{
	m_color.blue = color.blue;
	m_color.green = color.green;
	m_color.red = color.red;
	m_color.pixel = 0;

	return true;
}
