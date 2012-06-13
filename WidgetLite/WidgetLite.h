#ifndef _ML_WIDGET_H_
#define _ML_WIDGET_H_

#include "../GfxLite/GfxLite.h"
#include "widgetif.h"

#ifdef WIN32
#include "win32/BaseWnd.h"
#include "win32/FolderDialog.h"
#include "win32/FileSaveDlg.h"
#include "win32/FileOpenDlg.h"
#include "win32/MLMenu.h"
#include "win32/WndResizer.h"
#include "win32/DlgChooseColor.h"
#include "win32/SkinToolTip.h"
#endif

#ifdef _LINUX_GTK2

#endif


#ifdef _MAC_OSX
#include "Mac/Cursor.h"
#include "Mac/BaseWnd.h"
#include "Mac/FolderDialog.h"
#include "Mac/FileSaveDlg.h"
#include "Mac/FileOpenDlg.h"
#include "Mac/MLMenu.h"
#include "Mac/WndResizer.h"
#include "Mac/DlgChooseColor.h"
#include "Mac/SkinToolTip.h"
#endif

#include "WndDrag.h"
#include "desktop.h"

#endif // _ML_WIDGET_H_
