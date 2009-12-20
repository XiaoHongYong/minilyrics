// SkinTextButton.cpp: implementation of the CMPSkinInfoTextCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinInfoTextCtrlEx.h"

UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrlEx,  _T("InfoTextEx"))

CMPSkinInfoTextCtrlEx::CMPSkinInfoTextCtrlEx()
{
	m_nMsgWant = 0;
}

CMPSkinInfoTextCtrlEx::~CMPSkinInfoTextCtrlEx()
{
	g_eventDispather.UnregisterHanlder(ET_UI_INFO_TEXT, this);
	g_eventDispather.UnregisterHanlder(ET_UI_LONG_ERROR_TEXT, this);
}

void CMPSkinInfoTextCtrlEx::OnCreate()
{
	CLyricShowObj::OnCreate();

	g_eventDispather.RegisterHanlder(ET_UI_INFO_TEXT, this);
	g_eventDispather.RegisterHanlder(ET_UI_LONG_ERROR_TEXT, this);

	// Do not proceed these messages.
	g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
	if (m_bUseBgImg)
		m_pSkin->UnregisterTimerObject(this);
}

void CMPSkinInfoTextCtrlEx::Draw(CRawGraph *pGraph)
{
	pGraph->SetFont(&m_font);

	int		nLineHeight = GetLineHeight();
	int		y = (m_rcObj.top + m_rcObj.bottom) / 2 - m_vText.size() * nLineHeight / 2;
	int		x;

	if (m_vText.size() > 1)
		x = m_rcObj.left + m_nXMargin;
	else
		x = AUTO_CAL_X;

	if (y < m_rcObj.top + m_nYMargin)
		y = m_rcObj.top + m_nYMargin;

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
		m_rcObj.top + m_nYMargin,
		m_rcObj.right - m_nXMargin,
		m_rcObj.bottom - m_nYMargin);

	pGraph->SetClipBoundBox(rcClip);

	for (size_t i = 0; i < m_vText.size(); i++)
	{
		DrawRow(pGraph, m_vText[i], x, y, LP_CUR_LINE);
		y += nLineHeight;
	}

	pGraph->RecoverClipBox();
}

void CMPSkinInfoTextCtrlEx::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_INFO_TEXT)
	{
		m_vUnWrapedText.clear();
		AppendInfoText(pEvent->strValue);
		WrapInfoText();
		Invalidate();
	}
	else if (pEvent->eventType == ET_UI_LONG_ERROR_TEXT)
	{
		V_Str		vStr;

		m_vUnWrapedText.clear();
		StrSplit(pEvent->strValue.c_str(), '\n', vStr);
		for (size_t i = 0; i < vStr.size(); i++)
		{
			TrimStr(vStr[i], '\r');
			AppendInfoText(vStr[i]);
		}

		WrapInfoText();
		Invalidate();
	}
	else
		CLyricShowObj::OnEvent(pEvent);
}

void CMPSkinInfoTextCtrlEx::OnLyrDrawContextChanged()
{

}

void CMPSkinInfoTextCtrlEx::OnPlayTimeChangedUpdate()
{

}

void CMPSkinInfoTextCtrlEx::OnSize(int cx, int cy)
{
	CUIObject::OnSize(cx, cy);

	WrapInfoText();
}

void CMPSkinInfoTextCtrlEx::AppendInfoText(const tstring &str)
{
	PLYRIC_FRAG	pFrag;

	pFrag = (PLYRIC_FRAG)new BYTE[sizeof(LYRIC_FRAG) + str.size() * sizeof(TCHAR)];

	pFrag->nBegTime = 0;
	pFrag->nEndTime = 0;
	pFrag->nDrawWidth = 0;
	_tcscpy_safe(pFrag->szLyric, str.size() + 1, str.c_str());
	pFrag->nLen = str.size();
	pFrag->dwAttrib = LSA_TEMP_TIME_BEG | LSA_TEMP_TIME_END;

	PML_ROW		pRow = new ML_ROW;
	pRow->nBegTime = 0;
	pRow->nEndTime = 0;
	pRow->nRowType = ML_ROW_LYRICS;
	pRow->szContent = NULL;
	pRow->vFrags.push_back(pFrag);

	m_vUnWrapedText.push_back(pRow);
}

bool WrapDisplayLyrics(VLyrLines &lyrLinesSrc, CMLyrLines &lyrLinesOut, CRawGraph *pGraph, int nWidthMax);

void CMPSkinInfoTextCtrlEx::WrapInfoText()
{
	CRawGraph		*pGraphics;

	pGraphics = m_pContainer->GetMemGraph();

	pGraphics->SetFont(&m_font);

	m_vText.clear();
	WrapDisplayLyrics(m_vUnWrapedText, m_vText, pGraphics, m_rcObj.Width() - m_nXMargin * 4);
}
