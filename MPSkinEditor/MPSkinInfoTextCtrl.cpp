// MPSkinInfoCtrl.cpp: implementation of the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinInfoTextCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrl, _T("InfoText"))

CMPSkinInfoTextCtrl::CMPSkinInfoTextCtrl()
{
	m_strText = _T("Artist - Title - ZikiPlayer");
	m_nMsgWant = UO_MSG_WANT_LBUTTON;
	m_nTimerIDHideInfo = 0;
}

CMPSkinInfoTextCtrl::~CMPSkinInfoTextCtrl()
{
	if (m_nTimerIDHideInfo != 0)
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);
}
void CMPSkinInfoTextCtrl::OnCreate()
{
	CSkinScrollText::OnCreate();
}

void CMPSkinInfoTextCtrl::OnTimer(int nId)
{
	if (nId == m_nTimerIDHideInfo)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);
		m_nTimerIDHideInfo = 0;
		Invalidate();
	}
	else
		CSkinScrollText::OnTimer(nId);
}

void CMPSkinInfoTextCtrl::Draw(CRawGraph *pGraph)
{
	CSkinScrollText::Draw(pGraph);
// 	if (m_imgBk.IsValid())
// 	{
// 		if (m_BltMode == BLT_COPY)
// 		{
// 			m_imgBk.Blt(pGraph, m_rcObj.left, m_rcObj.top);
// 		}
// 		else if (m_BltMode == BLT_TILE)
// 		{
// 			m_imgBk.TileBlt(pGraph, m_rcObj.left, m_rcObj.top,
// 				m_rcObj.Width(), m_rcObj.Height());
// 		}
// 		else
// 		{
// 			m_imgBk.StretchBlt(pGraph, m_rcObj.left, m_rcObj.top,
// 						m_rcObj.Width(), m_rcObj.Height());
// 		}
// 	}
// 
// 	if (!m_font.IsValid())
// 	{
// 		m_font.Create(m_strFontName.c_str(), m_strFontName.c_str(), m_nFontHeight, 
// 			m_bFontBold ? FW_BOLD : FW_NORMAL, false, false);
// 	}
// 
// 	pGraph->SetFont(&m_font);
// 	pGraph->SetTextColor(m_clrText);
// 
// 	CRect		rc = m_rcObj;
// 
// 	rc.left += m_nLeftMargin;
// 	pGraph->DrawText(m_strText.c_str(), (int)m_strText.size(), &rc);
}

