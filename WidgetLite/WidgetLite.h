#ifndef _ML_WIDGET_H_
#define _ML_WIDGET_H_

#include "widgetif.h"

#ifdef WIN32
#include "win32/desktop.h"
#include "win32/BaseWnd.h"
#include "win32/FolderDialog.h"
#include "win32/FileSaveDlg.h"
#include "win32/FileOpenDlg.h"
#include "win32/MLMenu.h"
#include "win32/WndResizer.h"
#ifdef _WIN32_DESKTOP
#include "win32/DlgChooseColor.h"
#else
#include "../MiniLyrics6/win32/wce/DlgChooseColor.h"
#endif

#ifdef _WIN32_WCE
#include "win32/wince/SkinToolTip.h"
#else
#include "win32/SkinToolTip.h"
#endif
#endif

#ifdef _LINUX_GTK2

#endif


#ifdef _MAC_OSX

#endif

#include "WndDrag.h"

#endif // _ML_WIDGET_H_
