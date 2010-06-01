// RawImageData.h: interface for the RawImageData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_)
#define AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_

#pragma once

#ifndef _WIN32
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD * LPRGBQUAD;
#endif

enum PIX_FORMAT
{
	PF_UNKNOWN		= -1,
	PF_RGBA32, //		= agg::pix_format_rgba32,
	PF_RGB24, //		= agg::pix_format_rgb24,
	PF_PALLETE256,
	PF_PALLETE16,
	PF_PALLETE2,
};

typedef uint32		BlendPixMode;

enum _BlendPixMode
{
	// Target Channel
	BPM_CHANNEL_RGB		= 1,
	BPM_CHANNEL_ALPHA	= 1 << 1,

	// Operation
	BPM_OP_COPY			= 1 << 8,
	BPM_OP_BLEND		= 1 << 9,
	BPM_OP_XOR			= 1 << 10,
	BPM_OP_MULTIPLY		= 1 << 11,

	//
	// Combined Channel and operaton:
	//

	BPM_COPY			= BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_COPY,
	BPM_BLEND			= BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_BLEND,
	BPM_MULTIPLY		= BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_MULTIPLY,

	BPM_BLEND_RGB		= BPM_CHANNEL_RGB | BPM_OP_BLEND,
	BPM_BLEND_ALPHA		= BPM_CHANNEL_ALPHA | BPM_OP_BLEND,
	BPM_COPY_ALPHA		= BPM_CHANNEL_ALPHA | BPM_OP_BLEND,
	BPM_COPY_RGB		= BPM_CHANNEL_RGB | BPM_OP_BLEND,

	BPM_XOR_RGB			= BPM_CHANNEL_RGB | BPM_OP_XOR,

	BPM_BILINEAR		= 1 << 17,		// available option for stretch blt
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

void RawImageBGR24Set(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b);

void RawImageBGRA32Set(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b, uint8 a);

void RawImageSet(PRAW_IMAGE_DATA pRawImage, uint8 r, uint8 g, uint8 b, uint8 a);

void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset);

void AdjustImageHue(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio);

void AdjustColorHue(COLORREF &clr, float hueOffset);

void AdjustColorHue(COLORREF &clr, float hueOffset, float saturationRatio, float luminanceRatio);

COLORREF HLSToRGB(float hue, float saturation, float luminance);

void RGBToHLS(COLORREF clr, float &hue, float &saturation, float &luminance);

#endif // !defined(AFX_RAWIMAGEDATA_H__99C17B14_06C0_4675_8030_19018283D265__INCLUDED_)
