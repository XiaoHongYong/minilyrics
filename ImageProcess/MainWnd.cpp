// MainWnd.cpp: implementation of the CMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWnd.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_pixfmt_rgba.h"
#include "agg_image_accessors.h"

#include "DlgAdjustHue.h"


HBITMAP LoadBitmapFromFile(LPCTSTR szFile);


struct HLS
{
	float		m_hue;         // 0.0 .. 360.0  // 
	float		m_saturation;  // 0.0 .. 1.0    // 
	float		m_luminance;   // 0.0 .. 1.0    // 
};

static unsigned char ToRGBx(float rm1, float rm2, float rh)
{
	if		(rh > 360.0f) rh -= 360.0f;
	else if (rh <   0.0f) rh += 360.0f;
	
	if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
	else if (rh < 180.0f) rm1 = rm2;
	else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

	return static_cast<unsigned char>(rm1 * 255);
}

COLORREF HLSToRGB(HLS &hls)
{
	COLORREF		clr;
    if (hls.m_saturation == 0.0) // Grauton, einfacher Fall
    {
		unsigned char	c = unsigned char(hls.m_luminance * 255.0);
		clr = RGB(c, c, c);
    }
    else
    {
		float rm1, rm2;

		if (hls.m_luminance <= 0.5f)
			rm2 = hls.m_luminance + hls.m_luminance * hls.m_saturation;  
		else
			rm2 = hls.m_luminance + hls.m_saturation - hls.m_luminance * hls.m_saturation;

		rm1 = 2.0f * hls.m_luminance - rm2;
		clr = RGB(ToRGBx(rm1, rm2, hls.m_hue + 120.0f),
				ToRGBx(rm1, rm2, hls.m_hue),
				ToRGBx(rm1, rm2, hls.m_hue - 120.0f));
    }

	return clr;
}

