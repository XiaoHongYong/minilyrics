// RawImageData.cpp: implementation of the RawImageData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ILIO.h"
#include "RawImageData.h"

#include "../SDK/Agg/include/agg_scanline_p.h"
#include "../SDK/Agg/include/agg_renderer_scanline.h"
#include "../SDK/Agg/include/agg_pixfmt_rgba.h"
#include "../SDK/Agg/include/agg_pixfmt_rgb.h"
#include "../SDK/Agg/include/agg_rasterizer_scanline_aa.h"

#define BMPWIDTHBYTES(dwWidth, dwBitCount)	(( ( dwWidth * dwBitCount + 7 ) / 8 + 3 ) & ~3)

PRAW_IMAGE_DATA Convert32BppTo24BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA	dst = new RAW_IMAGE_DATA;
	uint8		*rowSrc, *rowDst;

	dst->Create(src->width, src->height, 24);

	rowSrc = src->RowPtr(0);
	rowDst = dst->RowPtr(0);
	for (int y = 0; y < src->height; y++)
	{
		uint8	*pixDst, *pixSrc;
		pixDst = rowDst;
		pixSrc = rowSrc;
		for (int x = 0; x < src->width; x++)
		{
			pixDst[0] = pixSrc[0];
			pixDst[1] = pixSrc[1];
			pixDst[2] = pixSrc[2];
			pixDst += 3;
			pixSrc += 4;
		}
		rowSrc += src->stride;
		rowDst += dst->stride;
	}

	return dst;
}

PRAW_IMAGE_DATA Convert16BppTo24BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA	dst = new RAW_IMAGE_DATA;
	uint8		*rowSrc, *rowDst;

#define CLRMAX	256				//最大颜色数
#define	RMASK	0x1F			//0000 00  00 000   1 1111
#define RMOVE	0				//右移 0 位
#define RMAX	32				//最大颜色数
#define GMASK	0x3E0			//0000 00  11 111   0 0000
#define GMOVE	5				//
#define GMAX	32				//最大颜色数
#define BMASK	0x7C00			//1111 11  00 000   0 0000
#define BMOVE	10				//
#define BMAX	32				//最大颜色数

	uint8	clrTable[(int)RMAX];//颜色转换表
	for (int i = 0 ; i < (int)RMAX; i++)
		clrTable[i] = (uint8)((float)i * CLRMAX / RMAX);
	uint16	clrSrc;

	dst->Create(src->width, src->height, 24);

	rowSrc = src->RowPtr(0);
	rowDst = dst->RowPtr(0);
	for (int y = 0; y < src->height; y++)
	{
		uint8	*pixDst;
		uint16	*pixSrc;
		pixDst = rowDst;
		pixSrc = (uint16*)rowSrc;
		for (int x = 0; x < src->width; x++)
		{
			clrSrc = *pixSrc;
			pixDst[0] = clrTable[clrSrc & RMASK];
			pixDst[1] = clrTable[(clrSrc & GMASK) >> GMOVE];
			pixDst[2] = clrTable[(clrSrc & BMASK) >> BMOVE];
			pixDst += 3;
			pixSrc++;
		}
		rowSrc += src->stride;
		rowDst += dst->stride;
	}

	return dst;
}

PRAW_IMAGE_DATA Convert8BppTo24BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA	dst = new RAW_IMAGE_DATA;
	uint8		*rowSrc, *rowDst;

	dst->Create(src->width, src->height, 24);

	rowSrc = src->RowPtr(0);
	rowDst = dst->RowPtr(0);
	for (int y = 0; y < src->height; y++)
	{
		uint8	*pixDst, *pixSrc;
		pixDst = rowDst;
		pixSrc = rowSrc;
		for (int x = 0; x < src->width; x++)
		{
			*pixDst = src->pallete[*pixSrc].rgbBlue;
			pixDst++;
			*pixDst = src->pallete[*pixSrc].rgbGreen;
			pixDst++;
			*pixDst = src->pallete[*pixSrc].rgbRed;
			pixDst++;
			pixSrc++;
		}
		rowSrc += src->stride;
		rowDst += dst->stride;
	}

	return dst;
}

