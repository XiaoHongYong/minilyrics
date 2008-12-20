// CommonWidget.cpp: implementation of the CommonWidget class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "CommonWidget.h"


//////////////////////////////////////////////////////////////////////////

bool CWidgetLable::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_pWndParent = pWndParent;
	m_nID = nID;

	m_window = gtk_label_new(szText);
	ASSERT(m_window);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_button_new_with_mnemonic(szText);
	ASSERT(m_window);

	gtk_signal_connect(GTK_OBJECT(m_window), "clicked", GTK_SIGNAL_FUNC(cb_button_click), this);

	return true;
}

void CWidgetButton::cb_button_click(GtkWidget *widget, gpointer data)
{
	CWidgetButton	*pThis = (CWidgetButton *)data;
	if (pThis->m_pWndParent)
		pThis->m_pWndParent->OnCommand(pThis->GetCtrlID(), 0);
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetColorButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_color_button_new();
	ASSERT(m_window);

	gtk_signal_connect(GTK_OBJECT(m_window), "color-set", GTK_SIGNAL_FUNC(cb_button_click), this);

	return true;
}

void CWidgetColorButton::SetColor(const MLColor &clr)
{
	GdkColor	color;
	GdkColormap	*clrMap;

	color.red = clr.r();
	color.green = clr.g();
	color.blue = clr.b();

	clrMap = gtk_widget_get_colormap(m_window);
	gdk_colormap_alloc_color(clrMap, &color, true, true);

	gtk_color_button_set_color(GTK_COLOR_BUTTON(m_window), &color);
}

void CWidgetColorButton::GetColor(MLColor &clr)
{
	GdkColor	color;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(m_window), &color);

	clr.set(color.red, color.green, color.blue);
}

/*
void CWidgetColorButton::SetBgColor(const MLColor &clr)
{
	GdkColor	gclr;
	gclr.blue = clr.blue;
	gclr.green = clr.green;
	gclr.red = clr.red;
	gclr.pixel = 0;

	gtk_widget_modify_base(m_window, GTK_STATE_NORMAL, &gclr);
	gtk_widget_modify_fg(m_window, GTK_STATE_NORMAL, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_NORMAL, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_SELECTED, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_PRELIGHT, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_ACTIVE, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_INSENSITIVE, &gclr);
	gtk_widget_modify_bg(m_window, GTK_STATE_PRELIGHT, &gclr);
}*/


//////////////////////////////////////////////////////////////////////////

bool CWidgetToggleButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_toggle_button_new_with_mnemonic(szText);
	ASSERT(m_window);

	// gtk_signal_connect(GTK_OBJECT(m_window), "toggled", GTK_SIGNAL_FUNC(cb_toggle_button), this);
	gtk_signal_connect(GTK_OBJECT(m_window), "clicked", GTK_SIGNAL_FUNC(cb_toggle_button), this);

	return true;
}

void CWidgetToggleButton::cb_toggle_button(GtkWidget *widget, gpointer data)
{
	CWidgetToggleButton	*pThis = (CWidgetToggleButton *)data;
	if (pThis->m_pWndParent)
		pThis->m_pWndParent->OnCommand(pThis->GetCtrlID(), 0);
}

bool CWidgetToggleButton::IsChecked()
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_window)))
		return true;
	else
		return false;
}

void CWidgetToggleButton::SetCheck(bool bChecked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_window), bChecked);
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetCheckButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_check_button_new_with_mnemonic(szText);
	ASSERT(m_window);

	// gtk_signal_connect(GTK_OBJECT(m_window), "toggled", GTK_SIGNAL_FUNC(cb_toggle_button), this);
	gtk_signal_connect(GTK_OBJECT(m_window), "clicked", GTK_SIGNAL_FUNC(cb_toggle_button), this);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetRadioButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, CWidgetRadioButton *pGroupStart, int nID)
{
	GSList	*radioGroup = NULL;

	m_nID = nID;
	m_pWndParent = pWndParent;

	if (pGroupStart)
		radioGroup = gtk_radio_button_group(GTK_RADIO_BUTTON(pGroupStart->GetHandle())); 

	m_window = gtk_radio_button_new_with_mnemonic(radioGroup, szText);
	// m_window = gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(pGroup->m_pRadioGroup), szText);
	ASSERT(m_window);

	// gtk_signal_connect(GTK_OBJECT(m_window), "toggled", GTK_SIGNAL_FUNC(cb_toggle_button), this);
	gtk_signal_connect(GTK_OBJECT(m_window), "clicked", GTK_SIGNAL_FUNC(cb_toggle_button), this);

	return true;
}

//////////////////////////////////////////////////////////////////////////

CWidgetFrame::CWidgetFrame()
{

}

bool CWidgetFrame::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_frame_new(szText);
	ASSERT(m_window);

	return true;
}

//////////////////////////////////////////////////////////////////////////

CWidgetEditBox::CWidgetEditBox()
{

}

CWidgetEditBox::~CWidgetEditBox()
{

}

bool CWidgetEditBox::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	m_window = gtk_entry_new();
	ASSERT(m_window);

	gtk_entry_set_text(GTK_ENTRY(m_window), szText);
	gtk_signal_connect(GTK_OBJECT(m_window), "changed", GTK_SIGNAL_FUNC(cb_changed), this);
// 	gtk_signal_connect(GTK_OBJECT(m_window), "cut-clipboard", GTK_SIGNAL_FUNC(cb_cut_paste), this);
// 	gtk_signal_connect(GTK_OBJECT(m_window), "paste-clipboard", GTK_SIGNAL_FUNC(cb_cut_paste), this);
// 	gtk_signal_connect(GTK_OBJECT(m_window), "delete-from-cursor", GTK_SIGNAL_FUNC(cb_del_from_cursor), this);
// 	gtk_signal_connect(GTK_OBJECT(m_window), "insert-at-cursor", GTK_SIGNAL_FUNC(cb_insert_at_cursor), this);

	// SetEditable(true);

	return true;
}

