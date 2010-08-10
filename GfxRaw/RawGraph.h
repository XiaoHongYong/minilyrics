// RawGraph.h: interface for the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAWGRAPH_H__DB0AA2D5_3D46_4B35_9FF4_F93C76ED577B__INCLUDED_)
#define AFX_RAWGRAPH_H__DB0AA2D5_3D46_4B35_9FF4_F93C76ED577B__INCLUDED_

#pragma once


#include "../ImageLib/RawImageData.h"


namespace GfxRaw
{

enum
{
	PIX_B				= 0,
	PIX_G				= 1,
	PIX_R				= 2,
	PIX_A				= 3,		// Alpha channel
	PIX_SIZE			= 4
};

};

#ifdef _WIN32
#include "win32/RawGraphData.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/RawGraphData.h"
#endif

#ifdef _MAC
#include "mac/RawGraphData.h"
#endif


class CRawBmpFont;

//
// CRawGraph is a 32 bit depth Graphics.
//
// Its graph buffer can be accessed directly.
//
class CRawGraph : public CRawGraphData
{
public:
	CRawGraph();
	virtual ~CRawGraph();

	class CClipBoxAutoRecovery {
	public:
		CClipBoxAutoRecovery(CRawGraph *pGraph) {
			m_pGraph = pGraph;
			pGraph->GetClipBoundBox(m_rcClipOrg);
		}
		~CClipBoxAutoRecovery() {
			if (m_pGraph)
				m_pGraph->ResetClipBoundBox(m_rcClipOrg);
		}

		void Recover() {
			if (m_pGraph) {
				m_pGraph->ResetClipBoundBox(m_rcClipOrg);
				m_pGraph = NULL;
			}
		}

	protected:
		CRawGraph	*m_pGraph;
		CRect		m_rcClipOrg;

	};

	class COpacityBlendAutoRecovery {
	public:
		COpacityBlendAutoRecovery(CRawGraph *pGraph, int nOpacityPainting) {
			if (nOpacityPainting != 255)
			{
				m_pGraph = pGraph;
				m_nOpacityPainting = pGraph->GetOpacityPainting();

				nOpacityPainting = m_nOpacityPainting * nOpacityPainting / 255;
				m_pGraph->SetOpacityPainting(nOpacityPainting);
			}
			else
				m_pGraph = NULL;
		}
		~COpacityBlendAutoRecovery() {
			if (m_pGraph)
				m_pGraph->SetOpacityPainting(m_nOpacityPainting);
		}

		void Recover() {
			if (m_pGraph) {
				m_pGraph->SetOpacityPainting(m_nOpacityPainting);
				m_pGraph = NULL;
			}
		}

	protected:
		CRawGraph	*m_pGraph;
		int			m_nOpacityPainting;

	};

	virtual bool Create(int cx, int cy, IMLGraphicsBase *pGraph, int nBitCount = 32);

	virtual void Line(int x1, int y1, int x2, int y2);

	virtual void Rectangle(int x, int y, int width, int height);

	virtual void SetPen(CRawPen &pen);

	virtual void GetClipBoundBox(CRect &rc);
	virtual void SetClipBoundBox(const CRect &rc);
	virtual void ResetClipBoundBox(const CRect &rc);
	virtual void ClearClipBoundBox();

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect, int xLeftClipOffset = 0);

	virtual bool TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder);
	virtual bool DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, int xLeftClipOffset = 0);
	virtual bool DrawTextOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat = DT_SINGLELINE | DT_LEFT);

	virtual bool DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat = DT_SINGLELINE | DT_LEFT);

	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);


	virtual void SetTextColor(const MLColor &color)
	{
		m_clrText = color;
	}

public:
	void SetFont(CRawBmpFont *font);

	void ResetAlphaChannel(BYTE byAlpha, const RECT *lpRect);
	void MultiplyAlpha(BYTE byAlpha, const RECT *lpRect);

	void VertAlphaFadeOut(const RECT *lpRect, bool bTop);
	void VertFadeOut(const RECT *lpRect, const MLColor &clrBg, bool bTop);

	void HorzAlphaFadeOut(const RECT *lpRect, bool bLeft);
	void HorzFadeOut(const RECT *lpRect, const MLColor &clrBg, bool bLeft);

	void FillRectXOR(const RECT *lpRect, const MLColor &clrFill);

	void FillRect(const RECT *lpRect, PRAW_IMAGE_DATA pImgMask, const RECT *rcMask, const MLColor &clrFill, BlendPixMode bpm = BPM_COPY);
	void FillRect(const RECT *lpRect, const MLColor &clrFill, BlendPixMode bpm = BPM_COPY);

	void FillRect(int x, int y, int nWidth, int nHeight, const MLColor &clrFill, BlendPixMode bpm = BPM_COPY)
	{
		CRect	rc(x, y, x + nWidth, y + nHeight);

		FillRect(&rc, clrFill, bpm);
	}

	int GetOpacityPainting() const { return m_nOpacityPainting; }
	int SetOpacityPainting(int nOpacityPainting) { int nOrg = m_nOpacityPainting; m_nOpacityPainting = nOpacityPainting; return nOrg; }

	bool SetEnableAlphaChannel(bool bEnable) { bool bOrg = m_bAlphaChannelEnabled; m_bAlphaChannelEnabled = (bEnable && m_imageData.bitCount == 32); return bOrg; }
	bool GetEnableAlphaChannel() const { return m_bAlphaChannelEnabled; }

	//
	// The following APIs provide logical coordinate and real coordinate convert.
	//

	POINT SetOrigin(const POINT &ptOrg);
	void GetOrigin(POINT &ptOrg) const { ptOrg = m_ptOrigin; }

	void GetMappedClipBoundRect(RECT &rc) const { rc = m_rcClip; rc.left += m_ptOrigin.x; rc.top += m_ptOrigin.y; rc.right += m_ptOrigin.x; rc.bottom += m_ptOrigin.y; }

	void MapPoint(POINT &pt) const { pt.x += m_ptOrigin.x; pt.y += m_ptOrigin.y; }
	void MapRect(RECT &rc) const { rc.left += m_ptOrigin.x; rc.top += m_ptOrigin.y; rc.right += m_ptOrigin.x; rc.bottom += m_ptOrigin.y; }

	template<class _int> void MapPoint(_int &x, _int &y) const { x += m_ptOrigin.x; y += m_ptOrigin.y; }
	template<class _int> _int MapX(_int x) { return x + m_ptOrigin.x; }
	template<class _int> _int MapY(_int y) { return y + m_ptOrigin.y; }

	template<class _int> _int RevertMapX(_int x) { return x - m_ptOrigin.x; }
	template<class _int> _int RevertMapY(_int y) { return y - m_ptOrigin.y; }

protected:
	void ClipAndMapRect(const RECT *lpRect, int &x, int &y, int &xMax, int &yMax);

protected:
	MLColor			m_clrText;

	CRawPen			m_pen;

	RECT			m_rcClip;

	CRawBmpFont		*m_rawBmpFont;

	// The Opacity of painting (lines, text, images) to graph
	int				m_nOpacityPainting;

	// The origin of graph, default is 0, 0.
	POINT			m_ptOrigin;
	
	bool			m_bAlphaChannelEnabled;

};

#include "RawImage.h"
// #include "RawBmpFont.h"

#endif // !defined(AFX_RAWGRAPH_H__DB0AA2D5_3D46_4B35_9FF4_F93C76ED577B__INCLUDED_)
