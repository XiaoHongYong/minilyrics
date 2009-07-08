/********************************************************************
	Created  :	2001?12?15? 0:28:39
	FileName :	UIObject.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MLSkin.h"
#include "UIObject.h"


ID_TO_STRING	__idszBltMode[] = 
{
	{ BLT_STRETCH, _T("stretch") },
	{ BLT_TILE, _T("tile") },
	{ BLT_COPY, _T("copy") },
	{ 0, NULL }
};

BLT_MODE BltModeFromStr(LPCTSTR szBltMode)
{
	return (BLT_MODE)StringToID(__idszBltMode, szBltMode, BLT_COPY);
}

ID_TO_STRING	__idszBlendPixMode[] = 
{
	{ BPM_BLEND, _T("alpha_blend") },
	{ BPM_MULTIPLY, _T("alpha_mask") },
	{ BPM_COPY, _T("copy") },
	{ 0, NULL }
};

BlendPixMode BlendPixModeFromStr(LPCTSTR szBpm)
{
	return (BlendPixMode)StringToID(__idszBlendPixMode, szBpm, BPM_COPY);
}

ID_TO_STRING __AlignText[] = 
{
	// DEFINE_IDT(AT_LEFT),		// left and top is 0.
	DEFINE_IDT(AT_CENTER),
	DEFINE_IDT(AT_RIGHT),
	// DEFINE_IDT(AT_TOP),
	DEFINE_IDT(AT_VCENTER),
	DEFINE_IDT(AT_BOTTOM),
	{ 0, NULL },
};

DWORD AlignTextFromStr(LPCTSTR szAlignText)
{
	return GetCombinationValue(__AlignText, szAlignText);
}

void AlignTextToStr(DWORD dwAlignTextFlags, tstring &str)
{
	GetCombinationStrValue(__AlignText, dwAlignTextFlags, str);
}

void CUIObjProperty::ToUIObjProperties(vector<tstring> &vProperties)
{
	vProperties.push_back(strName);
	vProperties.push_back(strValue);
	if (valueType == VT_IMAGE)
	{
		vProperties.insert(vProperties.end(), vOptions.begin(), vOptions.end());
	}
}

void CUIObjProperties::AddPropStr(LPCTSTR szName, LPCTSTR szValue, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));

	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = szValue;
	property.valueType = CUIObjProperty::VT_STR;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}


void CUIObjProperties::AddPropID(LPCTSTR szName, LPCTSTR szValue, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = szValue;
	property.valueType = CUIObjProperty::VT_ID;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropInt(LPCTSTR szName, int nValue, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	_itostr(nValue, property.strValue, 10);
	property.valueType = CUIObjProperty::VT_INT;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropVar(LPCTSTR szName, CFormula &formular, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = formular.GetFormula();
	property.valueType = CUIObjProperty::VT_VAR_INT;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropBoolStr(LPCTSTR szName, BOOL bValue, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = BOOLTOSTR(bValue);
	property.valueType = CUIObjProperty::VT_BOOL_STR;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropImage(LPCTSTR szName, LPCTSTR szRectName, LPCTSTR szImage, CSFImage &image, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;
	TCHAR				szRect[256];

	_stprintf_s(szRect, CountOf(szRect), _T("%d,%d,%d,%d"), 
		image.m_x, image.m_y,
		image.m_cx, image.m_cy);

	property.vOptions.push_back(szRectName);
	property.vOptions.push_back(szRect);

	property.strName = szName;
	property.strValue = szImage;
	property.valueType = CUIObjProperty::VT_IMAGE;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropImageEx(LPCTSTR szName, LPCTSTR szRectName, LPCTSTR szImage, CSFImage &image, vector<tstring> &vExtraProperies, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;
	TCHAR				szRect[256];

	_stprintf_s(szRect, CountOf(szRect), _T("%d,%d,%d,%d"), 
		image.m_x, image.m_y,
		image.m_cx, image.m_cy);

	property.vOptions.push_back(szRectName);
	property.vOptions.push_back(szRect);
	property.vOptions.insert(property.vOptions.end(), vExtraProperies.begin(), vExtraProperies.end());

	property.strName = szName;
	property.strValue = szImage;
	property.valueType = CUIObjProperty::VT_IMAGE;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropImageFile(LPCTSTR szName, LPCTSTR szImageFile, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = szImageFile;
	property.valueType = CUIObjProperty::VT_IMG_FILE;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropFile(LPCTSTR szName, LPCTSTR szFile, LPCTSTR szExtFilter, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = szFile;
	property.vOptions.push_back(szExtFilter);
	property.valueType = CUIObjProperty::VT_FILE;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropColor(LPCTSTR szName, MLColor &clrValue, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	ColorToStr(clrValue, property.strValue);
	property.valueType = CUIObjProperty::VT_COLOR;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropFontName(LPCTSTR szName, LPCTSTR szFont, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = szFont;
	property.valueType = CUIObjProperty::VT_FONT_NAME;
	property.bToXmlAttrib = bToXmlAttrib;

	push_back(property);
}

void CUIObjProperties::AddPropBltMode(LPCTSTR szName, BLT_MODE bltMode, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = IDToString(__idszBltMode, bltMode, __idszBltMode[0].szId);
	property.valueType = CUIObjProperty::VT_COMB_STR;
	property.bToXmlAttrib = bToXmlAttrib;
	property.vOptions.clear();
	for (int i = 0; __idszBltMode[i].szId != NULL; i++)
		property.vOptions.push_back(__idszBltMode[i].szId);

	push_back(property);	
}

void CUIObjProperties::AddPropBlendPixMode(LPCTSTR szName, BlendPixMode bpm, bool bToXmlAttrib)
{
	ASSERT(!IsPropExist(szName));
	CUIObjProperty		property;

	property.strName = szName;
	property.strValue = IDToString(__idszBlendPixMode, bpm, __idszBlendPixMode[0].szId);
	property.valueType = CUIObjProperty::VT_COMB_STR;
	property.bToXmlAttrib = bToXmlAttrib;
	property.vOptions.clear();
	for (int i = 0; __idszBlendPixMode[i].szId != NULL; i++)
		property.vOptions.push_back(__idszBlendPixMode[i].szId);

	push_back(property);	
}

void CUIObjProperties::DelProp(LPCTSTR szName)
{
	iterator		it, itEnd;
	itEnd = end();
	for (it = begin(); it != itEnd; ++it)
	{
		CUIObjProperty		&property = *it;

		if (_tcsicmp(szName, property.strName.c_str()) == 0)
		{
			erase(it);
			return;
		}
	}
}

void CUIObjProperties::ToXMLCategoryAttrib(CXMLWriter &xmlStream)
{
	iterator		it, itEnd;
	itEnd = end();
	for (it = begin(); it != itEnd; ++it)
	{
		CUIObjProperty		&property = *it;

		if (!property.bToXmlAttrib)
			continue;

		xmlStream.WriteAttribute(property.strName.c_str(), property.strValue.c_str());

		if (property.valueType == CUIObjProperty::VT_IMAGE)
		{
			for (int i = 0; i < (int)property.vOptions.size(); i += 2)
				xmlStream.WriteAttribute(property.vOptions[i].c_str(), property.vOptions[i + 1].c_str());
		}
	}
}

void CUIObjProperties::ToUIObjProperties(vector<tstring> &vProperties)
{
	iterator	it, itEnd;
	itEnd = end();
	for (it = begin(); it != itEnd; ++it)
		(*it).ToUIObjProperties(vProperties);
}

bool CUIObjProperties::IsPropExist(LPCTSTR szName)
{
	iterator	it, itEnd;
	itEnd = end();
	for (it = begin(); it != itEnd; ++it)
	{
		CUIObjProperty	&prop = *it;
		if (_tcsicmp(szName, prop.strName.c_str()) == 0)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


LPCTSTR CUIObject::ms_szClassName = _T("UIObject");

CUIObject::CUIObject()
{
	m_pSkin = NULL;
	m_pContainer = NULL;

//	m_nType = 0;
	m_nId = SKIN_UIOBJ_ID_UNDEFINE;
	m_nAutoUniID = 0;
	
	m_nMsgWant = 0;

	m_rcObj.SetRect(0, 0, 50, 30);

//	m_strText[0] = '\0';
	m_bTempTooltip = false;

	m_bEnable = true;
	m_bVisible = true;
	m_bHideIfWndInactive = false;
	m_bHideIfMouseInactive = false;
	m_bUseSkinBg = false;

	m_bTranslucencyWithSkin = false;

	m_bCreated = false;
}

CUIObject::~CUIObject()
{
	if (m_pSkin)
	{
		if (m_pSkin->GetCaptureMouse() == this)
		{
			m_pSkin->ReleaseCaptureMouse(this);
		}

		if (m_strTooltip.size() > 0 && m_nId != SKIN_UIOBJ_ID_UNDEFINE)
			m_pSkin->DelTool(m_nAutoUniID);

		m_pSkin->UnregisterTimerObject(this);
	}
}

void CUIObject::Draw(CRawGraph *pGraph)
{
//	pGraph->Rectangle(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height());
}

void CUIObject::UpdateTranslucencyWithSkin(CRawGraph *pGraph)
{
	if (!IsUpdateTranslucencyWithSkin())
		return;

	MLColor		clrAlpha;
	clrAlpha.setAlpha((byte)m_pSkin->m_nTranslucencyAlphaDynamic);
	if (m_imgMask.IsValid())
	{
		CRect	rcMask;
		rcMask.SetRect(m_imgMask.m_x, m_imgMask.m_y, m_imgMask.m_x + m_imgMask.m_cx, m_imgMask.m_y + m_imgMask.m_cy);
		pGraph->FillRect(&m_rcObj, m_imgMask.GetHandle(), &rcMask, clrAlpha, BPM_MULTIPLY);
	}
	else
		pGraph->FillRect(&m_rcObj, clrAlpha, BPM_MULTIPLY);
}

BOOL CUIObject::ReCalculatePos(FORMULA_VAR vars[])
{
	int	nValue;

	if (m_formLeft.CalCualteValue(vars, nValue))
		m_rcObj.left = nValue;

	if (m_formTop.CalCualteValue(vars, nValue))
		m_rcObj.top = nValue;

	if (m_pContainer)
	{
		m_rcObj.left += m_pContainer->m_rcObj.left;
		m_rcObj.top += m_pContainer->m_rcObj.top;
	}

	if (m_formWidth.CalCualteValue(vars, nValue))
		m_rcObj.right = m_rcObj.left + nValue;

	if (m_formHeight.CalCualteValue(vars, nValue))
		m_rcObj.bottom = m_rcObj.top + nValue;

	OnSize(m_rcObj.Width(), m_rcObj.Height());

	if (!m_strTooltip.empty()/* && m_nId != SKIN_UIOBJ_ID_UNDEFINE*/)
	{
		m_pSkin->DelTool(m_nAutoUniID);
		m_pSkin->AddTool(TOLOCAL(m_strTooltip.c_str()), &m_rcObj, m_nAutoUniID);
	}

	return TRUE;
}

