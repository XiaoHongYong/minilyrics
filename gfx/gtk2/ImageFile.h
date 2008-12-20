
#pragma once

struct IMAGE_GTK2
{
	GdkPixmap		*m_pPixmap;
	GdkBitmap		*m_pmask;
	GdkColor		m_transparentColor;
	int				m_nWidth, m_nHeight, m_nDepth;
};

typedef IMAGE_GTK2	*IMAGE_HANDLE;


IMAGE_HANDLE LoadBitmapFromFile(LPCTSTR szFile, IMLGraphicsBase *pGraph = NULL);

IMAGE_HANDLE LoadBitmapFromMem(const char *data, int nLen, IMLGraphicsBase *pGraph = NULL);

void ReleaseImageHandle(IMAGE_HANDLE hImage);
