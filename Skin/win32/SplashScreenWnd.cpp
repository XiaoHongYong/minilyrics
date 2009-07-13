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
	// Load image, then display it.
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

	return true;
}

LRESULT CSplashScreenWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
// 	switch (message)
// 	{
// 	case WM_INITDIALOG:
// 		return OnInitDialog();
// 		break;
// 	default:
		return CBaseWnd::WndProc(message, wParam, lParam);
//	}
}

BOOL CSplashScreenWnd::UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph)
{
	HDC		hdc, hdcSrc;
	CRect	rcWnd;
	POINT	ptSrc, ptDest;
	SIZE	sizeWnd;
	DWORD	dwStyle, dwStyleNew;
	BLENDFUNCTION	blend;

	hdc = GetDC(m_hWnd);

	hdcSrc = pGraph->GetHandle();

	// ????WS_EX_LAYERED??
	dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	dwStyleNew = dwStyle | WS_EX_LAYERED;
	if (dwStyle != dwStyleNew)
		SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleNew);

	GetWindowRect(&rcWnd);

	blend.AlphaFormat = AC_SRC_ALPHA;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;

	ptSrc.x = ptSrc.y = 0;
	ptDest.x = rcWnd.left;
	ptDest.y = rcWnd.top;
	sizeWnd.cx = rcWnd.Width();
	sizeWnd.cy = rcWnd.Height();

	BOOL bRet = UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);
	//BOOL bRet = UpdateLayeredWindow(m_hWnd, NULL, NULL, NULL, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

	ReleaseDC(m_hWnd, hdc);

	return bRet;
}
