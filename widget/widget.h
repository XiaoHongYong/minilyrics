#pragma once

#include "../WidgetLite/WidgetLite.h"
#include "WidgetNotify.h"

#ifdef WIN32
#include "win32/BaseDialog.h"
#include "win32/WidgetCtrlBase.h"
#include "win32/WidgetListBox.h"
#include "win32/WidgetListCtrl.h"
#include "win32/WidgetTreeCtrl.h"
#include "win32/CommonWidget.h"
#include "win32/WidgetEditBox.h"
#include "win32/HyperLinkCtrl.h"
#include "win32/WidgetComboBox.h"
#include "win32/MLWndHelper.h"
#include "win32/WidgetTabCtrl.h"
#include "win32/WidgetTrackBar.h"
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
#include "gtk2/DlgChooseColor.h"
// #include "gtk2/EditEx.h"
#endif


#ifdef _MAC_OSX
#include "widgetif.h"
#include "Mac/BaseWnd.h"
/*#include "Mac/desktop.h"
#include "Mac/SkinToolTip.h"
#include "Mac/MLMenu.h"
#include "Mac/WndResizer.h"
#include "Mac/WidgetCtrlBase.h"
#include "Mac/HyperLinkCtrl.h"
#include "Mac/CommonWidget.h"
#include "Mac/WidgetTreeCtrl.h"
#include "Mac/WidgetListBox.h"*/
#endif

