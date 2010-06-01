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
	bool IsValid() const { return false; }

	virtual void Destroy();

	bool Load(LPCTSTR szFile);

	bool Blt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc);

	bool StretchBlt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc);

	int width() const { return m_cx; }
	int height() const { return m_cy; }

	bool GetOrginalSize(int &nWidth, int &nHeight);

	void Attach(IMAGE_HANDLE hBmp);
	void Detach();

	IMAGE_HANDLE GetHandle() const { return m_hImage; }

public:
	int				m_cx, m_cy, m_nDepth;

	// for CSFImage
	IMAGE_HANDLE	m_hImage;

};


#endif // _MLIMAGE_INC_
