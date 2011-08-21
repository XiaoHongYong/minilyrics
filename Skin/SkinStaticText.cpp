/********************************************************************
	Created  :	2001Äê12ÔÂ15ÈÕ 0:42:06
	FileName :	SkinStaticText.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "Skin.h"
#include "SkinStaticText.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinStaticText, _T("Text"))

CSkinStaticText::CSkinStaticText()
{
	m_nLeftMargin = 2;
	m_bCaptionText = false;

	m_BltMode = BLT_COPY;
	m_dwAlignText = AT_LEFT;

	m_bPaintBg = false;
	m_clrBg.set(0, 0, 0);
}

CSkinStaticText::~CSkinStaticText()
{

}

void CSkinStaticText::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);
}

int CountOfChar(LPCTSTR str, TCHAR ch)
{
	int count = 0;
	while (*str)
	{
		if (*str == ch)
			count++;
		str++;
	}
	return count;
}

void CSkinStaticText::OnMeasureSizeByContent()
{
	CRawGraph *pGraph = m_pContainer->GetMemGraph();

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	if (m_layoutParams & (LAYOUT_HEIGHT_WRAP_CONENT | LAYOUT_WIDTH_WRAP_CONENT))
	{
		int nLines = CountOfChar(m_strText.c_str(), '\n') + 1;
		const int TEXT_MARGIN = 8;
		SIZE size = { 0 };
		if (nLines > 1 && IsFlagSet(m_layoutParams, LAYOUT_WIDTH_WRAP_CONENT))
		{
			int widthMax = 0;
			V_Str vLines;
			StrSplit(m_strText.c_str(), '\n', vLines);
			for (uint i = 0; i < vLines.size(); i++)
			{
				if (pGraph->GetTextExtentPoint32(vLines[i].c_str(), vLines[i].size(), &size)
					&& size.cx > widthMax)
					widthMax = size.cx;
			}
		}
		else
			pGraph->GetTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size);

		if (m_layoutParams & LAYOUT_HEIGHT_WRAP_CONENT)
			m_rcObj.bottom = m_rcObj.top + (size.cy + 2) * nLines + TEXT_MARGIN / 2;

		if (m_layoutParams & LAYOUT_WIDTH_WRAP_CONENT)
			m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
	}
}

void CSkinStaticText::OnAdjustHue(float hue, float saturation, float luminance)
{
	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

bool CSkinStaticText::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nId != UID_INVALID)
		m_pSkin->PostCustomCommandMsg(m_nId);

	return true;
}

void CSkinStaticText::Draw(CRawGraph *pGraph)
{
	if (m_imgBk.IsValid())
	{
		if (m_BltMode == BLT_COPY)
		{
			m_imgBk.Blt(pGraph, m_rcObj.left, m_rcObj.top);
		}
		else if (m_BltMode == BLT_TILE)
		{
			m_imgBk.TileBlt(pGraph, m_rcObj.left, m_rcObj.top,
				m_rcObj.Width(), m_rcObj.Height());
		}
		else
		{
			m_imgBk.StretchBlt(pGraph, m_rcObj.left, m_rcObj.top,
						m_rcObj.Width(), m_rcObj.Height());
		}
	}
	else
	{
		if (m_bPaintBg && !m_bUseParentBg)
			pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_clrBg);
	}

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	CRect		rc = m_rcObj;
	LPCTSTR		szText = NULL;

	rc.left += m_nLeftMargin;

	if (m_bCaptionText)
		szText = m_pSkin->GetCaptionText();
	else
		szText = m_strText.c_str();

	MLColor clrText = m_font.GetTextColor(m_bEnable);

	if (m_font.IsOutlined())
	{
		pGraph->DrawTextOutlined(szText, (int)_tcslen(szText), &rc, 
			clrText, m_font.GetColorOutlined(), m_dwAlignText | DT_END_ELLIPSIS | DT_PREFIX_TEXT);
	}
	else
	{
		pGraph->SetTextColor(clrText);
		pGraph->DrawText(szText, (int)_tcslen(szText), &rc, m_dwAlignText | DT_END_ELLIPSIS | DT_PREFIX_TEXT);
	}
}

BOOL CSkinStaticText::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strImgBkFile = szValue;
		m_imgBk.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_IMAGERECT) == 0)
	{
		if (!GetRectValue(szValue, m_imgBk))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("BltMode")) == 0)
	{
		m_BltMode = BltModeFromStr(szValue);
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (_tcsicmp(szProperty, _T("PaintBg")) == 0)
	{
		m_bPaintBg = IsTRUE(szValue);
	}
	else if (_tcsicmp(szProperty, _T("BgColor")) == 0)
	{
		GetColorValue(m_clrBg, szValue);
	}
	else if (_tcsicmp(szProperty, _T("LeftMargin")) == 0)
	{
		m_nLeftMargin = _ttoi(szValue);
	}
	else if (IsPropertyName(szProperty, _T("CaptionText")))
		m_bCaptionText = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("AlignText")) == 0)
	{
		m_dwAlignText = AlignTextFromStr(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinStaticText::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strImgBkFile.c_str(), m_imgBk, !m_strImgBkFile.empty());

	listProperties.AddPropBltMode(_T("BltMode"), m_BltMode);
	m_font.EnumProperties(listProperties);
	listProperties.AddPropInt(_T("LeftMargin"), m_nLeftMargin);
	listProperties.AddPropStr(_T("Text"), m_strText.c_str(), !m_strText.empty());
	listProperties.AddPropBoolStr(_T("PaintBg"), m_bPaintBg, m_bPaintBg);
	listProperties.AddPropColor(_T("BgColor"), m_clrBg, m_bPaintBg || m_clrBg.get() != 0);

	tstring		str;
	AlignTextToStr(m_dwAlignText, str);
	listProperties.AddPropStr(_T("AlignText"), str.c_str());
}
#endif // _SKIN_EDITOR_
