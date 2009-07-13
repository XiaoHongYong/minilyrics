// DlgChooseFont.cpp: implementation of the CDlgChooseFont class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../MPlayerApp.h"
#include "DlgChooseFont.h"

// #define FLAG_BOLD		1
// #define FLAG_ITALIC		(0x1 << 1)
// 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgChooseFont::CDlgChooseFont()
{
	m_window = NULL;
}

CDlgChooseFont::~CDlgChooseFont()
{
	if (m_window)
		gtk_widget_destroy(m_window);
}

int CDlgChooseFont::DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontHeight, int nWeight, int nItalic)
{
	int		nRet;

	m_strFontFaceName = szFontFaceName;
	m_nFontHeight = nFontHeight;
	m_nWeight = nWeight;
	m_nItalic = nItalic;

	m_window = gtk_font_selection_dialog_new("Select Font");

	{
		// set font name
		tstring		strFontName;

		// bold
		strFontName += szFontFaceName;
		if (nWeight >= FW_BOLD)
			strFontName += " Bold";
		else if (nWeight >= FW_SEMIBOLD)
			strFontName += " Semi-Bold";

		// italic
		if (nItalic == FS_ITALIC)
			strFontName += " Italic";
		else if (nItalic == FS_OBLIQUE)
			strFontName += " Oblique";

		// font size
		strFontName += " ";
		strFontName += CStrPrintf(_T("%d"), nFontHeight).c_str();

		gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(m_window), strFontName.c_str());
	}

	nRet = gtk_dialog_run(GTK_DIALOG(m_window));

	if (nRet == IDOK)
	{
		bool	bUnderLine;
		gchar	*fontName;
		gchar	*szPos, *szEnd;
		V_Str	vStr;
//     char *deco[] = {
//         "Semi-Bold", "Bold", 
//         "Italic", 
//         "Semi-Expanded", "Expanded", 
//         "Extra-Condensed", "Semi-Condensed", "Condensed", 
//         "Ultra-Light", "Light", 
//         "Oblique", NULL };

		fontName = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(m_window));
		StrBreak(fontName, ' ', vStr);

		// font size.
		m_nFontHeight = 12;
		if (vStr.size() > 0)
		{
			m_nFontHeight = _ttoi(vStr.back().c_str());
			vStr.pop_back();
		}

		// Italic?
		// Oblique
		m_nItalic = FS_NORMAL;
		if (vStr.size() > 0)
		{
			if (_tcsicmp(vStr.back().c_str(), _T("Italic")) == 0)
			{
				m_nItalic = FS_ITALIC;
				vStr.pop_back();
			}
			else if (_tcsicmp(vStr.back().c_str(), _T("Oblique")) == 0)
			{
				m_nItalic = FS_OBLIQUE;
				vStr.pop_back();
			}
		}

		// Semi-Bold
		// Bold?
		m_nWeight = FW_NORMAL;
		if (vStr.size() > 0)
		{
			if (_tcsicmp(vStr.back().c_str(), _T("Bold")) == 0)
			{
				m_nWeight = FW_BOLD;
				vStr.pop_back();
			}
			else if (_tcsicmp(vStr.back().c_str(), _T("Semi-Bold")) == 0)
			{
				m_nWeight = FW_SEMIBOLD;
				vStr.pop_back();
			}
		}

		// "Extra-Condensed", "Semi-Condensed", "Condensed", 
		if (vStr.size() > 0)
		{
			if (_tcsicmp(vStr.back().c_str(), _T("Extra-Condensed")) == 0)
			{
				vStr.pop_back();
			}
			else if (_tcsicmp(vStr.back().c_str(), _T("Semi-Condensed")) == 0)
			{
				vStr.pop_back();
			}
			else if (_tcsicmp(vStr.back().c_str(), _T("Condensed")) == 0)
			{
				vStr.pop_back();
			}
		}

		// "Ultra-Light", "Light", 
		if (vStr.size() > 0)
		{
			if (_tcsicmp(vStr.back().c_str(), _T("Ultra-Light")) == 0)
			{
				vStr.pop_back();
			}
			else if (_tcsicmp(vStr.back().c_str(), _T("Light")) == 0)
			{
				vStr.pop_back();
			}
		}

		// font name
		m_strFontFaceName.resize(0);
		for (int i = 0; i < vStr.size(); i++)
		{
			if (i != 0)
				m_strFontFaceName += " ";
			m_strFontFaceName += vStr[i];
		}
	}

	return nRet;
}

LPCTSTR CDlgChooseFont::GetFaceName()
{
	return m_strFontFaceName.c_str();
}

int CDlgChooseFont::GetHeight()
{
	return m_nFontHeight;
}

int CDlgChooseFont::GetWeight()
{
	return m_nWeight;
}

int CDlgChooseFont::GetItalic()
{
	return m_nItalic;
}
