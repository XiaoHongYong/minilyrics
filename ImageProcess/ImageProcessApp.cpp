// ImageProcess.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWnd.h"

HBITMAP LoadBitmapFromFile(LPCTSTR szFile)
{
	HANDLE		hFile = NULL;
	DWORD		dwFileSize;
	LPVOID		pvData = NULL;
	HGLOBAL		hGlobal = NULL;
	DWORD		dwBytesRead = 0;
	LPSTREAM	pstm = NULL;
	LPPICTURE	pPicture = NULL;
	HRESULT		hr;
	HBITMAP		hBitmap;
	BOOL		bRead;

	// open file
	hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!hFile)
		goto FAILED;

	// get file size
	dwFileSize = GetFileSize(hFile, NULL);
	if (-1 == dwFileSize)
		goto FAILED;


	// alloc memory based on file size
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
	if (!hGlobal)
		goto FAILED;

	pvData = GlobalLock(hGlobal);
	if (!pvData)
		goto FAILED;


	// read file and store in global memory
	bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
	if (!bRead)
		goto FAILED;


	// create IStream* from global memory
	hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
	if (!SUCCEEDED(hr) || pstm == NULL)
		goto FAILED;

	// Create IPicture from image file
	hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID
		*)&pPicture);
	if (!SUCCEEDED(hr) || pPicture == NULL)
		goto FAILED;

	pstm->Release();

	// get the bitmap handle
	if (S_OK != pPicture->get_Handle((OLE_HANDLE FAR *) &hBitmap))
	{
		ERRLOG0(_T("Can't get bitmap handle from picture"));
		goto FAILED;
	}

	hBitmap = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);

	pPicture->Release();

	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);
	CloseHandle(hFile);

	return hBitmap;
FAILED:

	if (hGlobal)
	{
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
	}

	if (hFile)
		CloseHandle(hFile);

	if (pstm)
		pstm->Release();
	if (pPicture)
		pPicture->Release();

	return NULL;
}

int WINAPI _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	MSG msg;
	CMainWnd	wnd;

	InitBaseFrameWork(hInstance, NULL, _T("ImageProcess.ini"), _T("ImageProcess"));

	wnd.Create(_T("Image process"), CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, WS_OVERLAPPEDWINDOW);
	wnd.ShowWindow(SW_SHOW);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