void RGBToHLS(COLORREF clr, HLS &hls)
{
    // Konvertierung
    unsigned char minval = min(GetRValue(clr), min(GetGValue(clr), GetBValue(clr)));
    unsigned char maxval = max(GetRValue(clr), max(GetGValue(clr), GetBValue(clr)));
    float mdiff  = float(maxval) - float(minval);
    float msum   = float(maxval) + float(minval);

    hls.m_luminance = msum / 510.0f;

    if (maxval == minval) 
    {
		hls.m_saturation = 0.0f;
		hls.m_hue = 0.0f; 
    }
    else 
    { 
		float rnorm = (maxval - GetRValue(clr)  ) / mdiff;      
		float gnorm = (maxval - GetGValue(clr)  ) / mdiff;
		float bnorm = (maxval - GetBValue(clr)  ) / mdiff;   

		hls.m_saturation = (hls.m_luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if (GetRValue(clr)   == maxval) hls.m_hue = 60.0f * (6.0f + bnorm - gnorm);
		if (GetGValue(clr) == maxval) hls.m_hue = 60.0f * (2.0f + rnorm - bnorm);
		if (GetBValue(clr)  == maxval) hls.m_hue = 60.0f * (4.0f + gnorm - rnorm);
		if (hls.m_hue > 360.0f) hls.m_hue = hls.m_hue - 360.0f;
    }
}


inline void AdjustPixHue(byte *pix, float hueOffset, float saturationRatio, float luminanceRatio)
{
	float hue, saturation, luminance;

	unsigned char minval = min(pix[0], min(pix[1], pix[2]));
	unsigned char maxval = max(pix[0], max(pix[1], pix[2]));
	float mdiff  = float(maxval) - float(minval);
	float msum   = float(maxval) + float(minval);

	luminance = msum / 510.0f;

	if (maxval == minval)
	{
		saturation = 0.0f;
		hue = 0.0f;
	}
	else
	{
		float rnorm = (maxval - pix[2]  ) / mdiff;
		float gnorm = (maxval - pix[1]  ) / mdiff;
		float bnorm = (maxval - pix[0]  ) / mdiff;

		saturation = (luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if (pix[2]   == maxval)		 hue = 60.0f * (6.0f + bnorm - gnorm);
		else if (pix[1]   == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
		else						 hue = 60.0f * (4.0f + gnorm - rnorm);
		if (hue > 360.0f) hue = hue - 360.0f;
	}

	hue += hueOffset;
	if (saturationRatio > 0)
		saturation += (float)((1.0 - saturation) * saturationRatio);
	else
		saturation *= -saturationRatio;
	ASSERT(saturation >= 0 && saturation <= 1.0);
/*	saturation += saturationRatio;
	if (saturation < 0)
		saturation = 0;
	else if (saturation > 1.0)
		saturation = 1.0;*/

	if (luminanceRatio > 0)
		luminance += (float)((1.0 - luminance) * luminanceRatio);
	else
		luminance *= -luminanceRatio;
	ASSERT(luminance >= 0 && luminance <= 1.0);

	if (saturation == 0.0)
	{
		pix[0] = pix[1] = pix[2] = unsigned char(luminance * 255.0);
	}
	else
	{
		float rm1, rm2;

		if (luminance <= 0.5f)
			rm2 = luminance + luminance * saturation;  
		else
			rm2 = luminance + saturation - luminance * saturation;

		rm1 = 2.0f * luminance - rm2;
		pix[2] = ToRGBx(rm1, rm2, hue + 120.0f);
		pix[1] = ToRGBx(rm1, rm2, hue);
		pix[0] = ToRGBx(rm1, rm2, hue - 120.0f);
	}
}


// hueOffset: 0-360
void AdjustImageHueTest24(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio)
{
	byte		*p, *pRow;

	pRow = pImage->RowPtr(0);
	for (int y = 0; y < pImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pImage->width; x++)
		{
			AdjustPixHue(p, hueOffset, saturationRatio, luminanceRatio);
			p += 3;
		}
		pRow += pImage->stride;
	}
}

// hueOffset: 0 ~ 360
// luminanceRatio, saturationRatio: 0 ~ 1.0, increase ratio
//									-1.0 ~ 0, ratio of scale
void AdjustImageHueTest(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio)
{
	if (pImage->bitCount == 24)
		AdjustImageHueTest24(pImage, hueOffset, saturationRatio, luminanceRatio);
}

// 
// void DrawText(HDC hdc, PRAW_IMAGE_DATA imgData, int x, int y, LPCTSTR szText, COLORREF clrTxt)
// {
// 	HDC		hdcDesktop = NULL;
// 	HFONT	hFont, hFontOld;
// 	LPCTSTR	szStr = _T("中国，China, 斯坦姆福德-宾尼特智商等级测试");
// 	LOGFONT lgfont = {50, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
// 		ANTIALIASED_QUALITY/*ANTIALIASED_QUALITY*/, DEFAULT_PITCH | FF_ROMAN, _T("Verdana")};
// 
// 	if (hdc == NULL)
// 	{
// 		hdcDesktop = ::GetDC(NULL);
// 		hdc = hdcDesktop;
// 	}
// 
// 	HBITMAP		hbmpTemp;
// 	HDC			hdcMem;
// 
// 	hdcMem = CreateCompatibleDC(hdc);
//     HBITMAP		hbmp = CreateCompatibleBitmap(hdc, imgData->width, imgData->height);
//     hbmpTemp = (HBITMAP)SelectObject(hdcMem, hbmp);
// 
// 	imgData.BitBlt(hdcMem, 0, 0,  imgData->width, imgData->width, 0, 0, SRCCOPY);
// 
// 	SetTextColor(hdcMem, clrTxt);
// 	SetBkMode(hdcMem, TRANSPARENT);
// 
// 	hFont = CreateFontIndirect(&lgfont);
// 	hFontOld = (HFONT)SelectObject(hdcMem, hFont);
// 
// 	TextOut(hdcMem, x, y, szText, _tcslen(szText));
// 
// 	SelectObject(hdcMem, hFontOld);
// 	DeleteObject(hFont);
// 
// 	imgData.CopyFromDC(hdcMem, CRect(0, 0, imgData->width, imgData->height));
// 
// 	SelectObject(hdcMem, hbmpTemp);
// 	DeleteDC(hdcMem);
// 
// 	if (hdcDesktop)
// 		::ReleaseDC(NULL, hdcDesktop);
// }

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainWnd::CMainWnd()
{
	m_bTranslucencyLayered = false;
}

CMainWnd::~CMainWnd()
{

}

//unsigned long		*nGdiplus;

bool CMainWnd::OnCreate()
{
	if (!CBaseWnd::OnCreate())
		return false;

//	CGdiplusGraphics::Startup(&nGdiplus);

	HMENU	m_hMenu = ::LoadMenu(GetAppInstance(), MAKEINTRESOURCE(IDC_IMAGEPROCESS));

	::SetMenu(m_hWnd, m_hMenu);

	CRect		rc;

	GetClientRect(&rc);

	IMLGraphicsBase	*pGraph = GetGraphics();
	m_memGraph.Create(rc.Width(), rc.Height(), pGraph);
	br.CreateSolidBrush(MLColor(RGB(255, 0, 0)));

	ReleaseGraphics(pGraph);

	m_img.Attach(::LoadBitmapFromFile(_T("c:\\1.jpg")));
	m_img.GetOrginalSize(m_img.m_cx, m_img.m_cy);

	m_rawImage.Load(_T("c:\\1.jpg"));
	m_rawImage2.Load(_T("c:\\2.jpg"));
	m_rawImage3.Load(_T("c:\\2.png"));

	m_rawFont.Create(_T("Tahoma"), _T("Tahoma"), 43, FW_NORMAL, false);

	m_font.Create(_T("Tahoma"), _T("Tahoma"), 13, FW_NORMAL, false);

	return true;
}

void CMainWnd::OnDestroy()
{
	CBaseWnd::OnDestroy();
	PostQuitMessage(0);
}

int GetKernelFactor(int *kernel, int nWidth)
{
	int		f = 0;

	for (int i = nWidth * nWidth - 1; i >= 0; i--)
		f += kernel[i];

	return f;
}

void CMainWnd::OnPaint(IMLGraphicsBase *pGraph)
{
	CRect		rc;
	int			width, height;

	GetClientRect(&rc);
	width  = rc.Width();
	height = rc.Height();

	int			nOpacityPainting;

	if (m_rawImage.IsValid())
		m_rawImage.Blt(&m_memGraph, 0, 0);

	nOpacityPainting = m_memGraph.SetOpacityPainting(128);

	//if (m_rawImage2.IsValid() && m_rawImage3.IsValid())
	//	m_rawImage2.MaskBlt(&m_memGraph, 0, 0, &m_rawImage3, BPM_BLEND);
	m_memGraph.SetOrigin(CPoint(0, 0));

	// m_memGraph.SetClipBoundBox(CRect(5, 5, 50, 100));

	m_memGraph.SetOrigin(CPoint(5, 10));
	m_rawImage2.Blt(&m_memGraph, 0, 0);

	m_memGraph.SetOrigin(CPoint(15, 20));
	m_rawImage2.Blt(&m_memGraph, 0, 0);

	m_memGraph.SetOpacityPainting(nOpacityPainting);


	m_memGraph.SetFont(&m_rawFont);

	CRect	rcFill;

	rcFill.SetRect(10, 20, 10 + 20, 20 + 10);

	m_memGraph.SetOrigin(CPoint(0, 0));
	m_memGraph.Clear(255, 0, 0, 255, &rcFill);
	m_memGraph.TextOut(-110, 10, _T("Hello"), 5);

	m_memGraph.SetOrigin(CPoint(15, 35));
	m_memGraph.Clear(0, 255, 0, 255, &rcFill);
	m_memGraph.TextOut(10, 10, _T("Hello2"), 6);

	m_memGraph.DrawToGraphics(pGraph, 0, 0,  rc.Width(), rc.Height(), 0, 0);
	// pGraph->DrawImage(&m_img, 0, 0,  m_img.m_cx, m_img.m_cy, 0, 0);
}

void HLSTest();

void CMainWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	switch (uID)
	{
// 	case IDC_BLUR3:
// 		{
// 			DWORD		dwBeg, dwEnd;
// 			int			kernel[9] =  { 0, 1, 0, 1, 4, 1, 0, 1, 0 };
// 
// 			dwBeg = GetTickCount();
// 
// 			m_imgData.Filter(kernel, 3, GetKernelFactor(kernel, 3));
// 
// 			dwEnd = GetTickCount();
// 			DBGLOG1(_T("Cost: %d"), dwEnd - dwBeg);
// 
// 			InvalidateRect();
// 		}
// 		break;
	case IDC_ADJUST_IMAGE_HUE:
		{
			CDlgAdjustHue		dlg;

			// HLSTest();
			PRAW_IMAGE_DATA		pOrg = new RAW_IMAGE_DATA;

			pOrg->Create(m_rawImage.width(), m_rawImage.height(), m_rawImage.GetHandle()->bitCount);
			dlg.m_rawImgOrg.Attach(pOrg);
			m_rawImage.Blt(&dlg.m_rawImgOrg, 0, 0, BPM_COPY);

			dlg.m_pMainWnd = this;

			if (dlg.DoModal(this) != IDOK)
				return;
		}
		break;
	case IDC_HLS:
		{
			m_rawImage.Destroy();
			PRAW_IMAGE_DATA		hImage = new RAW_IMAGE_DATA;
			hImage->Create(360, 50, 32);
			m_rawImage.Attach(hImage);

			byte		*p, *pRow;
			HLS			hls;
			pRow = hImage->RowPtr(0);
			for (int y = 0; y < hImage->height; y++)
			{
				p = pRow;
				for (int x = 0; x < hImage->width; x++)
				{
					hls.m_hue = x;
					hls.m_luminance = 0.5;
					hls.m_saturation = (float)y / hImage->height;
					*((COLORREF*)p) = HLSToRGB(hls);
					p[3] = 0xff;
					p += 4;
				}
				pRow += hImage->stride;
			}
			InvalidateRect();
		}
		break;
	case IDC_OPEN:
		{
			OnOpenImage();
			break;
		}
	case IDM_ABOUT:
	   DialogBox(GetAppInstance(), (LPCTSTR)IDD_ABOUTBOX, m_hWnd, (DLGPROC)About);
	   break;
	case IDM_EXIT:
	   Destroy();
	   break;
	case IDC_TEST1:
		break;
	}
}

/*
void CMainWnd::OnPaint(IMLGraphicsBase *pGraph)
{
	HDC		hdc = pGraph->GetHandle();

    RECT rt;
    GetClientRect(&rt);

    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    //============================================================ 
    //Creating compatible DC and a bitmap to render the image 
    BITMAPINFO bmp_info; 
    bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    bmp_info.bmiHeader.biWidth = width; 
    bmp_info.bmiHeader.biHeight = height; 
    bmp_info.bmiHeader.biPlanes = 1; 
    bmp_info.bmiHeader.biBitCount = 32; 
    bmp_info.bmiHeader.biCompression = BI_RGB; 
    bmp_info.bmiHeader.biSizeImage = 0; 
    bmp_info.bmiHeader.biXPelsPerMeter = 0; 
    bmp_info.bmiHeader.biYPelsPerMeter = 0; 
    bmp_info.bmiHeader.biClrUsed = 0; 
    bmp_info.bmiHeader.biClrImportant = 0; 

    HDC mem_dc = ::CreateCompatibleDC(hdc); 

    void* buf = 0; 

    HBITMAP bmp = ::CreateDIBSection( 
        mem_dc, 
        &bmp_info, 
        DIB_RGB_COLORS, 
        &buf, 
        0, 
        0 
    ); 

    // Selecting the object before doing anything allows you 
    // to use AGG together with native Windows GDI.
    HBITMAP temp = (HBITMAP)::SelectObject(mem_dc, bmp);


	



    //------------------------------------------------------------ 
    // Display the image. If the image is B-G-R-A (32-bits per pixel)
    // one can use AlphaBlend instead of BitBlt. In case of AlphaBlend
    // one also should clear the image with zero alpha, i.e. rgba8(0,0,0,0)

    ::BitBlt(
      hdc,  
      rt.left,      
      rt.top,      
      width,  
      height, 
      mem_dc,
      0,
      0,     
      SRCCOPY
    );

    // Free resources 
    ::SelectObject(mem_dc, temp); 
    ::DeleteObject(bmp); 
    ::DeleteObject(mem_dc);
}*/

void CMainWnd::OnSize(int cx, int cy)
{
	IMLGraphicsBase	*pGraph = GetGraphics();

	m_memGraph.Destroy();
	m_memGraph.Create(cx, cy, pGraph);

	ReleaseGraphics(pGraph);
}

void CMainWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnOpenImage();
}

