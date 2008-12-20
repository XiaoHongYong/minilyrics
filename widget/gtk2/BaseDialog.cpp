// BaseDialog.cpp: implementation of the CBaseDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "BaseDialog.h"
#include "../widget.h"
#include "WinUserPart.h"

bool CWidgetWin32Layout::Create(CBaseWnd *pWndParent, CDlgResource *pdlgRes)
{
	m_pWndParent = pWndParent;
	m_dlgRes = pdlgRes;

	m_window = gtk_fixed_new();
	gtk_widget_show(m_window);

	InitCreateDlgItems();

	g_signal_connect(m_window, "size-allocate",
		G_CALLBACK(Cb_SizeAllocate), this);

	return m_window != NULL;
}

void CWidgetWin32Layout::OnSizeAllocate(GtkAllocation *allocation)
{
	if (m_dlgRes->empty())
		return;

	CDlgResource::iterator	it, itEnd;
	GtkAllocation	size;
	float			fRadio;
	GtkRequisition	requisition;

	GtkWidget	*temp = gtk_button_new_with_label("Test");

	gtk_widget_size_request(temp, &requisition);
	fRadio = (float)requisition.height / 10;
	if (fRadio < 1.0 || fRadio >= 5.0)
		fRadio = 1.0;
	fRadio = 2.0;

	gtk_widget_destroy(temp);

	CRect		rc;
	if (!IsFlagSet(m_dlgRes->m_nStyle, WS_CHILD))
	{
		m_pWndParent->GetWindowRect(&rc);
		m_pWndParent->MoveWindow(rc.left, rc.top, m_dlgRes->m_width * fRadio, m_dlgRes->m_height * fRadio, false);
	}

	printf("Radio: %d, %f\n", requisition.height, fRadio);

	itEnd = m_dlgRes->end();
	for (it = m_dlgRes->begin(); it != itEnd; ++it)
	{
		CDlgResource::CWidgetItem	&item = *it;

		if (item.pWnd)
		{
			int		x, y, width, height;
			x = (int)(item.x * fRadio + 0.5);
			y = (int)(item.y * fRadio + 0.5);
			width = (int)(item.width * fRadio + 0.5);
			height = (int)(item.height * fRadio + 0.5);
			ASSERT(width > 0 && height > 0);
			if (item.widgetClass == CDlgResource::WC_COMB_BOX || 
				item.widgetClass == CDlgResource::WC_BUTTON || 
				item.widgetClass == CDlgResource::WC_COLOR_BUTTON || 
				item.widgetClass == CDlgResource::WC_RADIO_BUTTON || 
				item.widgetClass == CDlgResource::WC_CHECK_BOX || 
				item.widgetClass == CDlgResource::WC_EDIT)
			{
				gtk_widget_size_request(item.pWnd->GetHandle(), &requisition);
				height = requisition.height;
				// width = requisition.width;
			}
			// gtk_fixed_move(GTK_FIXED(m_window), item.pWnd->GetHandle(), x, y);
			item.pWnd->MoveWindow(x, y, width, height);
// 			printf("%d, %d, %d, %d,\n",(int)(item.x * fRadio + 0.5), 
// 				(int)(item.y * fRadio + 0.5), 
// 				(int)(item.width * fRadio + 0.5), 
// 				(int)(item.height * fRadio + 0.5)
// 				);
		}
	}
}

int CWidgetWin32Layout::InitCreateDlgItems()
{
	CDlgResource::iterator	it, itEnd;
	int			nRet;

	itEnd = m_dlgRes->end();
	for (it = m_dlgRes->begin(); it != itEnd; ++it)
	{
		CDlgResource::CWidgetItem	&item = *it;

		strrep((TCHAR*)item.strName.c_str(), '&', '_');
		nRet = CreateWidget(item);
		// ASSERT(item.pWnd != NULL);
	}

	return ERR_OK;
}

void CWidgetWin32Layout::PutChild(CBaseWnd *pChild, int x, int y)
{
	if (pChild->GetHandle())
		gtk_fixed_put(GTK_FIXED(m_window), pChild->GetHandle(), x, y);
}