/*

#include "agg_trans_bilinear.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_interpolator_trans.h"
#include "agg_span_subdiv_adaptor.h"

void ScaleImage(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, CRect &rcSrc)
{
    typedef agg::pixfmt_bgra32                             pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    pixfmt            pixfDest(bufDest);
    pixfmt            pixfSrc(bufSrc);

	float		dx, dy, xSrc, ySrc;
	uint8		*rowSrc, *rowDest, *pixSrc, *pixDest;

	dx = (float)rcSrc.Width() / rcDest.Width();
	dy = (float)rcSrc.Height() / rcDest.Height();
	ySrc = rcSrc.top;

	rowDest = pixfDest.row_ptr(rcDest.top);

	for (int y = rcDest.top; y < rcDest.bottom; y++)
	{
		xSrc = rcSrc.left;
		rowSrc = pixfSrc.row_ptr(ySrc);
		pixDest = rowDest;
		for (int x = rcDest.left; x < rcDest.right; x++)
		{
			pixSrc = rowSrc + ((int)xSrc) * pixfmt::pix_width;
			pixDest[0] = pixSrc[0];
			pixDest[1] = pixSrc[1];
			pixDest[2] = pixSrc[2];
			pixDest[3] = pixSrc[3];

			xSrc += dx;
			pixDest += pixfmt::pix_width;
		}
		ySrc += dy;
		rowDest += pixfDest.stride();
	}
}

void ScaleImage3(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, CRect &rcSrc)
{
    typedef agg::pixfmt_bgra32                             pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    pixfmt            pixfDest(bufDest);
    pixfmt            pixfSrc(bufSrc);

	float		dx, dy, xSrc, ySrc;
	uint8		*rowSrc, *rowDest, *pixSrc, *pixDest;

	dx = (float)rcSrc.Width() / rcDest.Width();
	dy = (float)rcSrc.Height() / rcDest.Height();
	ySrc = rcSrc.top;

	rowDest = pixfDest.row_ptr(rcDest.top);

	for (int y = rcDest.top; y < rcDest.bottom; y++)
	{
		xSrc = rcSrc.left;
		rowSrc = pixfSrc.row_ptr(ySrc);
		pixDest = rowDest;
		for (int x = rcDest.left; x < rcDest.right; x++)
		{
			pixSrc = rowSrc + ((int)xSrc) * pixfmt::pix_width;
			pixDest[0] = pixSrc[0];
			pixDest[1] = pixSrc[1];
			pixDest[2] = pixSrc[2];
			pixDest[3] = pixSrc[3];

			xSrc += dx;
			pixDest += pixfmt::pix_width;
		}
		ySrc += dy;
		rowDest += pixfDest.stride();
	}
}

void ScaleImage4(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, CRect &rcSrc)
{
    typedef agg::pixfmt_bgra32                             pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    pixfmt            pixfDest(bufDest);
    pixfmt            pixfSrc(bufSrc);

	float		dx, dy, xSrc, ySrc;
	uint8		*rowSrc, *rowDest, *pixSrc, *pixSrcNext, *pixDest;

	dx = (float)rcSrc.Width() / rcDest.Width();
	dy = (float)rcSrc.Height() / rcDest.Height();
	ySrc = rcSrc.top;

	rowDest = pixfDest.row_ptr(rcDest.top);

	for (int y = rcDest.top; y < rcDest.bottom; y++)
	{
		xSrc = rcSrc.left;
		rowSrc = pixfSrc.row_ptr(ySrc);
		pixDest = rowDest;
		for (int x = rcDest.left; x < rcDest.right; x++)
		{
			pixSrc = rowSrc + ((int)xSrc) * pixfmt::pix_width;
			pixSrcNext = pixSrc + pixfSrc.stride();

			// f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
			float u, v;
			u = xSrc - (int)xSrc;
			v = ySrc - (int)ySrc;

			for (int i = 0; i < 4; i++)
				pixDest[i] = pixSrc[i];
				//pixDest[i] = (1-u) * (1-v) * pixSrc[i] + (1-u)*v*pixSrcNext[i] + 
				//u * (1-v) * pixSrc[i + pixfmt::pix_width] + u*v*pixSrcNext[i + pixfmt::pix_width];

			xSrc += dx;
			pixDest += pixfmt::pix_width;
		}
		ySrc += dy;
		rowDest += pixfDest.stride();
	}
}

void ScaleImage2(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, CRect &rcSrc)
{
    typedef agg::pixfmt_bgra32                             pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

	agg::scanline_u8  g_scanline;
	agg::rasterizer_scanline_aa<> g_rasterizer;
    pixfmt            pixf(bufDest);
    pixfmt_pre        pixf_pre(bufDest);
    renderer_base     rb(pixf);
    renderer_base_pre rb_pre(pixf_pre);

    renderer_solid r(rb);


    rb.clear(agg::rgba(1, 1, 1));

    g_rasterizer.clip_box(0, 0, bufDest.width(), bufDest.height());
    g_rasterizer.reset();
    g_rasterizer.move_to_d(rcDest.left, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.bottom);
    g_rasterizer.line_to_d(rcDest.left, rcDest.bottom);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    pixfmt pixf_img(bufSrc);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_img);

	double		ploygon[8];
	int			g_x1, g_y1, g_x2, g_y2;

	g_x1 = rcSrc.left;
	g_y1 = rcSrc.top;
	g_x2 = rcSrc.right;
	g_y2 = rcSrc.bottom;

	ploygon[0] = rcDest.left;
	ploygon[1] = rcDest.top;
	ploygon[2] = rcDest.right;
	ploygon[3] = rcDest.top;
	ploygon[4] = rcDest.right;
	ploygon[5] = rcDest.bottom;
	ploygon[6] = rcDest.left;
	ploygon[7] = rcDest.bottom;

    agg::trans_bilinear tr(ploygon, g_x1, g_y1, g_x2, g_y2);
    if(tr.is_valid())
    {
        typedef agg::span_interpolator_linear<agg::trans_bilinear> interpolator_type;
        interpolator_type interpolator(tr);

        typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
                                                interpolator_type> span_gen_type;
        span_gen_type sg(ia, interpolator, filter);
        agg::render_scanlines_aa(g_rasterizer, g_scanline, rb_pre, sa, sg);
    }
}

*/