void CUIObject::SetProperties(vector<tstring> &vProperties)
{
	for (int i = 0; i < (int)vProperties.size(); i += 2)
		SetProperty(vProperties[i].c_str(), vProperties[i + 1].c_str());
}

BOOL CUIObject::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	assert(m_pSkin);

	if (_tcsicmp(szProperty, SZ_PN_LEFT) == 0)
		m_formLeft.SetFormula(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_TOP) == 0)
		m_formTop.SetFormula(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_WIDTH) == 0)
		m_formWidth.SetFormula(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_HEIGHT) == 0)
		m_formHeight.SetFormula(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_ID) == 0)
	{
		const STRING_ID_MAP		*pId;
		m_strID = szValue;
		pId = m_pSkin->GetSkinFactory()->GetIDByNameEx(szValue);
		if (pId)
		{
			m_nId = pId->nId;
			if (m_strTooltip.empty() && pId->szToolTip)
			{
				m_strTooltip = pId->szToolTip;
				m_bTempTooltip = TRUE;
			}
		}
		else
			m_nId = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
		m_strName = szValue;
	else if (_tcsicmp(szProperty, _T("Visible")) == 0)
		m_bVisible = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("HideIfWndInactive")) == 0)
		m_bHideIfWndInactive = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("HideIfMouseInactive")) == 0)
		m_bHideIfMouseInactive = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("UseSkinBg")) == 0)
		m_bUseSkinBg = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("TranslucencyWithSkin")) == 0)
		m_bTranslucencyWithSkin = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_TOOLTIP) == 0)
	{
		if (_tcscmp(m_strTooltip.c_str(), szValue) != 0)
		{
			m_strTooltip = szValue;
			if (m_bTempTooltip)
				m_bTempTooltip = FALSE;
		}
	}
	else if (IsPropertyName(szProperty, _T("ImageMask")))
	{
		m_strMaskImage = szValue;
		m_imgMask.LoadFromSRM(m_pSkin->GetSkinFactory(), m_strMaskImage.c_str());
	}
	else if (IsPropertyName(szProperty, _T("ImageMaskRect")))
	{
		if (!GetRectValue(szValue, m_imgMask))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else
		return FALSE;

	return TRUE;
}

