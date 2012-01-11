#include "stdafx.h"
#include "SkinActivateCtrl.h"
#include "DlgAbout.h"

UIOBJECT_CLASS_NAME_IMP(CSkinActivateCtrl, _T("ActivateCtrl"));

#define BTN_MARGIN_RIGHT		30
#define SZ_ID_ACTIVATE_CLOSE	_T("ID_CLOSE_ACTIVATE_NOTICE")

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
	m_nDuration = 15;
	m_nIdOverlappedCtrl = UID_INVALID;
}

CSkinActivateCtrl::~CSkinActivateCtrl()
{
}

void CSkinActivateCtrl::OnCreate()
{
	CSkinContainer::OnCreate();

	m_font.OnCreate(m_pSkin);

	m_pCloseBtn = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, CSkinButton::ClassName(), this);
	m_pCloseBtn->SetProperty(SZ_PN_RECT, _T("w-20,9,9,9"));
	m_pCloseBtn->SetProperty(SZ_PN_IMAGE, _T("close_small.png"));
	m_pCloseBtn->SetProperty(SZ_PN_IMAGE_SIZE, _T("9,9"));
	m_pCloseBtn->SetProperty(SZ_PN_IMAGE_POS, _T("0,0"));
	m_pCloseBtn->SetProperty(_T("ImageSelPos"), _T("0,9"));
	m_pCloseBtn->SetProperty(_T("ImageFocusPos"), _T("0,18"));
	m_pCloseBtn->SetProperty(SZ_PN_ID, SZ_ID_ACTIVATE_CLOSE);

	AddUIObject(m_pCloseBtn);
	m_pCloseBtn->SetVisible(false, false);

	m_pTimerText = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, CSkinStaticText::ClassName(), this);
	m_pTimerText->SetProperty(SZ_PN_RECT, _T("w-20,9,15,20"));
	m_pTimerText->SetText(CStrPrintf(_T("%d"), m_nDuration).c_str());
	tstring	strColor;
	ColorToStr(m_font.GetTextColor(), strColor);
	m_pTimerText->SetProperty(_T("TextColor"), strColor.c_str());
	AddUIObject(m_pTimerText);

	m_nIdTimerText = m_pSkin->RegisterTimerObject(this, 1000);
	m_nCountDown = m_nDuration;

	m_Cursor.LoadStdCursor(CCursor::C_HAND);
}

bool CSkinActivateCtrl::OnCustomCommand(int nId)
{
	ASSERT(m_pCloseBtn);
	if (m_pCloseBtn && nId == m_pCloseBtn->GetID())
	{
		SetVisible(false, false);
		if (m_nIdOverlappedCtrl != UID_INVALID)
			m_pSkin->SetUIObjectVisible(m_nIdOverlappedCtrl, true, false);
		m_pContainer->GetParent()->RecalculateUIObjSizePos(m_pContainer);
		m_pContainer->Invalidate();
		return true;
	}

	return false;
}

void CSkinActivateCtrl::OnTimer(int nId)
{
	if (nId != m_nIdTimerText)
		return;

	m_nCountDown--;
	if (m_nCountDown < 0)
	{
		m_nCountDown = m_nDuration;
		m_pTimerText->SetVisible(false, false);
		m_pCloseBtn->SetVisible(true, false);

		m_pSkin->UnregisterTimerObject(this, m_nIdTimerText);
	}
	else
		m_pTimerText->SetText(CStrPrintf(_T("%d"), m_nCountDown).c_str());

	Invalidate();
}

bool CSkinActivateCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsInReactArea(point))
	{
		MLWidget::SetCursor(m_Cursor);

		ShowActivateDialog(m_pSkin);
		OnCustomCommand(m_pCloseBtn->GetID());
		return true;
	}

	return CSkinContainer::OnLButtonDown(nFlags, point);
}

bool CSkinActivateCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (IsInReactArea(point))
		MLWidget::SetCursor(m_Cursor);

	return CSkinContainer::OnLButtonDown(nFlags, point);
}

bool CSkinActivateCtrl::OnMouseMove(CPoint point)
{
	if (IsInReactArea(point))
		MLWidget::SetCursor(m_Cursor);

	return CSkinContainer::OnMouseMove(point);
}

bool CSkinActivateCtrl::IsInReactArea(POINT pt)
{
	CRect rc = m_rcContent;
	rc.right -= BTN_MARGIN_RIGHT;

	return Tobool(rc.PtInRect(pt));
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
	else if (IsPropertyName(szProperty, _T("Duration")))
		m_nDuration = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("OverlappedCtrl")))
		m_nIdOverlappedCtrl = GetIDByName(szValue);
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
		CRect rcImage(0, 0, image->width, image->height);
		GradientFillImageHorz(image, rcImage, m_vClrBg, m_nCountBgClrs);
		m_imgBg.Attach(image);
	}

	m_imgBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, BPM_COPY);

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	pGraph->SetTextColor(m_font.GetTextColor());

	CRect rc = m_rcContent;
	rc.right -= BTN_MARGIN_RIGHT;
	pGraph->DrawText(m_strText.c_str(), m_strText.size(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

	CSkinContainer::Draw(pGraph);
}