bool CWidgetWin32Layout::ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew)
{
	// return m_layout.ReplaceChildPos(nIDChildSrcPos, pChildNew);
	CDlgResource::CWidgetItem	*item = m_dlgRes->GetItemByID(nIDChildSrcPos);
	if (!item)
		return false;

	CDlgResource::CWidgetItem	newItem = *item;
	newItem.widgetClass = CDlgResource::WC_UNKNOWN;
	newItem.nID = pChildNew->GetCtrlID();
	newItem.pWnd = pChildNew;
	newItem.strName.resize(0);
	newItem.nStyle = 0;

	PutChild(pChildNew, newItem.x, newItem.y);
	m_dlgRes->push_back(newItem);

	ReAllocateItemSize(newItem);

	return true;
}

void CWidgetWin32Layout::Cb_SizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	CWidgetWin32Layout	*pLayout = (CWidgetWin32Layout*)user_data;

	pLayout->OnSizeAllocate(allocation);
}

void CWidgetWin32Layout::ReAllocateItemSize(CDlgResource::CWidgetItem &item)
{
	GtkAllocation	size;
	float			fRadio;
	GtkRequisition	requisition;

	GtkWidget	*temp = gtk_button_new_with_label("Test");

	gtk_widget_size_request(temp, &requisition);
	fRadio = (float)requisition.height / 10;
	if (fRadio < 1.0 || fRadio >= 5.0)
		fRadio = 1.0;

	fRadio = 2.0;

	gtk_widget_destroy(temp);

	CRect		rc;
	m_pWndParent->GetWindowRect(&rc);
	m_pWndParent->MoveWindow(rc.left, rc.top, m_dlgRes->m_width * fRadio, m_dlgRes->m_height * fRadio, false);

	printf("Radio: %d, %f\n", requisition.height, fRadio);

	if (item.pWnd)
	{
		int		x, y, width, height;
		x = (int)(item.x * fRadio + 0.5);
		y = (int)(item.y * fRadio + 0.5);
		width = (int)(item.width * fRadio + 0.5);
		height = (int)(item.height * fRadio + 0.5);
		ASSERT(width > 0 && height > 0);
		if (item.widgetClass == CDlgResource::WC_COMB_BOX || 
			item.widgetClass == CDlgResource::WC_BUTTON || 
			item.widgetClass == CDlgResource::WC_COLOR_BUTTON || 
			item.widgetClass == CDlgResource::WC_RADIO_BUTTON || 
			item.widgetClass == CDlgResource::WC_CHECK_BOX || 
			item.widgetClass == CDlgResource::WC_EDIT)
		{
			gtk_widget_size_request(item.pWnd->GetHandle(), &requisition);
			height = requisition.height;
			// width = requisition.width;
		}
		// gtk_fixed_move(GTK_FIXED(m_window), item.pWnd->GetHandle(), x, y);
		item.pWnd->MoveWindow(x, y, width, height);
// 			printf("%d, %d, %d, %d,\n",(int)(item.x * fRadio + 0.5), 
// 				(int)(item.y * fRadio + 0.5), 
// 				(int)(item.width * fRadio + 0.5), 
// 				(int)(item.height * fRadio + 0.5)
// 				);
	}
}

