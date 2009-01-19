
#ifndef WALLPAPER_UTIL_H
#define WALLPAPER_UTIL_H
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <string>

//////////////////////////////////////////////////////////////////////////
// check local resource
#define DT_HARDDISK		1
#define DT_USBSTORAGE	2
#define DT_CD			4
#define DT_FLOPPYDISK	8
#define DT_REMOTE		16

unsigned int GetNumberOfDisk(unsigned int type);
CXStr GetLetterOfDisk(unsigned int type, unsigned int index);

void TravelDir(TCHAR* dir);

//////////////////////////////////////////////////////////////////////////
// check internet resource
IMedia* CheckSignalUrl(TCHAR* url);



#endif // WALLPAPER_UTIL_H