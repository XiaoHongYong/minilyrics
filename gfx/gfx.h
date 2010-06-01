#ifndef _ML_GFX_H_
#define _ML_GFX_H_

class IMLGraphicsBase;
class MLColor;
class CMLImage;
class CMLBrush;
class CMLPen;
class COurRgn;
class CRegion;


#include "IMLGraphicsBase.h"
#include "FontInfo.h"

#ifdef _LINUX_GTK2

#include "gtk2/Cursor.h"
#include "gtk2/MLColor.h"
#include "gtk2/MLBrush.h"
#include "gtk2/MLFont.h"
#include "gtk2/MLPen.h"
#include "gtk2/MLImage.h"
#include "gtk2/Region.h"
#include "gtk2/MLGraphics.h"

#endif

#ifdef _MAC

#include "mac/Cursor.h"
#include "mac/MLColor.h"
#include "mac/MLBrush.h"
#include "mac/MLFont.h"
#include "mac/MLPen.h"
#include "mac/MLImage.h"
#include "mac/Region.h"
#include "mac/MLGraphics.h"

#endif

#ifdef _WIN32
#include "MLHandlerMap.h"

#include "win32/MemDC.h"
#include "win32/Cursor.h"
#include "win32/MLColor.h"
#include "win32/MLBrush.h"
#include "win32/MLFont.h"
#include "win32/MLPen.h"
#include "win32/MLImage.h"
#include "win32/Region.h"
#include "win32/GdiGraphics.h"
#ifndef _WINCE_
#include "win32/GdiplusGraphics.h"
#endif
#include "MLGraphicsFactory.h"
#include "win32/MemGdiGraphics.h"
#endif

#endif