PRAW_IMAGE_DATA Convert4BppTo24BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA	dst = new RAW_IMAGE_DATA;
	uint8		*rowSrc, *rowDst;

	dst->Create(src->width, src->height, 24);

	rowSrc = src->RowPtr(0);
	rowDst = dst->RowPtr(0);
	for (int y = 0; y < src->height; y++)
	{
		uint8	*pixDst, *pixSrc;
		pixDst = rowDst;
		pixSrc = rowSrc;
		for (int x = 0; x < src->width; x++)
		{
			uint8	px;
			if (x % 2 == 0)
				px = (*pixSrc >> 4 )& 0xF;
			else
			{
				px = *pixSrc & 0xF;
				pixSrc++;
			}
			*pixDst = src->pallete[px].rgbBlue;
			pixDst++;
			*pixDst = src->pallete[px].rgbGreen;
			pixDst++;
			*pixDst = src->pallete[px].rgbRed;
			pixDst++;
		}
		rowSrc += src->stride;
		rowDst += dst->stride;
	}

	return dst;
}

PRAW_IMAGE_DATA ConvertTo24BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA		dst = NULL;
	switch (src->bitCount)
	{
	case 24:
		return src;
	case 32:
		dst = Convert32BppTo24BppRawImage(src);
		break;
	case 16:
		dst = Convert16BppTo24BppRawImage(src);
		break;
	case 8:
		dst = Convert8BppTo24BppRawImage(src);
		break;
	case 4:
		dst = Convert4BppTo24BppRawImage(src);
		break;
	}

	FreeRawImage(src);

	return dst;
}

PRAW_IMAGE_DATA Convert24BppTo32BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA	dst = new RAW_IMAGE_DATA;
	uint8		*rowSrc, *rowDst;

	dst->Create(src->width, src->height, 32);

	rowSrc = src->RowPtr(0);
	rowDst = dst->RowPtr(0);
	for (int y = 0; y < src->height; y++)
	{
		uint8	*pixDst, *pixSrc;
		pixDst = rowDst;
		pixSrc = rowSrc;
		for (int x = 0; x < src->width; x++)
		{
			pixDst[0] = pixSrc[0];
			pixDst[1] = pixSrc[1];
			pixDst[2] = pixSrc[2];
			pixDst[3] = 255;
			pixDst += 4;
			pixSrc += 3;
		}
		rowSrc += src->stride;
		rowDst += dst->stride;
	}

	return dst;
}

PRAW_IMAGE_DATA ConvertTo32BppRawImage(PRAW_IMAGE_DATA src)
{
	PRAW_IMAGE_DATA		dst = NULL;
	switch (src->bitCount)
	{
	case 32:
		return src;
	case 24:
		dst = Convert24BppTo32BppRawImage(src);
		FreeRawImage(src);
		break;
	default:
		src = ConvertTo24BppRawImage(src);
		dst = Convert24BppTo32BppRawImage(src);
		FreeRawImage(src);
		break;
	}

	return dst;
}

PRAW_IMAGE_DATA LoadRawImageDataFromBmpFile(IILIO *io);

PRAW_IMAGE_DATA LoadRawImageDataFromPngFile(IILIO *io);

PRAW_IMAGE_DATA LoadRawImageDataFromJpgFile(IILIO *io);

PRAW_IMAGE_DATA LoadRawImageDataFromGifFile(IILIO *io);

PRAW_IMAGE_DATA LoadRawImageDataFromFile(LPCTSTR szFile)
{
#ifdef _IPHONE
	return NULL;
#else
	LPCTSTR			szExt;
	int				n;
	CFileILIO		io;
	PRAW_IMAGE_DATA	image = NULL;

	n = _tcslen(szFile);
	if (n < 4)
		return NULL;

	if (!io.Open(szFile))
		return NULL;

	szExt = szFile + n - 4;
	if (_tcsicmp(szExt, _T(".bmp")) == 0)
		image = LoadRawImageDataFromBmpFile(&io);
	else if (_tcsicmp(szExt, _T(".png")) == 0)
		image = LoadRawImageDataFromPngFile(&io);
	else if (_tcsicmp(szExt, _T(".jpg")) == 0
		|| _tcsicmp(szExt, _T("jpeg")) == 0)
		image = LoadRawImageDataFromJpgFile(&io);
	else if (_tcsicmp(szExt, _T(".gif")) == 0)
		image = LoadRawImageDataFromGifFile(&io);
	else
		image = NULL;

	if (image)
	{
		if (image->bitCount != 24 && image->bitCount != 32)
			return ConvertTo24BppRawImage(image);
	}

	return image;
#endif // _IPHONE
}


