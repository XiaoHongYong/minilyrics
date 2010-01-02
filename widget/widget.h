#ifndef _ML_WIDGET_H_
#define _ML_WIDGET_H_

#include "widgetif.h"

#ifdef WIN32
#include "win32/WidgetLocalization.h"
#include "win32/desktop.h"
#include "win32/BaseWnd.h"
#include "win32/BaseDialog.h"
#include "win32/WidgetListBox.h"
#include "win32/WidgetListCtrl.h"
#include "win32/WidgetTreeCtrl.h"
#include "win32/CommonWidget.h"
#include "win32/WidgetEditBox.h"
#include "win32/WidgetColorButton.h"
#include "win32/HyperLinkCtrl.h"
#include "win32/WidgetComboBox.h"
#include "win32/FolderDialog.h"
#include "win32/FileSaveDlg.h"
#include "win32/FileOpenDlg.h"
#include "win32/htmlhelp.h"
#include "win32/MLMenu.h"
#include "win32/WndResizer.h"
#include "win32/MLWndHelper.h"
#include "win32/WidgetImageButton.h"
#include "win32/WidgetTabCtrl.h"
#include "win32/WidgetTrackBar.h"
// #include "win32/EditEx.h"

#ifdef _WIN32_WCE
#include "win32/wince/SkinToolTip.h"
#else
#include "win32/HtmlWnd.h"
#include "win32/ToolTipWnd.h"
#include "win32/SkinToolTip.h"
#endif
#endif

#ifdef _LINUX_GTK2
#include "gtk2/WidgetLocalization.h"
#include "gtk2/BaseWnd.h"
#include "gtk2/BaseDialog.h"
#include "gtk2/desktop.h"
#include "gtk2/SkinToolTip.h"
#include "gtk2/MLMenu.h"
#include "gtk2/WndResizer.h"
#include "gtk2/WidgetCtrlBase.h"
#include "gtk2/HyperLinkCtrl.h"
#include "gtk2/CommonWidget.h"
#include "gtk2/WidgetTreeCtrl.h"
#include "gtk2/WidgetListBox.h"
#include "gtk2/WidgetListCtrl.h"
#include "gtk2/WidgetComboBox.h"
#include "gtk2/FileFolderDlg.h"
#include "gtk2/MLWndHelper.h"
#include "gtk2/WidgetImageButton.h"
#include "gtk2/WidgetTabCtrl.h"
#include "gtk2/WidgetTrackBar.h"
// #include "gtk2/EditEx.h"
#endif

#include "WndDrag.h"

#endif // _ML_WIDGET_H_
