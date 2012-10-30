// SkinVis.cpp: implementation of the CMPSkinVis class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifndef SKIN_EDITOR
#include "MPlayerAppBase.h"
#endif
#include "MPSkinVis.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinVis, _T("Vis"))

CMPSkinVis::CMPSkinVis()
{
	m_nMsgWant = UO_MSG_WANT_LBUTTON;
	m_clrFg.set(RGB(0, 255, 0));
	m_pen.CreateSolidPen(1, m_clrFg);

	m_nLeftMargin = m_nRightMargin = m_nTopMargin = m_nBottomMargin = 0;
	m_nSpectrumColSpace = 0;
	m_nSpectrumUnitHeight = 1;
	m_visParamCur = NULL;
	if (_tcsicmp(g_Profile.GetString(_T("VisMode"), _T("")),  _T("Osilloscope")) == 0)
		m_visMode = VM_OSILLOSCOPE;
	else
		m_visMode = VM_SPECTRUM;
}

CMPSkinVis::~CMPSkinVis()
{
#ifndef SKIN_EDITOR
	CMPlayerAppBase::GetEventsDispatcher()->UnRegisterHandler(ET_VIS_DRAW_UPDATE, this);
#endif
}

#ifndef SKIN_EDITOR
void CMPSkinVis::OnCreate()
{
	CUIObject::OnCreate();

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHandler(ET_VIS_DRAW_UPDATE, this);
}

void CMPSkinVis::OnEvent(const IEvent *pEvent)
{
	CEventVisDrawUpdate		*pVisEvt = (CEventVisDrawUpdate*)pEvent;

	//// the pVisEvt->pVisParam should be a static param.
	m_visParamCur = pVisEvt->pVisParam;

	Invalidate();
}
#endif

bool CMPSkinVis::OnLButtonUp(UINT nFlags, CPoint point)
{

	if (m_visMode == VM_OSILLOSCOPE)
	{
		m_visMode = VM_SPECTRUM;
		g_Profile.WriteString(_T("VisMode"), _T("Spectrum"));
	}
	else
	{
		m_visMode = VM_OSILLOSCOPE;
		g_Profile.WriteString(_T("VisMode"), _T("Osilloscope"));
	}


	return true;
}

void CMPSkinVis::Draw(CRawGraph *pGraph)
{
	if (m_visParamCur)
	{
		if (m_visMode == VM_SPECTRUM)
			DrawSpectrum(pGraph, m_visParamCur);
		else
			DrawOsilloscope(pGraph, m_visParamCur);
	}
	else
		m_imgBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_imgBg.m_x, m_imgBg.m_y);
}

BOOL CMPSkinVis::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strBgFile = szValue;
		m_imgBg.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_IMAGERECT) == 0)
	{
		if (!GetRectValue(szValue, m_imgBg))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	if (_tcsicmp(szProperty, _T("SpectrumColImage")) == 0)
	{
		m_strSpectrumColFile = szValue;
		m_imgSpectrumCol.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, _T("SpectrumColRect")) == 0)
	{
		if (!GetRectValue(szValue, m_imgSpectrumCol))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("SpectrumColSpace")) == 0)
	{
		m_nSpectrumColSpace = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("SpectrumUnitHeight")) == 0)
	{
		m_nSpectrumUnitHeight = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("LeftMargin")) == 0)
	{
		m_nLeftMargin = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("RightMargin")) == 0)
	{
		m_nRightMargin = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("TopMargin")) == 0)
	{
		m_nTopMargin = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("BottomMargin")) == 0)
	{
		m_nBottomMargin = _ttoi(szValue);
	}
	else if (_tcsicmp(szProperty, _T("VisMode")) == 0)
	{
		if (_tcsicmp(szValue,  _T("Osilloscope")) == 0)
			m_visMode = VM_OSILLOSCOPE;
		else
			m_visMode = VM_SPECTRUM;
	}
	else if (_tcsicmp(szProperty, _T("FgColor")) == 0)
	{
		GetColorValue(m_clrFg, szValue);
		m_pen.CreateSolidPen(1, m_clrFg);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CMPSkinVis::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBgFile.c_str(), m_imgBg);
	listProperties.AddPropImage(_T("SpectrumColImage"), _T("SpectrumColRect"), m_strSpectrumColFile.c_str(), m_imgSpectrumCol);

	listProperties.AddPropInt(_T("SpectrumColSpace"), m_nSpectrumColSpace);

	listProperties.AddPropInt(_T("SpectrumUnitHeight"), m_nSpectrumUnitHeight);

	listProperties.AddPropInt(_T("LeftMargin"), m_nLeftMargin);
	listProperties.AddPropInt(_T("RightMargin"), m_nRightMargin);
	listProperties.AddPropInt(_T("TopMargin"), m_nTopMargin);
	listProperties.AddPropInt(_T("BottomMargin"), m_nBottomMargin);

	listProperties.AddPropColor(_T("FgColor"), m_clrFg);


	CUIObjProperty		prop;

	prop.strName = _T("VisMode");
	if (m_visMode == VM_OSILLOSCOPE)
		prop.strValue = _T("Osilloscope");
	else
		prop.strValue = _T("Spectrum");
	prop.valueType = CUIObjProperty::VT_COMB_STR;
	prop.vOptions.push_back(_T("Osilloscope"));
	prop.vOptions.push_back(_T("Spectrum"));
	listProperties.push_back(prop);
}
#endif // _SKIN_EDITOR_

