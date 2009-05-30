// SkinPicText.cpp: implementation of the CSkinPicText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinPicText.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SZ_PIC_TEXT		_T(" 0123456789:-ABCDEFGHIJKLMNOPQRSTUVWXYZ");

UIOBJECT_CLASS_NAME_IMP(CSkinPicText, _T("PicText"));

extern ID_TO_STRING __AlignText[];

CSkinPicText::CSkinPicText()
{
	m_strImgChars = SZ_PIC_TEXT;
	m_nSpaceCharPos = 0;
	m_dwAlignTextFlags = 0;
	m_nCharWidth = 0;
}

CSkinPicText::~CSkinPicText()
{

}

void CSkinPicText::Draw(CRawGraph *pGraph)
{
	// 0123456789:- 
	
	if (m_vPos.size() != m_strText.size())
	{
		LPCTSTR		szPicText = m_strImgChars.c_str();
		LPCTSTR		szPos;
		m_vPos.clear();
		for (int i = 0; i < (int)m_strText.size(); i++)
		{
			TCHAR		ch = m_strText[i];
			szPos = _tcschr(szPicText, ch);
			if (szPos)
				m_vPos.push_back(int(szPos - szPicText) * m_nCharWidth);
			else
				m_vPos.push_back(m_nSpaceCharPos * m_nCharWidth);
		}
	}

	int		x;

	if (m_dwAlignTextFlags == AT_CENTER)
	{
		x = m_rcObj.left + (m_rcObj.Width() - (int)m_strText.size() * m_nCharWidth) / 2;
		if (x <= m_rcObj.left)
			x = m_rcObj.left;
		else
			m_imgText.ExtendXBlt(pGraph, m_rcObj.left, m_rcObj.top, x - m_rcObj.left, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y, m_nCharWidth, m_imgText.height());
	}
	else
		x = m_rcObj.left;
	for (int i = 0; i < (int)m_strText.size(); i++)
	{
		m_imgText.Blt(pGraph, 
			x, m_rcObj.top, m_nCharWidth, m_imgText.height(), m_vPos[i], m_imgText.m_y);
		x += m_nCharWidth;
		if (x > m_rcObj.right)
			break;
	}

	if (x < m_rcObj.right)
		m_imgText.ExtendXBlt(pGraph, x, m_rcObj.top, m_rcObj.right - x, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y, m_nCharWidth, m_imgText.height());

/*	while (x < m_rcObj.right)
	{
		pGraph->DrawImage(&m_imgText, x, m_rcObj.top, m_nCharWidth, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y);
		x += m_nCharWidth;
	}*/
}

BOOL CSkinPicText::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("Text")) == 0)
	{
		m_strText = szValue;
		m_vPos.clear();
	}
	else if (_tcsicmp(szProperty, _T("AlignText")) == 0)
	{
		m_dwAlignTextFlags = AlignTextFromStr(szValue);
	}
	else if (_tcsicmp(szProperty, _T("CharWidth")) == 0)
	{
		m_nCharWidth = _ttoi(szValue);
		m_vPos.clear();
	}
	else if (_tcsicmp(szProperty, _T("TextImage")) == 0)
	{
		m_strImgText = szValue;
		m_imgText.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, _T("ImageChars")) == 0)
	{
		m_strImgChars = szValue;
		LPCTSTR		szPos = _tcschr(m_strImgChars.c_str(), ' ');
		if (szPos)
			m_nSpaceCharPos = int(szPos - m_strImgChars.c_str());
		else
			m_nSpaceCharPos = 0;
		m_vPos.clear();
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinPicText::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	tstring		str;
	AlignTextToStr(m_dwAlignTextFlags, str);
	listProperties.AddPropStr(_T("AlignText"), str.c_str());
	listProperties.AddPropInt(_T("CharWidth"), m_nCharWidth);
	listProperties.AddPropImageFile(_T("TextImage"), m_strImgText.c_str());
	listProperties.AddPropStr(_T("ImageChars"), m_strImgChars.c_str());
}

void CSkinPicText::SetText(LPCTSTR szText)
{
	m_strText = szText;
	m_vPos.clear();
}
