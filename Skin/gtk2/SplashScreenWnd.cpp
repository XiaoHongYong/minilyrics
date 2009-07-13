// SplashScreenWnd.cpp: implementation of the CSplashScreenWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SplashScreenWnd.h"

CSplashScreenWnd::CSplashScreenWnd()
{
}

CSplashScreenWnd::~CSplashScreenWnd()
{
}

bool CSplashScreenWnd::Show(LPCTSTR szImageFile)
{
/*	// Load image, then display it.
	CRawImage		image;
	CRawGraph		memGraph;

	if (!image.Load(szImageFile))
	{
		ERRLOG1(_T("Load Splash Image file: %s, FAILED."), szImageFile);
		return false;
	}

	int				x, y;

	x = (GetSystemMetrics(SM_CXSCREEN) - image.m_cx) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - image.m_cy) / 2;

	if (!Create(NULL, x, y, image.m_cx, image.m_cy, NULL, DS_NOIDLEMSG))
		return false;

	IMLGraphicsBase	*pGraph = GetGraphics();

	ASSERT(pGraph);
	if (!pGraph)
	{
		Destroy();
		return false;
	}

	memGraph.Create(image.m_cx, image.m_cy, pGraph, 32);

	image.Blt(&memGraph, 0, 0, BPM_BLEND);

	UpdateLayeredWindowUsingMemGraph(&memGraph);

	ReleaseGraphics(pGraph);

	ShowWindow(SW_SHOW);

	SetTopmost(true);
	SetToolWindow(true);
*/
	return true;
}
