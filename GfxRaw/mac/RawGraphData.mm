#include "stdafx.h"
#include "GfxRawMac.h"
#include "GfxRaw.h"
#include "RawGraphData.h"

CRawGraphData::CRawGraphData()
{
	m_context = NULL;
}

CRawGraphData::~CRawGraphData()
{
	Destroy();
}

bool CRawGraphData::Create(int cx, int cy, CMLGraphics *pGraph, int nBitCount)
{
	ASSERT(nBitCount == 32);

	if (!m_imageData.Create(cx, cy, nBitCount))
		return false;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
	// create the bitmap context
	m_context = CGBitmapContextCreate(m_imageData.buff, m_imageData.width, m_imageData.height, 8,
														m_imageData.AbsStride(), colorSpace,
														// this will give us an optimal BGRA format for the device:
														(kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst));
	CGColorSpaceRelease(colorSpace);

	return m_context != NULL;
}

void CRawGraphData::Destroy()
{
	if (m_context != NULL)
		CGContextRelease(m_context);

	m_imageData.Free();
}

PRAW_IMAGE_DATA RawImageDataFromCGImage(CGImageRef cgimage)
{
	ASSERT(cgimage);
	if (!cgimage)
		return NULL;
	
	int width = (int)CGImageGetWidth(cgimage);
	int height = (int)CGImageGetHeight(cgimage);

	CRawGraphData	rawGraph;
	CGRect rcDest = CGRectMake(0, 0, width, height);
	rawGraph.Create(width, height, NULL, 32);
	rawGraph.GetRawBuff()->ReverseUpDown();

	CGContextDrawImage(rawGraph.GetHandle(), rcDest, cgimage);

	PRAW_IMAGE_DATA pRawImage = new RAW_IMAGE_DATA;
	pRawImage->Switch(rawGraph.GetRawBuff());

	return pRawImage;
}

void CRawGraphData::DrawToGraphics(CMLGraphics *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	CGImageRef rawImage = CGBitmapContextCreateImage(m_context);
	CMLGraphics		*pGraph = (CMLGraphics *)pTarg;
	CGRect rcDest = CGRectMake(xdest, ydest, width, height);
	
	if (xsrc + width >= m_imageData.width)
		width = m_imageData.width - xsrc;
	if (ysrc + height >= m_imageData.height)
		height = m_imageData.height - ysrc;

	if (xsrc == 0 && ysrc == 0 && m_imageData.width == width && m_imageData.height == height)
	{
		CGContextDrawImage(pGraph->GetHandle(), rcDest, rawImage);
		CGImageRelease(rawImage);
		return;
	}

	CGRect rcSrc = CGRectMake(xsrc, m_imageData.height - ysrc - height, width, height);
	CGImageRef partImage = CGImageCreateWithImageInRect(rawImage, rcSrc);

	CGContextDrawImage(pGraph->GetHandle(), rcDest, partImage);
	CGImageRelease(partImage);
	CGImageRelease(rawImage);
}

void CRawGraphData::DrawToGraphicsStretch(CMLGraphics *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
}
