#include "StdAfx.h"
#include "gfx.h"
#include "MLImage.h"

CMLImage::CMLImage(void)
{
	m_pPixmap = NULL;
	m_pmask = NULL;
	// m_transparentColor = 0;
	m_cx = 0;
	m_cy = 0;

	m_hImage = NULL;
}

CMLImage::~CMLImage(void)
{
	Destroy();
}

void CMLImage::Destroy()
{
	if (m_pPixmap)
	{
		gdk_pixmap_unref(m_pPixmap);
		m_pPixmap = NULL;
	}

	if (m_hImage)
	{
		delete m_hImage;
		m_hImage = NULL;
	}
}

bool CMLImage::Load(LPCTSTR szFile, IMLGraphicsBase *pGraph)
{
	CMLGraphics		*pGtkGraph = (CMLGraphics *)pGraph;

	if (pGraph)
		m_pPixmap = gdk_pixmap_create_from_xpm(pGtkGraph->GetDrawable(), &m_pmask, &m_transparentColor, szFile);
	else
	{
		// MLDebug("Begin gdk_pixmap_create_from_xpm\n");
		assert(g_GtkContextObj.GetGtkWindow()->window);
		m_pPixmap = gdk_pixmap_create_from_xpm(g_GtkContextObj.GetGtkWindow()->window, &m_pmask, &m_transparentColor, szFile);
		assert(m_pPixmap);
	}

	if (m_pPixmap)
	{
		gdk_drawable_get_size(m_pPixmap, &m_cx, &m_cy);
		m_nDepth = gdk_drawable_get_depth(m_pPixmap);
	}
	else
	{
		m_cy = 0;
		m_cx = 0;
	}

	return m_pPixmap != NULL;
}

bool CMLImage::Blt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc)
{
	pGraphics->DrawImage(this, xDest, yDest, widthDest, heightDest, xSrc, ySrc);

	return true;
}

bool CMLImage::StretchBlt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc)
{
	pGraphics->DrawImageStretch(this, xDest, yDest, widthDest, heightDest, xSrc, ySrc, widthSrc, heightSrc);

	return true;
}

bool CMLImage::GetOrginalSize(int &nWidth, int &nHeight)
{
	gdk_drawable_get_size(m_pPixmap, &nWidth, &nHeight);

	m_cx = nWidth;
	m_cy = nHeight;

	return true;
}

void CMLImage::Attach(IMAGE_HANDLE hBmp)
{
	m_hImage = hBmp;

	m_pPixmap = m_hImage->m_pPixmap;
	m_pmask = m_hImage->m_pmask;
	m_nDepth = m_hImage->m_nDepth;
	m_cy = m_hImage->m_nHeight;
	m_cx = m_hImage->m_nWidth;
	m_transparentColor = m_hImage->m_transparentColor;
}

void CMLImage::Detach()
{
	m_hImage = NULL;
}
