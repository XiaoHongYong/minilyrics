// RawImageData.h: interface for the RawImageData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_)
#define AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_

#pragma once

#include "platform/agg_platform_support.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_rgb.h"
#include "agg_rasterizer_scanline_aa.h"

enum PIX_FORMAT
{
	PF_UNKNOWN		= -1,
	PF_RGBA32, //		= agg::pix_format_rgba32,
	PF_RGB24, //		= agg::pix_format_rgb24,
	PF_PALLETE256,
	PF_PALLETE16,
	PF_PALLETE2,
};

class RAW_IMAGE_DATA
{
public:
	RAW_IMAGE_DATA();

	void Attach(uint8 *buf, int nWidth, int nHeight, int nBitCount);
	void Attach(RAW_IMAGE_DATA *pSrc);
	void Switch(RAW_IMAGE_DATA *pSrc);
	void Detach();

	bool Create(int nWidth, int nHeight, int nBitCount);
	bool CreateReverse(int nWidth, int nHeight, int nBitCount);

	void Free();

	void SetClrUsed(int _nClrUsed);

	uint8 *RowPtr(int y)
	{
		if (stride > 0)
			return buff + stride * y;
		else
			return buff - stride * (height - 1 - y);
	}

	uint8 *PixPtr(int x, int y)
	{
		uint8		*p;

		p = RowPtr(y);
		switch (pixFormat)
		{
		case PF_RGBA32:
			p += x * 4;
			break;
		case PF_RGB24:
			p += x * 3;
			break;
		case PF_PALLETE256:
			p += x;
			break;
		case PF_PALLETE16:
		case PF_PALLETE2:
			p += x * bitCount / 8;
			break;
		}

		return p;
	}

	RGBQUAD GetPixel(int x, int y);

	size_t GetBuffSize() { return (stride > 0 ? stride : -stride) * height; }

	int AbsStride() { return stride > 0 ? stride : -stride; }

public:
	uint8			*buff;
	uint16			width, height;
	int16			stride;
	int16			bitCount;
	PIX_FORMAT		pixFormat;

	uint8			nClrUsed;
	RGBQUAD			*pallete;

};

// class RAW_IMAGE_DATA_PALLETE : public RAW_IMAGE_DATA
// {
// public:
// 	int				nClrUsed;
// 	RGBQUAD			pallete[256];		// max pallete is 256
// 
// };
// typedef RAW_IMAGE_DATA_PALLETE * PRAW_IMAGE_DATA_PALLETE;

typedef RAW_IMAGE_DATA * PRAW_IMAGE_DATA;

PRAW_IMAGE_DATA LoadRawImageDataFromFile(LPCTSTR szFile);

PRAW_IMAGE_DATA LoadRawImageDataFromMem(const void *buf, int nSize);

PRAW_IMAGE_DATA ConvertTo24BppRawImage(PRAW_IMAGE_DATA src);

void FreeRawImage(PRAW_IMAGE_DATA pRawImage);

void RIDBgr24Set(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b);

void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset);

void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio);

void AdjustColorHue(COLORREF &clr, float hueOffset);

void AdjustColorHue(COLORREF &clr, float hueOffset, float saturationRatio, float luminanceRatio);

COLORREF HLSToRGB(float hue, float saturation, float luminance);

void RGBToHLS(COLORREF clr, float &hue, float &saturation, float &luminance);

#endif // !defined(AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_)
