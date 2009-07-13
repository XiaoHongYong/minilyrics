// BmpFile.cpp: implementation of the CBmpFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ILIO.h"
#include "RawImageData.h"

#ifndef _WIN32

#pragma pack(push)
#pragma pack(1)
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
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack(pop)

#endif

PRAW_IMAGE_DATA LoadRawImageDataFromBmpFile(IILIO *io)
{
	PRAW_IMAGE_DATA		pImage = NULL;
	BITMAPFILEHEADER	bfh;
	BITMAPINFOHEADER	bih;
	DWORD				dwDataSize;
	uint32				nFileLen;
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

/*
BOOL CBmpFile::Save(LPCTSTR szFile)
{
	BITMAPFILEHEADER	bmfh;
	FILE				*fp;

	fp = _tfopen(szFile, _T("wb"));
	if (!fp)
	{
		ERRLOG1(_T("Can't Create file: %s"), szFile);
		return FALSE;
	}

	bmfh.bfSize = sizeof(bmfh);
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfType = MAKEWORD((BYTE)'B',(BYTE)'M');

	// Write BITMAPFILEHEADER
	if (fwrite(&bmfh, 1, sizeof(bmfh), fp) != sizeof(bmfh))
		goto FAILED;

	// Wirte BITMAPINFO
	if (fwrite(m_lpImgData->GetBitMapInfo(), 1, m_lpImgData->GetBitMapInfoSize(), fp) != m_lpImgData->GetBitMapInfoSize())
		goto FAILED;

	// Write Data
	if (fwrite(m_lpImgData->GetImgData(), 1, m_lpImgData->GetImgDataSize(), fp) != m_lpImgData->GetImgDataSize())
		goto FAILED;

	fclose(fp);

	return TRUE;
FAILED:
	fclose(fp);

	return FALSE;
}
*/