BOOL CUIObject::SetPropertyInt(LPCTSTR szProperty, int nValue)
{
	TCHAR		szBuffer[64];

	_itot_s(nValue, szBuffer, CountOf(szBuffer), 10);
	return SetProperty(szProperty, szBuffer);
}

void CUIObject::OnCreate()
{
}
/*
LPCTSTR CUIObject::GetClassName()
{

}*/

void CUIObject::Invalidate()
{
	assert(m_pSkin);

	if (!IsVisible() || !IsParentVisible())
		return;

	if (m_pContainer)
		m_pContainer->InvalidateUIObject(this);
	else
		m_pSkin->InvalidateUIObject(this);
}

// COMMENT:
//		TIMER ??
void CUIObject::OnTimer(int nId)
{

}

void CUIObject::OnSize(int cx, int cy)
{

}


bool CUIObject::IsVisible()
{
	return m_bVisible && (!m_bHideIfWndInactive || m_pSkin->IsWndActive())
		&& (!m_bHideIfMouseInactive || !m_pSkin->IsMouseInactive());
}


void CUIObject::SetVisible(bool bVisible)
{
	if (m_bVisible != bVisible)
	{
		m_bVisible = bVisible;
		if (bVisible)
			m_pSkin->AddTool(TOLOCAL(m_strTooltip.c_str()), &m_rcObj, m_nAutoUniID);
		else
			m_pSkin->DelTool(m_nAutoUniID);
	}

	CUIObject	*pObjFocus;

	pObjFocus = m_pSkin->GetFocusUIObj();
	if (bVisible)
	{
		if (pObjFocus == NULL)
		{
			// try to set focus to one UIObject
			m_pSkin->FocusNextUIObject();
		}
	}
	else
	{
		// Will focus UIObject be hide by this call?
		if (!pObjFocus || !pObjFocus->IsVisible() || !pObjFocus->IsParentVisible())
		{
			// Change focus UIObject.
			m_pSkin->FocusNextUIObject();
		}
	}
}