int CWidgetWin32Layout::CreateWidget(CDlgResource::CWidgetItem &item)
{
	bool		bRet;

	switch (item.widgetClass)
	{
	case CDlgResource::WC_LABEL:
		{
			CWidgetLable	*pWidget = new CWidgetLable;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_GROUP_BOX:
		{
			CWidgetFrame	*pWidget = new CWidgetFrame;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_BUTTON:
		{
			CWidgetButton	*pWidget = new CWidgetButton;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_COLOR_BUTTON:
		{
			CWidgetColorButton	*pWidget = new CWidgetColorButton;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_RADIO_BUTTON:
		{
			CWidgetRadioButton	*pWidget = new CWidgetRadioButton;
			CWidgetRadioButton	*pWidgetRadioStart = NULL;

			// find start group radio button.
			if (!IsFlagSet(item.nStyle, WS_GROUP))
			{
				CDlgResource::iterator	it, itEnd;

				itEnd = m_dlgRes->end();
				for (it = m_dlgRes->begin(); it != itEnd; ++it)
				{
					CDlgResource::CWidgetItem	&i = *it;
					if (i.widgetClass == CDlgResource::WC_RADIO_BUTTON && i.pWnd)
					{
						if (!pWidgetRadioStart)
							pWidgetRadioStart = (CWidgetRadioButton*)i.pWnd;
						if (IsFlagSet(i.nStyle, WS_GROUP))
						{
							pWidgetRadioStart = (CWidgetRadioButton*)i.pWnd;
							printf("Start Group: %X, %s\n", pWidgetRadioStart, i.strName.c_str());
						}
					}
				}
			}

			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, pWidgetRadioStart, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_CHECK_BOX:
		{
			CWidgetCheckButton	*pWidget = new CWidgetCheckButton;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_EDIT:
		{
			CWidgetEditBox	*pWidget = new CWidgetEditBox;
			bRet = pWidget->Create(item.strName.c_str(), m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_COMB_BOX:
		{
			CWidgetComboBox	*pWidget = new CWidgetComboBox;
			bRet = pWidget->Create(m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_LISTBOX:
		{
			CWidgetListBox	*pWidget = new CWidgetListBox;
			bRet = pWidget->Create(m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_LISTCTRL:
		{
			CWidgetListCtrl	*pWidget = new CWidgetListCtrl;
			bRet = pWidget->Create(m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	case CDlgResource::WC_TREECTRL:
		{
			CWidgetTreeCtrl	*pWidget = new CWidgetTreeCtrl;
			bRet = pWidget->Create(m_pWndParent, item.nID);
			item.pWnd = pWidget;
		}
		break;
	default:
		// ASSERT(0);
		printf(_T("Unknown class type: %d, Name: %s, %d"), item.widgetClass, item.strName.c_str(), CDlgResource::WC_LABEL);
		break;
	}

	if (item.pWnd)
	{
		ASSERT(item.pWnd != NULL);
		PutChild(item.pWnd, item.x, item.y);
		// item.pWnd->MoveWindow(item.x, item.y, item.width, item.height);
		item.pWnd->ShowWindow(SW_SHOW);
		ASSERT(item.pWnd != NULL);
	}

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// CBaseDialog
CBaseDialog::CBaseDialog(UINT nIDTemplate)
{
	m_nIDTemplate = nIDTemplate;

	m_bModal = FALSE;
}

CBaseDialog::~CBaseDialog()
{
}

int CBaseDialog::DoModal(CBaseWnd *pWndParent)
{
	int		nRet;

	m_bModal = TRUE;

	nRet = CResourceMgr::GetDialog(m_dlgRes, m_nIDTemplate);
	printf("CResourceMgr::GetDialog: %d\n");
	if (nRet != ERR_OK)
		ERRLOG1(_T("Can't get dialog template by ID: %d"), m_nIDTemplate);

	printf("CResourceMgr::GetDialog: %s\n", m_dlgRes.m_strCaption.c_str());

// 	if (!CBaseWnd::Create(m_dlgRes.m_strCaption.c_str(), 
// 		m_dlgRes.m_x, m_dlgRes.m_y, m_dlgRes.m_width, m_dlgRes.m_height, pWndParent, WS_OVERLAPPED | WS_NOOWNERDRAW))
// 		return IDCANCEL;

	// m_window = gtk_dialog_new_with_buttons(m_dlgRes.m_strCaption.c_str(), )
// 	m_window = gtk_dialog_new_with_buttons(m_dlgRes.m_strCaption.c_str(), 
// 		pWndParent != NULL ? GTK_WINDOW(pWndParent->GetHandle()) : NULL,  
// 		(GtkDialogFlags)(GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), 
// 		NULL);
	m_window = gtk_dialog_new();

	g_signal_connect(m_window, "response", G_CALLBACK(cb_response), this);

	SetWindowText(m_dlgRes.m_strCaption.c_str());

	MoveWindow(m_dlgRes.m_x, m_dlgRes.m_y, m_dlgRes.m_width, m_dlgRes.m_height);

	gtk_window_set_modal(GTK_WINDOW(m_window), true);

	ConnectGtkSignals();

	m_layout.Create(this, &m_dlgRes);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_window)->vbox), m_layout.GetHandle());
	// GtkContainerAdd(&m_layout);

	OnInitDialog();

	gint	result;

	result = gtk_dialog_run(GTK_DIALOG(m_window));

	gtk_widget_hide(m_window);

	OnCommand(result, 0);

	return result;
}

BOOL CBaseDialog::Create(UINT nIDTemplate, CBaseWnd *pWndParent)
{
	int		nRet;

	m_bModal = FALSE;

	nRet = CResourceMgr::GetDialog(m_dlgRes, m_nIDTemplate);
	if (nRet != ERR_OK)
		ERRLOG1(_T("Can't get dialog template by ID: %d"), m_nIDTemplate);

	if (IsFlagSet(m_dlgRes.m_nStyle, WS_CHILD))
	{
		m_window = gtk_layout_new(NULL, NULL);
		m_layout.Create(this, &m_dlgRes);
		gtk_layout_put(GTK_LAYOUT(m_window), m_layout.GetHandle(), 0, 0);
		m_layout.ShowWindow(SW_SHOW);
		ShowWindow(SW_SHOW);

		// MoveWindow(m_dlgRes.m_x, m_dlgRes.m_y, m_dlgRes.m_width, m_dlgRes.m_height);
	}
	else
	{
		m_window = gtk_dialog_new();

		g_signal_connect(m_window, "response", G_CALLBACK(cb_response), this);

		SetWindowText(m_dlgRes.m_strCaption.c_str());

		gtk_window_set_modal(GTK_WINDOW(m_window), false);

		MoveWindow(m_dlgRes.m_x, m_dlgRes.m_y, m_dlgRes.m_width, m_dlgRes.m_height);

		m_layout.Create(this, &m_dlgRes);
		// m_layout.ShowWindow(SW_SHOW);
		gtk_container_add(GTK_CONTAINER (GTK_DIALOG(m_window)->vbox), m_layout.GetHandle());

		ConnectGtkSignals();
	}

	OnInitDialog();

	gtk_widget_show(m_window);

	return TRUE;
}

bool CBaseDialog::Destroy()
{
	if (IsFlagSet(m_dlgRes.m_nStyle, WS_CHILD))
	{
		m_layout.Destroy();
		m_window = NULL;
	}
	else
		CBaseWnd::Destroy();
}

void CBaseDialog::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDOK)
		OnOK();
	else if (uID == IDCANCEL)
		OnCancel();
}

void CBaseDialog::OnOK()
{
	gtk_dialog_response(GTK_DIALOG(m_window), IDOK);
}

void CBaseDialog::OnCancel()
{
	gtk_dialog_response(GTK_DIALOG(m_window), IDCANCEL);
}

BOOL CBaseDialog::IsModal()
{
	return m_bModal;
}

BOOL CBaseDialog::OnInitDialog()
{
    // gtk_container_remove(GTK_CONTAINER (m_window), GTK_DIALOG(m_window)->vbox);

	// 在开始的时候初始化多语言支持对象
//	m_LangDlgHelper.InitAddAll(m_hWnd);

	// 切换为本地语言
//	OnLanguageChanged();

	//
	// 居中此对话框
// 	if (!IsChildWnd(m_hWnd))
// 	{
// 		CenterWindowToMonitor(m_hWnd);
// 	}

	return TRUE;
}

void CBaseDialog::CheckButton(int nIDButton, bool bCheck)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDButton);
	if (pChild)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pChild->GetHandle()), bCheck);
	}
}

bool CBaseDialog::IsButtonChecked(int nIDButton)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDButton);
	if (pChild)
	{
		return Tobool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pChild->GetHandle())));
	}
	else
		return false;
}

