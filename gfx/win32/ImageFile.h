
#pragma once

typedef HBITMAP		IMAGE_HANDLE;

HBITMAP LoadBitmapFromFile(LPCTSTR szFile);

HBITMAP LoadBitmapFromMem(const char *data, int nLen);

void ReleaseImageHandle(IMAGE_HANDLE hImage);
