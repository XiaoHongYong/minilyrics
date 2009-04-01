// LyricShowTextEditObj.h: interface for the CLyricShowTextEditObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSHOWTEXTEDITOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_)
#define AFX_LYRICSHOWTEXTEDITOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_

#pragma once


#include "LyricShowObj.h"
#include "SkinEditCtrl_t.h"
#include "LyrEditHandler.h"
#include "DlgFindReplace.h"

class CLyricShowTextEditObj : public CSkinEditBoard_t<CLyricShowObj, CLyrEditHandler>, public IEditNotification, public IFindReplaceNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
	CLyricShowTextEditObj();
	virtual ~CLyricShowTextEditObj();

public:
	typedef CSkinEditBoard_t<CLyricShowObj, CLyrEditHandler>		_BaseClass;

public:
	virtual CRawGraph *EB_GetGraphics()
	{
		CRawGraph		*pGraph = m_pContainer->GetMemGraph();
		m_font.EnableGdiplusAntialias(CMLGraphicsFactory::GetGraphicsType() == MGT_GDIPLUS);
		pGraph->SetFont(&m_font);

		return pGraph;
	}

	virtual int EB_GetFontHeight() { return m_font.GetHeight(); }

	virtual void EB_SetFont(CMLFont &font) { m_font.CreateFrom(font); }

	virtual MLColor & GetColor(int nColorName);

	virtual byte GetBgAlpha()
	{
		return byte(m_pSkin->m_nTranslucencyAlphaDynamic);
	}

	virtual void OnCreate();

	virtual void OnEvent(const IEvent *pEvent);

	virtual BOOL OnCommand(DWORD dwCmdID, LPARAM lParam);

	virtual bool OnRButtonUp(UINT nFlags, CPoint point);

	virtual void OnKeyDown(UINT nChar, UINT nFlags);

	virtual void OnContexMenu(int xPos, int yPos);

	virtual void OnSaveLyrics();

	virtual void Draw(CRawGraph *pGraph);

	virtual void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);

	void UpdateLyricsProperties();

public:
	void OnPlayTimeChangedUpdate();
	void OnLyricsChanged();

public:
	virtual void OnStatusChanged(IEditNotification::Status status, bool bVal);
	virtual void OnTextChanged();

	virtual void OnTimer(int nId);

protected:
	bool TranslateAccelerator(UINT nChar, UINT nFlags);

	void UpdateTextFontColor();

	virtual void ReloadThemeOfSkin(bool bSavetoProfile);

	void OnFRCommand(IFindReplaceNotify::Command cmd);
	void ShowFindDialog(bool bFind = true);
	void FindNext();

	bool				m_bDiscardLyrChangeEvent;
	int					m_nTimerIDUpdateLyrics;

	CDlgFindReplace			m_dlgFindReplace;

};

#endif // !defined(AFX_LYRICSHOWTEXTEDITOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_)