bool CUIObject::IsParentVisible()
{
	CSkinContainer	*pContainer = m_pContainer;

	while (pContainer)
	{
		if (!pContainer->IsVisible())
			return false;

		pContainer = pContainer->GetParent();
	}

	return true;
}


void CUIObject::SetFocus()
{
	m_pSkin->SetFocusUIObject(this);
}

bool CUIObject::IsOnFocus()
{
	return m_pSkin->GetFocusUIObj() == this;
}

bool CUIObject::IsPtIn(POINT pt)
{
	if (!m_rcObj.PtInRect(pt))
		return false;

	if (m_imgMask.IsValid())
	{
		PRAW_IMAGE_DATA		rawImg = m_imgMask.GetHandle();
		RGBQUAD	quad = rawImg->GetPixel(pt.x, pt.y);
		if (quad.rgbReserved == 0)	// Alpha channel == 0
			return false;
	}

	return true;
}

void CUIObject::EnumProperties(CUIObjProperties &listProperties)
{
	listProperties.AddPropStr(SZ_PN_NAME, m_strName.c_str(), !m_strName.empty());

	listProperties.AddPropID(SZ_PN_ID, m_strID.c_str(), !m_strID.empty() || m_nId != ID_UNDEFINE);

	listProperties.AddPropVar(SZ_PN_LEFT, m_formLeft);
	listProperties.AddPropVar(SZ_PN_TOP, m_formTop);
	listProperties.AddPropVar(SZ_PN_WIDTH, m_formWidth);
	listProperties.AddPropVar(SZ_PN_HEIGHT, m_formHeight);

	listProperties.AddPropBoolStr(_T("Visible"), m_bVisible, !m_bVisible);
	listProperties.AddPropBoolStr(_T("HideIfWndInactive"), m_bHideIfWndInactive, m_bHideIfWndInactive);
	listProperties.AddPropBoolStr(_T("UseSkinBg"), m_bUseSkinBg, m_bUseSkinBg);
	listProperties.AddPropBoolStr(_T("TranslucencyWithSkin"), m_bTranslucencyWithSkin, m_bTranslucencyWithSkin);

	listProperties.AddPropStr(SZ_PN_TOOLTIP, m_strTooltip.c_str(), !m_strTooltip.empty() && !m_bTempTooltip);

	listProperties.AddPropImage(_T("ImageMask"), _T("ImageMaskRect"), m_strMaskImage.c_str(), m_imgMask, !m_strMaskImage.empty());
}

