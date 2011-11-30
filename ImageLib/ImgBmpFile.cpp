// BmpFile.cpp: implementation of the CBmpFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ILIO.h"
#include "RawImageData.h"

#ifndef _WIN32

#pragma pack(push)
typedef long LONG;

typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	int32      biWidth;
	int32      biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	int32      biXPelsPerMeter;
	int32      biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

#pragma pack(pop)

#endif

PRAW_IMAGE_DATA LoadRawImageDataFromBmpFile(IILIO *io)
{
	PRAW_IMAGE_DATA		pImage = NULL;
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	DWORD				dwDataSize;
	size_t				nFileLen;
	int					nClrUsed = 0;

	io->GetSize(nFileLen);

	if (nFileLen <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
		goto FAILED;

	if (io->Read(&bfh, sizeof(bfh)) != sizeof(bfh))
		goto FAILED;
	if (bfh.bfType != MAKEWORD((BYTE)'B',(BYTE)'M'))
		goto FAILED;

	if (io->Read(&bih, sizeof(bih)) != sizeof(bih))
		goto FAILED;

	switch (bih.biBitCount)
	{
	case 8:								/*Colors counts of RGBQUAD */
	case 4:
	case 2:
	case 1:
		nClrUsed = (bih.biClrUsed > 0 ? bih.biClrUsed : (2 << (bih.biBitCount - 1)));
		break;
	case 32:
	case 24:
	case 16:
	default:
		nClrUsed = 0;
		break;
	}

	// create raw image
	pImage = new RAW_IMAGE_DATA;
	ASSERT(pImage);
	if (!pImage->Create(bih.biWidth, bih.biHeight, bih.biBitCount))
		goto FAILED;

	if (nClrUsed > 0)
	{
		pImage->SetClrUsed(nClrUsed);
		if (io->Read(pImage->pallete, sizeof(RGBQUAD) * nClrUsed) != sizeof(RGBQUAD) * nClrUsed)
			goto FAILED;
	}

	dwDataSize = pImage->GetBuffSize();
	if (io->Read(pImage->buff, dwDataSize) != dwDataSize)
		goto FAILED;

	return pImage;
FAILED:
	if (pImage)
	{
		if (pImage->buff)
			pImage->Free();

		delete pImage;
	}

	return NULL;
}

int SaveBmpFileFromRawImageData(PRAW_IMAGE_DATA pImageData, LPCTSTR szFile)
{
	BITMAPFILEHEADER	bmfh;
	FILE				*fp;

	fp = _tfopen(szFile, _T("wb"));
	if (!fp)
	{
		ERRLOG1(_T("Can't create file: %s"), szFile);
		return ERR_OPEN_FILE;
	}

	bmfh.bfSize = sizeof(bmfh);
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfType = MAKEWORD((BYTE)'B',(BYTE)'M');

	// Write BITMAPFILEHEADER
	if (fwrite(&bmfh, 1, sizeof(bmfh), fp) != sizeof(bmfh))
		goto FAILED;

	BITMAPINFO	bmpInfo;
	memset(&bmpInfo, 0, sizeof(bmpInfo));
	bmpInfo.bmiHeader.biBitCount = pImageData->bitCount;
	bmpInfo.bmiHeader.biHeight = pImageData->height;
	bmpInfo.bmiHeader.biWidth = pImageData->width;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biSizeImage = pImageData->GetBuffSize();

	// Write BITMAPINFO
	if (fwrite(&bmpInfo, 1, bmpInfo.bmiHeader.biSize, fp) != bmpInfo.bmiHeader.biSize)
		goto FAILED;

	// Write Data
	if (fwrite(pImageData->buff, 1, pImageData->GetBuffSize(), fp) != pImageData->GetBuffSize())
		goto FAILED;

	fclose(fp);

	return ERR_OK;
FAILED:
	fclose(fp);

	return ERR_WRITE_FILE;
}
