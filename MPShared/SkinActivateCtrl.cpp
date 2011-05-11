#include "stdafx.h"
#include "SkinActivateCtrl.h"

UIOBJECT_CLASS_NAME_IMP(CSkinActivateCtrl, _T("ActivateCtrl"));

CSkinActivateCtrl::CSkinActivateCtrl()
{
	for (int i = 0; i < BG_CLR_MAX; i++)
		m_vClrBg[i].set(RGB(0, 0 , 0));

	m_vClrBg[0].set(0x80a7ff);
	m_vClrBg[1].set(0x00deff);
	m_vClrBg[2].set(0x1387fe);
	m_nCountBgClrs = 3;

	m_clrTxt.set(0xFFFFFF);

	m_formHeight.SetFormula(_T("25"));
}

CSkinActivateCtrl::~CSkinActivateCtrl()
{
}

void CSkinActivateCtrl::OnCreate()
{
	m_font.OnCreate(m_pSkin);

	CUIObject::OnCreate();
}

BOOL CSkinActivateCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return true;

	if (IsPropertyName(szProperty, _T("BgColors")))
	{
		V_Str vClrs;
		StrSplit(szValue, ',', vClrs);
		TrimStr(vClrs);
		if (vClrs.size() >= 1)
		{
			for (uint i = 0; i < vClrs.size(); i++)
				GetColorValue(m_vClrBg[i], vClrs[i].c_str());
			m_nCountBgClrs = vClrs.size();
		}
	}
	else if (!m_font.SetProperty(szProperty, szValue))
		return FALSE;

	return true;
}

void GradientFillImageHorz(PRAW_IMAGE_DATA pImage, CRect &rc, MLColor clrGradient[], int nClrCount);

void CSkinActivateCtrl::Draw(CRawGraph *pGraph)
{
	if (m_imgBg.IsValid())
	{
		PRAW_IMAGE_DATA image = m_imgBg.GetHandle();
		if (image->width != m_rcContent.Width() || image->height != m_rcContent.Height())
			m_imgBg.Destroy();
	}

	if (!m_imgBg.IsValid())
	{
		PRAW_IMAGE_DATA image = new RAW_IMAGE_DATA;
		if (!image->Create(m_rcContent.Width(), m_rcContent.Height(), 24))
		{
			delete image;
			return;
		}
		GradientFillImageHorz(image, CRect(0, 0, image->width, image->height), m_vClrBg, m_nCountBgClrs);
		m_imgBg.Attach(image);
	}

	m_imgBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, BPM_COPY);

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	pGraph->SetTextColor(m_font.GetTextColor());

	pGraph->DrawText(m_strText.c_str(), m_strText.size(), &m_rcContent, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
}
