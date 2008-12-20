#include "StdAfx.h"
#include "ImageFile.h"
#include <olectl.h>

#ifdef _WIN32_WCE

HBITMAP LoadBitmapFromFile(LPCTSTR szFile)
{
	return SHLoadDIBitmap(szFile);
}

#else // _WIN32_WCE

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


HBITMAP LoadBitmapFromMem(const char *data, int nLen)
{
	LPVOID		pvData = NULL;
	HGLOBAL		hGlobal = NULL;
	LPSTREAM	pstm = NULL;
	LPPICTURE	pPicture = NULL;
	HRESULT		hr;
	HBITMAP		hBitmap;

	// alloc memory based on file size
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, nLen);
	if (!hGlobal)
		goto FAILED;

	pvData = GlobalLock(hGlobal);
	if (!pvData)
		goto FAILED;

	// read file and store in global memory
	memcpy(pvData, data, nLen);

	// create IStream* from global memory
	hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
	if (!SUCCEEDED(hr) || pstm == NULL)
		goto FAILED;

	// Create IPicture from image file
	hr = ::OleLoadPicture(pstm, nLen, FALSE, IID_IPicture, (LPVOID *)&pPicture);
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

	return hBitmap;

FAILED:
	if (hGlobal)
	{
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
	}

	if (pstm)
		pstm->Release();
	if (pPicture)
		pPicture->Release();

	return NULL;
}

#endif // _WIN32_WCE

void ReleaseImageHandle(IMAGE_HANDLE hImage)
{
	DeleteObject(hImage);
}
