// SkinFadeArea.cpp: implementation of the CSkinFadeArea class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinFadeArea.h"


UIOBJECT_CLASS_NAME_IMP(CSkinFadeArea, _T("FadeArea"))


CSkinFadeArea::CSkinFadeArea()
{
}


CSkinFadeArea::~CSkinFadeArea()
{
}


void CSkinFadeArea::Draw(CRawGraph *pGraph)
{
	//
	// Adjust the alpha of the eara that CSkinFadeArea covers.
	//
	if (!m_pSkin->GetEnableTranslucencyLayered())
		return;

	if ((int)m_pSkin->m_nTranslucencyAlphaDynamic == m_pSkin->m_nTranslucencyAlphaOnActive)
		return;

	pGraph->MultiplyAlpha((byte)m_pSkin->m_nTranslucencyAlphaDynamic, &m_rcObj);
}
