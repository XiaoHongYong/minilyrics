#include "StdAfx.h"
#include "gfx.h"
#include "MLGraphics.h"
#include "ImageFile.h"


IMAGE_HANDLE LoadBitmapFromFile(LPCTSTR szFile, IMLGraphicsBase *pGraph)
{
	IMAGE_GTK2		*pImage = new IMAGE_GTK2;
	CMLGraphics		*pGtkGraph = (CMLGraphics *)pGraph;

	if (pGraph)
		pImage->m_pPixmap = gdk_pixmap_create_from_xpm(pGtkGraph->GetDrawable(), &pImage->m_pmask, &pImage->m_transparentColor, szFile);
	else
	{
		// MLDebug("Begin gdk_pixmap_create_from_xpm\n");
		assert(g_GtkContextObj.GetGtkWindow()->window);
		pImage->m_pPixmap = gdk_pixmap_create_from_xpm(g_GtkContextObj.GetGtkWindow()->window, &pImage->m_pmask, &pImage->m_transparentColor, szFile);
		assert(pImage->m_pPixmap);
	}

	if (pImage->m_pPixmap)
	{
		gdk_drawable_get_size(pImage->m_pPixmap, &pImage->m_nWidth, &pImage->m_nHeight);
		pImage->m_nDepth = gdk_drawable_get_depth(pImage->m_pPixmap);
		return pImage;
	}

	delete pImage;
	return NULL;
}

IMAGE_HANDLE LoadBitmapFromMem(const char *data, int nLen, IMLGraphicsBase *pGraph)
{
	return NULL;
}

void ReleaseImageHandle(IMAGE_HANDLE pImage)
{
	if (pImage->m_pPixmap)
		gdk_pixmap_unref(pImage->m_pPixmap);

	delete pImage;
}
