// MPSkinVis.h: interface for the CMPSkinVis class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_)
#define AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MPlayerLib/IMPlayer.h"
#include "../gfx/win32/MLPen.h"

#ifdef SKIN_EDITOR
class IEventHandler
{
};
#endif

class CMPSkinVis : public CUIObject, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	enum VisMode
	{
		VM_OSILLOSCOPE,
		VM_SPECTRUM,
	};

	CMPSkinVis();
	virtual ~CMPSkinVis();

#ifndef SKIN_EDITOR
	virtual void OnCreate();

	virtual void OnEvent(const IEvent *pEvent);
#endif

	virtual bool OnLButtonUp( UINT nFlags, CPoint point );

	virtual void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
	virtual void DrawOsilloscope(CRawGraph *pGraph, VisParam *visParam);

	virtual void DrawSpectrum(CRawGraph *pGraph, VisParam *visParam);

protected:
	VisMode				m_visMode;
	MLColor				m_clrFg;
	CRawPen				m_pen;

	tstring				m_strBgFile, m_strSpectrumColFile;;
	CSFImage			m_imgBg, m_imgSpectrumCol;
	int					m_nSpectrumUnitHeight;
	int					m_nSpectrumColSpace;
	int					m_nLeftMargin, m_nRightMargin, m_nTopMargin, m_nBottomMargin;

	VisParam			*m_visParamCur;

	// vector<int>			m_vSpectrumDropDownMax, m_vSpectrumMax;
	struct SpetrumCol
	{
		short		nMax;
		short		nDropMax;
		float		nDropSpeed;
	};
	vector<SpetrumCol>	m_vSpectrumDrop;

};

#endif // !defined(AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_)