UINT CBaseDialog::GetDlgItemText(int nIDItem, LPTSTR szString, int nMaxCount)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	if (pChild)
	{
		pChild->GetWindowText(szString, nMaxCount);
		return true;
	}
	else
		return false;
}

UINT CBaseDialog::GetDlgItemText(int nIDItem, tstring &str)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	if (pChild)
	{
		pChild->GetWindowText(str);
		return true;
	}
	else
		return false;
}

BOOL CBaseDialog::SetDlgItemText(int nIDItem, LPCTSTR szString)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	if (pChild)
	{
		pChild->SetWindowText(szString);
		return true;
	}
	else
		return false;
}

UINT CBaseDialog::GetDlgItemInt(int nIDItem, BOOL *pTranslated, BOOL bSigned)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	TCHAR		szInt[64];
	int			nValue;
	LPCTSTR		szTemp;
	if (pChild)
	{
		pChild->GetWindowText(szInt, CountOf(szInt));
		szTemp = szInt;
		ReadInt(szTemp, nValue);
		if (pTranslated)
			*pTranslated = szTemp != szInt;
		return nValue;
	}
	else
	{
		if (pTranslated)
			*pTranslated = false;
		return 0;
	}
}

BOOL CBaseDialog::SetDlgItemInt(int nIDItem, UINT uValue, BOOL bSigned)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	TCHAR		szInt[64];
	if (pChild)
	{
		if (bSigned)
			_stprintf(szInt, "%d", uValue);
		else
			_stprintf(szInt, "%u", uValue);
		pChild->SetWindowText(szInt);
		return true;
	}
	else
		return false;
}