PRAW_IMAGE_DATA LoadRawImageDataFromMem(const void *buf, int nSize)
{
#ifdef _IPHONE
	return NULL;
#else
	CBuffILIO		io;
	uint8			*tag = (uint8 *)buf;
	uint8			png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
	PRAW_IMAGE_DATA	image = NULL;

	if (!io.Open(buf, nSize))
		return NULL;

	if (memcmp(buf, "BM", 2) == 0)
		image = LoadRawImageDataFromBmpFile(&io);
	else if (memcmp(buf, png_signature, 8) == 0)
		image = LoadRawImageDataFromPngFile(&io);
	else if (tag[0] == 0xFF && tag[1] == 0xD8)
		return LoadRawImageDataFromJpgFile(&io);
	else if (memcmp(buf, "GIF8", 4) == 0)
		image = LoadRawImageDataFromGifFile(&io);
	else
		image = NULL;

	if (image)
	{
		if (image->bitCount != 24 && image->bitCount != 32)
			return ConvertTo24BppRawImage(image);
	}

	return image;
#endif // _IPHONE
}

void FreeRawImage(PRAW_IMAGE_DATA pRawImage)
{
	pRawImage->Free();
	delete pRawImage;
}


void RawImageBGR24Set(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b)
{
	uint8		*pRow, *p;

	pRow = pRawImage->RowPtr(0);

	for (int y = 0; y < pRawImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pRawImage->width; x++)
		{
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += 3;
		}
		pRow += pRawImage->stride;
	}
}

void RawImageBGRA32Set(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b, uint8 a)
{
	ASSERT(pRawImage->pixFormat == PF_RGBA32);

	uint8		*pRow, *p;

	pRow = pRawImage->RowPtr(0);

	for (int y = 0; y < pRawImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pRawImage->width; x++)
		{
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p[3] = a;
			p += 4;
		}
		pRow += pRawImage->stride;
	}
}

void RawImageSet(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b, uint8 a)
{
	if (pRawImage->pixFormat == PF_RGB24)
		RawImageBGR24Set(pRawImage, r, g, b);
	else
		RawImageBGRA32Set(pRawImage, r, g, b, a);
}

//////////////////////////////////////////////////////////////////////////

inline unsigned char ToRGBx(float rm1, float rm2, float rh)
{
	if		(rh > 360.0f)
	{
		rh -= 360.0f;
		if (rh > 360.0f)
			rh -= 360.0f;
	}
	else if (rh <   0.0f) rh += 360.0f;
	
	if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
	else if (rh < 180.0f) rm1 = rm2;
	else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

	return static_cast<unsigned char>(rm1 * 255);
}