int CWidgetEditBox::GetWindowText(LPTSTR szString, int nMaxCount)
{
	_tcscpy_safe(szString, nMaxCount, gtk_entry_get_text(GTK_ENTRY(m_window)));

	return _tcslen(szString);
}

int CWidgetEditBox::GetWindowText(tstring &str)
{
	str = gtk_entry_get_text(GTK_ENTRY(m_window));

	return str.size();
}

bool CWidgetEditBox::SetWindowText(LPCTSTR szText)
{
	gtk_entry_set_text(GTK_ENTRY(m_window), szText);
	return true;
}

bool CWidgetEditBox::IsEditable()
{
	return gtk_editable_get_editable(GTK_EDITABLE(m_window));
}

void CWidgetEditBox::SetEditable(bool bEditable)
{
	gtk_editable_set_editable(GTK_EDITABLE(m_window), bEditable);
}

void CWidgetEditBox::SetPasswordMode(bool bInputPassword)
{
	gtk_entry_set_visibility(GTK_ENTRY(m_window), !bInputPassword);
}

void CWidgetEditBox::SetSel(int nStart, int nEnd)
{
	gtk_entry_select_region(GTK_ENTRY(m_window), nStart, nEnd);
}

void CWidgetEditBox::ReplaceSel(LPCTSTR szText)
{
	int		nPos;

	gtk_editable_delete_selection(GTK_EDITABLE(m_window));

	nPos = gtk_editable_get_position(GTK_EDITABLE(m_window));

	gtk_editable_insert_text(GTK_EDITABLE(m_window), szText, strlen(szText), &nPos);
}

void CWidgetEditBox::SetBkColor(const MLColor &clr)
{
	
}

void CWidgetEditBox::SetMaxLength(int nMaxLen)
{
	gtk_entry_set_max_length(GTK_ENTRY(m_window), nMaxLen);
}

void CWidgetEditBox::InsertText(LPCTSTR szText, int nPos)
{
	gtk_editable_insert_text(GTK_EDITABLE(m_window), szText, strlen(szText), &nPos);
}

void CWidgetEditBox::DeleteText(int nStart, int nEnd)
{
	gtk_editable_delete_text(GTK_EDITABLE(m_window), nStart, nEnd);
}

void CWidgetEditBox::DeleteSel()
{
	gtk_editable_delete_selection(GTK_EDITABLE(m_window));
}

int CWidgetEditBox::GetLength()
{
	return GTK_ENTRY(m_window)->text_length;
}

void CWidgetEditBox::AppendText(LPCTSTR szText)
{
	int		nPos;

	nPos = GTK_ENTRY(m_window)->text_length;

	gtk_editable_insert_text(GTK_EDITABLE(m_window), szText, -1, &nPos);
}

void CWidgetEditBox::SetFont(const CMLFont &font, const MLColor &clrText)
{
	
}

//void CopySelToClipboard();
//void PasteFromClipboard();

void CWidgetEditBox::cb_changed(GtkEditable *widget, gpointer user_data)
{
	CWidgetEditBox		*pThis = (CWidgetEditBox *)user_data;
	pThis->m_pWndParent->OnCommand(pThis->m_nID, EN_CHANGE);
}

// void CWidgetEditBox::cb_cut_paste(GtkEditable *widget, gpointer user_data)
// {
// 	CWidgetEditBox		*pThis = (CWidgetEditBox *)user_data;
// 	pThis->m_pWndParent->OnCommand(pThis->m_nID, EN_CHANGE);
// }
// 
// void CWidgetEditBox::cb_del_from_cursor(GtkEntry     *entry,
// 					GtkDeleteType arg1,
// 					gint          arg2,
// 					gpointer      user_data)
// {
// 	CWidgetEditBox		*pThis = (CWidgetEditBox *)user_data;
// 	pThis->m_pWndParent->OnCommand(pThis->m_nID, EN_CHANGE);
// }
// 
// void CWidgetEditBox::cb_insert_at_cursor(GtkEntry *entry,
// 					gchar    *arg1,
// 					gpointer  user_data)
// {
// 	CWidgetEditBox		*pThis = (CWidgetEditBox *)user_data;
// 	pThis->m_pWndParent->OnCommand(pThis->m_nID, EN_CHANGE);
// }

//////////////////////////////////////////////////////////////////////////

CWidgetBox::CWidgetBox()
{

}

CWidgetBox::~CWidgetBox()
{

}

bool CWidgetBox::Create(bool bHorz, bool bHomogeneous, int nSpacing, CBaseWnd *pWndParent, int nID)
{
	m_pWndParent = pWndParent;
	m_nID = nID;

	if (bHorz)
		m_window = gtk_hbox_new(bHomogeneous, nSpacing);
	else
		m_window = gtk_vbox_new(bHomogeneous, nSpacing);
	ASSERT(m_window);

	if (!m_window)
		return false;

	if (!pWndParent)
		return false;

    gtk_widget_show(m_window);

	return true;
}

// pack 
void CWidgetBox::PackStart(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding)
{
	gtk_box_pack_start(GTK_BOX(m_window), pWnd->GetHandle(), bExpand, bFill, nPadding);
}

void CWidgetBox::PackEnd(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding)
{
	gtk_box_pack_end(GTK_BOX(m_window), pWnd->GetHandle(), bExpand, bFill, nPadding);
}
