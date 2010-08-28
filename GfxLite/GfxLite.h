
#pragma once

class CMLGraphics;


#ifndef _WIN32
// the dwFlags of DrawTextEx
#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_SINGLELINE       0x00000020
#define DT_NOCLIP           0x00000100
#define DT_END_ELLIPSIS     0x00008000
#else
#ifdef _WINCE_
#define DT_PATH_ELLIPSIS    0x00004000
#define DT_END_ELLIPSIS     0x00008000
#define DT_RTLREADING       0x00020000
#define DT_WORD_ELLIPSIS    0x00040000
#endif
#endif

#define DT_PREFIX_TEXT      0x02000000


#include "FontInfo.h"

#ifdef _WIN32
#include "MLHandlerMap.h"
#include "Win32/MLGraphics.h"
#include "Win32/MLColor.h"
#include "win32/Region.h"
#endif

#ifdef _LINUX_GTK2

#include "gtk2/MLGraphics.h"

#endif

#ifdef _MAC

#include "mac/MLGraphics.h"

#endif