inline void AdjustPixHue(byte *pix, float hueOffset)
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

    if (saturation == 0.0)
    {
		pix[0] = pix[1] = pix[2] = (byte)(luminance * 255.0);
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

	if (luminanceRatio > 0)
		luminance += (float)((1.0 - luminance) * luminanceRatio);
	else
		luminance *= -luminanceRatio;
	ASSERT(luminance >= 0 && luminance <= 1.0);

    if (saturation == 0.0)
    {
		pix[0] = pix[1] = pix[2] = (byte)(luminance * 255.0);
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

void AdjustColorHue(COLORREF &clr, float hueOffset)
{
	byte	*p = (byte *)&clr;
	byte	pix[3];
	pix[0] = p[2];
	pix[1] = p[1];
	pix[2] = p[0];
	AdjustPixHue(pix, hueOffset);
	p[0] = pix[2];
	p[1] = pix[1];
	p[2] = pix[0];
}

void AdjustColorHue(COLORREF &clr, float hueOffset, float saturationRatio, float luminanceRatio)
{
	byte	*p = (byte *)&clr;
	byte	pix[3];
	pix[0] = p[2];
	pix[1] = p[1];
	pix[2] = p[0];
	AdjustPixHue(pix, hueOffset, saturationRatio, luminanceRatio);
	p[0] = pix[2];
	p[1] = pix[1];
	p[2] = pix[0];
}

// hueOffset: 0-360
void AdjustImageHue32(PRAW_IMAGE_DATA pImage, float hueOffset)
{
	byte		*p, *pRow;

	pRow = pImage->RowPtr(0);
	for (int y = 0; y < pImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pImage->width; x++)
		{
			AdjustPixHue(p, hueOffset);
			p += 4;
		}
		pRow += pImage->stride;
	}
}

// hueOffset: 0-360
void AdjustImageHue24(PRAW_IMAGE_DATA pImage, float hueOffset)
{
	byte		*p, *pRow;

	pRow = pImage->RowPtr(0);
	for (int y = 0; y < pImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pImage->width; x++)
		{
			AdjustPixHue(p, hueOffset);
			p += 3;
		}
		pRow += pImage->stride;
	}
}

void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset)
{
	if (pImage->bitCount == 32)
		AdjustImageHue32(pImage, hueOffset);
	else if (pImage->bitCount == 24)
		AdjustImageHue24(pImage, hueOffset);
}

// hueOffset: 0-360
void AdjustImageHue32(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio)
{
	byte		*p, *pRow;

	pRow = pImage->RowPtr(0);
	for (int y = 0; y < pImage->height; y++)
	{
		p = pRow;
		for (int x = 0; x < pImage->width; x++)
		{
			AdjustPixHue(p, hueOffset, saturationRatio, luminanceRatio);
			p += 4;
		}
		pRow += pImage->stride;
	}
}

// hueOffset: 0-360
void AdjustImageHue24(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio)
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
void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio)
{
	if (pImage->bitCount == 32)
		AdjustImageHue32(pImage, hueOffset, saturationRatio, luminanceRatio);
	else if (pImage->bitCount == 24)
		AdjustImageHue24(pImage, hueOffset, saturationRatio, luminanceRatio);
}

COLORREF HLSToRGB(float hue, float saturation, float luminance)
{
	COLORREF		clr;

    if (saturation == 0.0)
    {
		unsigned char	c = (byte)(luminance * 255.0);
		clr = RGB(c, c, c);
    }
    else
    {
		float rm1, rm2;

		if (luminance <= 0.5f)
			rm2 = luminance + luminance * saturation;  
		else
			rm2 = luminance + saturation - luminance * saturation;

		rm1 = 2.0f * luminance - rm2;
		clr = RGB(ToRGBx(rm1, rm2, hue + 120.0f),
				ToRGBx(rm1, rm2, hue),
				ToRGBx(rm1, rm2, hue - 120.0f));
    }

	return clr;
}