bool CBaseDialog::EnableDlgItem(int nIDItem, bool bEnable)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	if (pChild)
	{
		pChild->EnableWindow(bEnable);
		return true;
	}
	else
		return false;
}

bool CBaseDialog::ShowDlgItem(int nIDItem, int nCmdShow)
{
	CBaseWnd	*pChild = m_dlgRes.GetDlgItem(nIDItem);
	if (pChild)
	{
		pChild->ShowWindow(nCmdShow);
		return true;
	}
	else
		return false;
}

CBaseWnd *CBaseDialog::GetDlgItem(int nIDItem)
{
	return m_dlgRes.GetDlgItem(nIDItem);
}


bool CBaseDialog::GetDlgItemRect(int nIDItem, RECT* lpRect)
{
	CBaseWnd	*pWnd = GetDlgItem(nIDItem);
	if (pWnd)
		return pWnd->GetWindowRect(lpRect);
	else
		return false;
}

bool CBaseDialog::ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew)
{
	return m_layout.ReplaceChildPos(nIDChildSrcPos, pChildNew);
}

GtkWidget *CBaseDialog::GetDlgItemHandle(int nIDItem)
{
	CBaseWnd	*p = m_dlgRes.GetDlgItem(nIDItem);
	if (p)
		return p->GetHandle();
	else
		return NULL;
}


void CBaseDialog::ConnectGtkSignals()
{
	//	gtk_window_set_decorated(GTK_WINDOW(m_window), false);

	/*当给窗口一个"delete_event"信号时(这个信号是由窗口管
	*理器发出的，通常是在点击窗口标题条右边的"×"按钮，或
	*者在标题条的快捷菜单上选择"close"选项时发出的)，我们
	*要求调用上面定义的delete_event()函数传递给这个回调函数
	*的数据是NULL，回调函数会忽略这个参数*/
	gtk_signal_connect(GTK_OBJECT(m_window), "delete_event",
		GTK_SIGNAL_FUNC(CBDeleteEvent), this);

	/*这里，我们给"destory"事件连接一个信号处理函数，
	*当我们在窗口上调用gtk_widget_destroy()函数
	*或者在"delete_event"事件的回调函数中返回FALSE
	* 时会发生这个事件*/
	gtk_signal_connect(GTK_OBJECT(m_window), "destroy",
		GTK_SIGNAL_FUNC(CBDestroyEvent), this);
	// g_print("Register CBDestroyEvent: %X\n", this);

	g_signal_connect(m_window, "window_state_event",
		G_CALLBACK(cb_window_state_event), this);
}

void CBaseDialog::cb_response(GtkDialog *dialog, gint arg1, gpointer user_data)
{
	CBaseDialog *pThis = (CBaseDialog *)user_data;

	pThis->OnClose();
	// pThis->Destroy();
}
