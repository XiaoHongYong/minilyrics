
#pragma once

#include <CoreGraphics/CGImage.h>

typedef CGImageRef	IMAGE_HANDLE;

IMAGE_HANDLE LoadBitmapFromFile(LPCTSTR szFile, IMLGraphicsBase *pGraph = NULL);

IMAGE_HANDLE LoadBitmapFromMem(const char *data, int nLen, IMLGraphicsBase *pGraph = NULL);

void ReleaseImageHandle(IMAGE_HANDLE hImage);