void CMPSkinVis::DrawOsilloscope(CRawGraph *pGraph, VisParam *visParam)
{
	m_imgBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_imgBg.m_x, m_imgBg.m_y);
	int		nWidthMax = m_rcObj.Width();
	int		nHeight = m_rcObj.Height() / 2;
	int		x, y, i;

	if (nWidthMax > VIS_N_WAVE_SAMPLE)
		nWidthMax = VIS_N_WAVE_SAMPLE;

	pGraph->SetPen(m_pen);

	y = m_rcObj.top + nHeight;
	i = 0;
	//for (i = 0; i < visParam->nChannels; i++)
	{
		int		yOffsetPrev = 0, yOffsetNew;
		yOffsetPrev = ((char)(visParam->waveformData[i][0] )) * nHeight/ 128;
		for (x = 1; x < nWidthMax; x ++)
		{
			yOffsetNew = ((char)(visParam->waveformData[i][x] )) * nHeight/ 128;
			pGraph->Line(x + m_rcObj.left - 1, y + yOffsetPrev, x + m_rcObj.left, y + yOffsetNew);
			yOffsetPrev = yOffsetNew;
			// LineTo(memDC,x,(y*256 + visParam->waveformData[y][x]^128)>>(visParam->nChannels-1));
		}
	}
}

void CMPSkinVis::DrawSpectrum(CRawGraph *pGraph, VisParam *visParam)
{
	m_imgBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_imgBg.m_x, m_imgBg.m_y);

	int		nHeight = m_rcObj.Height() - 2;
	int		nColWidth = m_imgSpectrumCol.width() + m_nSpectrumColSpace;
	int		nColCount = m_imgBg.width() / nColWidth;
	float	fCol = ((float)VIS_N_WAVE_SAMPLE) / 2 / nColCount;
	float	fStart, fEnd;
	int		x;

	if (m_vSpectrumDrop.size() != nColCount)
	{
		SpetrumCol	col;
		col.nDropMax = 0;
		col.nDropSpeed = (float)0.0001;
		col.nMax = 0;
		m_vSpectrumDrop.resize(nColCount, col);
	}

	fStart = 0;
	x = m_rcObj.left + m_nLeftMargin;
	for (int k = 0; k < nColCount; k++)
	{
		int		nMax = 0;
		fEnd = fStart + fCol;
		for (int i = (int)fStart ; i < (int)fEnd; i++)
		{
			if (visParam->spectrumData[0][i] > nMax)
				nMax = visParam->spectrumData[0][i];
		}

		SpetrumCol	&col = m_vSpectrumDrop[k];

		if (col.nDropMax < nMax)
		{
			col.nDropMax = nMax;
			col.nDropSpeed = (float)0.0001;
		}
		if (col.nMax < nMax)
			col.nMax = nMax;
		else
			nMax = col.nMax;

		// Draw spectrum column
		nMax = int((float)nMax * nHeight / 256 + 0.5) + 1;
		nMax -= nMax % m_nSpectrumColSpace;
		m_imgSpectrumCol.Blt(pGraph, x, m_rcObj.bottom - m_nBottomMargin - nMax, m_imgSpectrumCol.width(), nMax, m_imgSpectrumCol.m_x, m_imgSpectrumCol.m_y);

		// Draw spectrum column float...
		nMax = col.nDropMax;
		nMax = int((float)nMax * nHeight / 256 + 0.5) + 1;
		m_imgSpectrumCol.Blt(pGraph, x, m_rcObj.bottom - m_nBottomMargin - nMax, m_imgSpectrumCol.width(), m_nSpectrumUnitHeight, m_imgSpectrumCol.m_x, m_imgSpectrumCol.m_y);

		fStart = fEnd;
		x += nColWidth;

		col.nDropMax -= (short)col.nDropSpeed;
		col.nDropSpeed += (col.nDropSpeed / 5);
		if (col.nDropMax < 0)
			col.nDropMax = 0;
		col.nMax -= 8;
		if (col.nMax < 0)
			col.nMax = 0;
	}

/*	pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), &m_brBg);
	int		nWidthMax = m_rcObj.Width();
	int		nHeight = m_rcObj.Height() - 2;
	int		x, y, i;

	if (nWidthMax > VIS_N_WAVE_SAMPLE / 2)
		nWidthMax = VIS_N_WAVE_SAMPLE / 2;

	HPEN	hPenOld = (HPEN)::SelectObject(pGraph->GetHandle(), m_pen.GetHandle());

	y = m_rcObj.bottom - 2;
	x = 0;
	i = 0;
	{
		for (x = 0; x < nWidthMax; x ++)
		{
			pGraph->Line(x + m_rcObj.left, y + 1, x + m_rcObj.left, y - visParam->spectrumData[i][x] * nHeight / 256);
		}
	}

	::SelectObject(pGraph->GetHandle(), hPenOld);*/
}
