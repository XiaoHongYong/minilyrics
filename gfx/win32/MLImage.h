#ifndef _MLIMAGE_INC_
#define _MLIMAGE_INC_

#pragma once

#include "ImageFile.h"


class CMLImage
{
public:
	CMLImage(void);
	virtual ~CMLImage(void);

public:
	bool IsValid() const { return m_hBmp != NULL; }

	virtual void Destroy();

	bool Load(LPCTSTR szFile);

	bool Blt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc);

	bool StretchBlt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc);

	int width() const { return m_cx; }
	int height() const { return m_cy; }

	bool GetOrginalSize(int &nWidth, int &nHeight);

	void Attach(HBITMAP hBmp);
	void Detach();

	HBITMAP GetHandle() const { return m_hBmp; }

public:
//	int			width, height;
	int			m_cx;
	int			m_cy;

protected:
	HBITMAP		m_hBmp;

};


#endif // _MLIMAGE_INC_