void TestUpdateLayeredWindow(HWND hWnd, HBITMAP hbmp, int nBmpWidth, int nBmpHeight)
{
	HDC		hdc, hdcSrc;
	HBITMAP	hbmpOld;
	BLENDFUNCTION	blend;
	POINT	ptSrc, ptDest;
	SIZE	sizeWnd;
	RECT	rcWnd;
	DWORD	dwStyle;

	hdc = GetDC(hWnd);

	// 首先设置WS_EX_LAYERED属性
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	// SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle & ~ (WS_EX_LAYERED));
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

	hdcSrc = CreateCompatibleDC(hdc);

	hbmpOld = (HBITMAP)SelectObject(hdcSrc, hbmp);

#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA                0x01
#endif
// 
// 	::SetTextColor(hdcSrc, RGB(0, 255, 255));
// 	::SetBkColor(hdcSrc, RGB(255, 0, 255));
// 	// ::SetBkMode(hdcSrc, OPAQUE);
// 
// 	LPCTSTR		szText = _T("Hello world");
// 	::TextOut(hdcSrc, 10, 10, szText, _tcslen(szText));
// 
// 	::MoveToEx(hdcSrc, 10, 10, NULL);
// 	::LineTo(hdcSrc, 100, 10);

	GetWindowRect(hWnd, &rcWnd);

	blend.AlphaFormat = AC_SRC_ALPHA;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;

	ptSrc.x = ptSrc.y = 0;
	ptDest.x = rcWnd.left;
	ptDest.y = rcWnd.top;
	sizeWnd.cx = nBmpWidth;
	sizeWnd.cy = nBmpHeight;

	BOOL bRet = UpdateLayeredWindow(hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

	SelectObject(hdcSrc, hbmpOld);

	ReleaseDC(hWnd, hdc);
	DeleteDC(hdcSrc);
}

void CMainWnd::OnOpenImage()
{
	CFileOpenDlg	dlg(_T(""), NULL, _T("Pic files\0*.jpg;*.bmp;*.gif;*.png\0\0"), 0);

	if (dlg.DoModal(CBaseWnd::FromHandle(m_hWnd)) != IDOK)
		return;

	CCharBuffer	buff;

	OpenFileToBuffer(dlg.GetOpenFile(), buff);

	m_rawImage.Attach(LoadRawImageDataFromMem(buff.c_str(), buff.size()));
	// m_rawImage.Load(dlg.GetOpenFile());

	InvalidateRect(NULL, true);
}

LRESULT CMainWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		{
			if (m_bTranslucencyLayered)
			{
				// updates the position, size, shape, content, and translucency of a layered window
				return DefWindowProc(m_hWnd, message, wParam, lParam);
			}
			else
			{
				break;
			}
		}
		return 0;
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}