void RGBToHLS(COLORREF clr, float &hue, float &saturation, float &luminance)
{
    // Konvertierung
    unsigned char minval = min(GetRValue(clr), min(GetGValue(clr), GetBValue(clr)));
    unsigned char maxval = max(GetRValue(clr), max(GetGValue(clr), GetBValue(clr)));
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
		float rnorm = (maxval - GetRValue(clr)  ) / mdiff;      
		float gnorm = (maxval - GetGValue(clr)  ) / mdiff;
		float bnorm = (maxval - GetBValue(clr)  ) / mdiff;   

		if ((luminance <= 0.5f))
			saturation = mdiff / msum;
		else
		{
			saturation = (float)(mdiff / (2.0 - msum));
			if (saturation < 0)
				saturation = -saturation;
		}

		if (GetRValue(clr)   == maxval) hue = 60.0f * (6.0f + bnorm - gnorm);
		else if (GetGValue(clr) == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
		else if (GetBValue(clr)  == maxval) hue = 60.0f * (4.0f + gnorm - rnorm);
		if (hue > 360.0f) hue = hue - 360.0f;
    }
}
/*
void  rgb_to_hsl(
    float    r,
    float    g,
    float    b,
    float    *h,
    float    *s,
    float    *l )
{
    float  v, m, vm, r2, g2, b2;

    v = max( r, max(g, b) );

    m = min( r, min(g, b ));

    *l = (m + v) / 2.0;

    if( *l > 0.0 )
    {
        vm = v - m;
        *s = vm;

        if( *s > 0.0 )
        {
            if( *l <= 0.5 )
                *s /= v + m;
            else
                *s /= 2.0 - v - m;

            r2 = (v - r) / vm;
            g2 = (v - g) / vm;
            b2 = (v - b) / vm;

            if( r == v )
            {
                if( g == m )
                    *h = 5.0 + b2;
                else
                    *h = 1.0 - g2;
            }
            else if( g == v )
            {
                if( b == m )
                    *h = 1.0 + r2;
                else
                    *h = 3.0 - b2;
            }
            else
            {
                if( r == m )
                    *h = 3.0 + g2;
                else
                    *h = 5.0 - r2;
            }

            *h /= 6.0;
        }
    }
}
void  hsl_to_rgb(
    float   h,
    float   sl,
    float   l,
    float   *r,
    float   *g,
    float   *b )
{
    float  v;
    float  m, sv;
    int   sextant;
    float  fract, vsf, mid1, mid2;

    if( l <= 0.5 )
    {
        v = l * (1.0 + sl);
    }
    else
    {
        v = l + sl - l * sl;
    }

    if( v <= 0.0 )
    {
        *r = 0.0;
        *g = 0.0;
        *b = 0.0;
    }
    else
    {
        m = l + l - v;
        sv = (v - m) / v;
        h *= 6.0;
        sextant = (int) h;
        fract = h - (float) sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;

        switch( sextant )
        {
        case 0:
        case 6:  *r = v;     *g = mid1;  *b = m;     break;
        case 1:  *r = mid2;  *g = v;     *b = m;     break;
        case 2:  *r = m;     *g = v;     *b = mid1;  break;
        case 3:  *r = m;     *g = mid2;  *b = v;     break;
        case 4:  *r = mid1;  *g = m;     *b = v;     break;
        case 5:  *r = v;     *g = m;     *b = mid2;  break;
        }
    }
}


void HLSTest()
{
	COLORREF		clr, clrNew;
	int		r, g, b;
	float	H, S, L;
	float	H2, S2, L2;
	WORD	hue, saturation, luminance;
	float	R, G, B;
// 
// 	clrNew = RGB(255, 254, 253);
// 	rgb_to_hsl(255, 254, 253, &H, &S, &L);
// 	hsl_to_rgb(H, S, L, &R, &G, &B);
// 	RGBToHLS(clrNew, H, S, L);
// 	AdjustColorHue(clrNew, 0, -1.0, -0.8);
// 	RGBToHLS(clrNew, H2, S2, L2);
// 	clr = HLSToRGB(H2, S2, L2);
// 
// 	clr = RGB(255, 254, 253);
// 	clrNew = clr;
// 
// 	for (float l = -1.0; l < 0; l += 0.01)
// 	{
// 		clrNew = clr;
// 		AdjustColorHue(clrNew, 0, -1.0, l);
// 	}


//	return;

	{
		for (r = 0; r <= 255; r++)
		{
			for (g = 0; g <= 255; g++)
			{
				for (b = 0; b <= 255; b++)
				{
// 					rgb_to_hsl(r, g, b, &H, &S, &L);
// 					hsl_to_rgb(H, S, L, &R, &G, &B);
// 					if (abs(r - R) > 2
// 						|| abs(g - G) > 2
// 						|| abs(b - B) > 2)
// 					{
// 						DBGLOG0(_T("Not equal"));
// 					}


					clr = RGB(r, g, b);
					clrNew = clr;
					AdjustColorHue(clrNew, 0);//, -1.0, -1.0);

					//RGBToHLS(clr, H, S, L);
					//clrNew = HLSToRGB(H, S, L);
					if (abs(GetRValue(clr) - GetRValue(clrNew)) > 1
						|| abs(GetGValue(clr) - GetGValue(clrNew)) > 1
						|| abs(GetBValue(clr) - GetBValue(clrNew)) > 1)
					{
						DBGLOG0(_T("Not equal"));
					}
				}
			}
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

RAW_IMAGE_DATA::RAW_IMAGE_DATA()
{
	buff = NULL;
	width = height = 0;
	stride = 0;
	bitCount = 0;
	pixFormat = PF_UNKNOWN;

	nClrUsed = 0;
	pallete = NULL;
}

void RAW_IMAGE_DATA::Attach(uint8 *buf, int nWidth, int nHeight, int nBitCount)
{
	buff = buf;
	width = nWidth;
	height = nHeight;
	bitCount = nBitCount;
	if (nBitCount == 32)
		pixFormat = PF_RGBA32;
	else if (nBitCount == 24)
		pixFormat = PF_RGB24;
	else
		pixFormat = PF_UNKNOWN;

	stride = -BMPWIDTHBYTES(width, nBitCount);
}

void RAW_IMAGE_DATA::Attach(RAW_IMAGE_DATA *pSrc)
{
	buff = pSrc->buff;
	width = pSrc->width;
	height = pSrc->height;
	stride = pSrc->stride;
	bitCount = pSrc->bitCount;
	pixFormat = pSrc->pixFormat;
	nClrUsed = pSrc->nClrUsed;
	pallete = pSrc->pallete;
}

void RAW_IMAGE_DATA::Switch(RAW_IMAGE_DATA *pSrc)
{
	RAW_IMAGE_DATA		temp;
	temp.Attach(this);
	Attach(pSrc);
	pSrc->Attach(&temp);
	temp.Detach();
}

void RAW_IMAGE_DATA::Detach()
{
	buff = NULL;
	pallete = NULL;
	nClrUsed = 0;
}

bool RAW_IMAGE_DATA::Create(int nWidth, int nHeight, int nBitCount)
{
	width = nWidth;
	height = nHeight;
	bitCount = nBitCount;
	if (nBitCount == 32)
		pixFormat = PF_RGBA32;
	else if (nBitCount == 24)
		pixFormat = PF_RGB24;
	else
		pixFormat = PF_UNKNOWN;

	stride = -BMPWIDTHBYTES(width, nBitCount);

	buff = new uint8[abs(stride) * height];
	ASSERT(buff);

	if (nBitCount <= 8)
		pallete = new RGBQUAD[256];

	return true;
}

bool RAW_IMAGE_DATA::CreateReverse(int nWidth, int nHeight, int nBitCount)
{
	if (!Create(nWidth, nHeight, nBitCount))
		return false;

	stride = -stride;

	return true;
}

void RAW_IMAGE_DATA::SetClrUsed(int _nClrUsed)
{
	nClrUsed = (uint8)_nClrUsed;

	if (_nClrUsed == 0)
	{
		if (pallete)
		{
			delete[] pallete;
			pallete = NULL;
		}
		return;
	}
	if (!pallete)
		pallete = new RGBQUAD[256];
}

RGBQUAD RAW_IMAGE_DATA::GetPixel(int x, int y)
{
	uint8		*p;
	RGBQUAD		quad;

	p = RowPtr(y);
	switch (pixFormat)
	{
	case PF_RGBA32:
		{
			p += x * 4;
			quad.rgbBlue = p[0];
			quad.rgbGreen = p[1];
			quad.rgbRed = p[2];
			quad.rgbReserved = p[3];
		}
		break;
	case PF_RGB24:
		{
			p += x * 3;
			quad.rgbBlue = p[0];
			quad.rgbGreen = p[1];
			quad.rgbRed = p[2];
			quad.rgbReserved = 255;
		}
		break;
	case PF_PALLETE256:
		{
			p += x;
			if (pallete)
				quad = pallete[*p];
			else
				memset(&quad, 0, sizeof(quad));
		}
		break;
	case PF_PALLETE16:
	case PF_PALLETE2:
		{
			memset(&quad, 0, sizeof(quad));
			ASSERT(0 && "unsupported format.");
		}
		break;
	}
	return quad;
}

void RAW_IMAGE_DATA::Free()
{
	if (buff)
	{
		delete[] buff;
	}
	if (pallete)
	{
		delete[] pallete;
		pallete = NULL;
	}
}