int CUIObject::FromXML(CSkinContainer *pContainer, SXNode *pXmlNode)
{
// 	if (pXmlNode->GetName())
// 	{
// 		if (!IsKindOf(pXmlNode->GetName()))
// 		{
// 			ASSERT(0);
// 			return ERR_FAILED;
// 		}
// 	}

	m_pContainer = pContainer;

	SXNode::iterProperties	it, itEnd;

	itEnd = pXmlNode->listProperties.end();
	for (it = pXmlNode->listProperties.begin(); it != itEnd; ++it)
	{
		SXNode::Property	&prop = *it;
		if (!SetProperty(prop.strName.c_str(), prop.strValue.c_str()))
		{
		}
	}

	return ERR_OK;
}

void CUIObject::ToXML(CXMLWriter &xmlStream)
{
	CUIObjProperties	properites;

	EnumProperties(properites);

	xmlStream.WriteStartElement(GetClassName());

	properites.ToXMLCategoryAttrib(xmlStream);

	// ?????
	if (HasXMLChild())
	{
		OnToXMLChild(xmlStream);
	}

	xmlStream.WriteEndElement();
}

UINT CUIObject::GetProfileInt(LPCTSTR szKeyName, int nDefault)
{
	tstring		strKeyName;

	strKeyName = m_pSkin->GetSkinWndName();
	strKeyName += m_strName;
	strKeyName += szKeyName;

	return g_Profile.GetProfileInt(m_pSkin->GetSkinFactory()->GetSkinName(),
		strKeyName.c_str(), 
		nDefault);
}

// UINT CUIObject::GetProfileString(LPCTSTR szKeyName, tstring &strValue, LPCTSTR szDefault)
// {
// 	tstring		strKeyName;
// 
// 	strKeyName = m_pSkin->GetSkinWndName();
// 	strKeyName += m_strName;
// 	strKeyName += szKeyName;
// 
// 	return g_Profile.GetProfileString(m_pSkin->GetSkinFactory()->GetSkinName(),
// 		strKeyName.c_str(), 
// 		strValue);
// }

void CUIObject::WriteProfileInt(LPCTSTR szKeyName, int nValue)
{
	tstring		strKeyName;

	strKeyName = m_pSkin->GetSkinWndName();
	strKeyName += m_strName;
	strKeyName += szKeyName;

	g_Profile.WriteProfileInt(m_pSkin->GetSkinFactory()->GetSkinName(),
		strKeyName.c_str(), 
		nValue);
}
