// RawImage.h: interface for the CRawImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAWIMAGE_H__23B94FD7_4391_4E37_87FC_1404458B776E__INCLUDED_)
#define AFX_RAWIMAGE_H__23B94FD7_4391_4E37_87FC_1404458B776E__INCLUDED_

#pragma once

class CRawGraph;

bool BltRawImage(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, PRAW_IMAGE_DATA pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc);

//
//
//
class CRawImage  
{
public:
	CRawImage();
	virtual ~CRawImage();

	bool IsValid() const { return m_hImage != NULL; }

	virtual void Destroy();

	bool Load(LPCTSTR szFile);

	void Blt(CRawGraph *pGraphics, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND);
	bool Blt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND);

	// Use mask to blt image
	void MaskBlt(CRawGraph *pGraphics, int xDest, int yDest, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND);
	bool MaskBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND);

	void Blt(CRawImage *pGraphics, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);
	bool Blt(CRawImage *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

	void StretchBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND);
	bool StretchBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND);

	void StretchBlt(CRawImage *pGraphics, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);
	bool StretchBlt(CRawImage *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

	int width() const { return m_cx; }
	int height() const { return m_cy; }

	bool GetOrginalSize(int &nWidth, int &nHeight);

	void Attach(PRAW_IMAGE_DATA hImage);
	void Detach();

	PRAW_IMAGE_DATA GetHandle() const { return m_hImage; }

protected:
	bool MaskBlt(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, int xSrc, int ySrc, PRAW_IMAGE_DATA pImageMask, int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc);
	bool StretchBlt(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, agg::rect_f &rcSrc, BlendPixMode bpm, int nOpacitySrc);

public:
	int			m_x;
	int			m_y;
	int			m_cx;
	int			m_cy;
	
protected:
	RAW_IMAGE_DATA		*m_hImage;

};

#endif // !defined(AFX_RAWIMAGE_H__23B94FD7_4391_4E37_87FC_1404458B776E__INCLUDED_)
