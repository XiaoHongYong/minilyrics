#ifndef _ML_GEN_CTRL_INC_
#define _ML_GEN_CTRL_INC_

#include "BaseWnd.h"
#include "WidgetCtrlBase.h"
#include "ScrollBarHandler.h"

class CWidgetLable : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

class CWidgetButton : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	static void cb_button_click(GtkWidget *widget, gpointer data);

protected:

};

class CWidgetColorButton : public CWidgetButton
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	void SetColor(const MLColor &clr);
	void GetColor(MLColor &clr);

protected:

};

class CWidgetToggleButton : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	bool IsChecked();

	void SetCheck(bool bChecked);

protected:
	static void cb_toggle_button(GtkWidget *widget, gpointer data);

};

class CWidgetCheckButton : public CWidgetToggleButton
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

class CWidgetRadioButton : public CWidgetToggleButton
{
public:
	bool Create(LPCTSTR szText, CBaseWnd *pWndParent, CWidgetRadioButton *pGroupStart, int nID = ID_UNDEFINE);

};

class CWidgetFrame : public CWidgetCtrlBase
{
public:
	CWidgetFrame();

	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

/*
 * Edit Control Notification Codes
 */
#ifndef EN_CHANGE
#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#endif

class CWidgetEditBox : public CWidgetCtrlBase
{
public:
	CWidgetEditBox();
	~CWidgetEditBox();

	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	int GetWindowText(LPTSTR szString, int nMaxCount);
	int GetWindowText(tstring &str);
	bool SetWindowText(LPCTSTR szText);

	bool IsEditable();

	void SetEditable(bool bEditable);

	void SetPasswordMode(bool bInputPassword);

	void SetSel(int nStart, int nEnd);
	void ReplaceSel(LPCTSTR szText);
	void SetBkColor(const MLColor &clr);

	void SetMaxLength(int nMaxLen);

	void InsertText(LPCTSTR szText, int nPos);
	void DeleteText(int nStart, int nEnd);
	void DeleteSel();

	int GetLength();

	void AppendText(LPCTSTR szText);

	void SetFont(const CMLFont &font, const MLColor &clrText);

//	void CopySelToClipboard();
//	void PasteFromClipboard();

protected:
	static void cb_changed(GtkEditable *widget, gpointer user_data);
/*	static void cb_cut_paste(GtkEditable *widget, gpointer user_data);
	static void cb_del_from_cursor(GtkEntry     *entry,
							GtkDeleteType arg1,
							gint          arg2,
							gpointer      user_data);
	static void cb_insert_at_cursor(GtkEntry *entry,
							gchar    *arg1,
							gpointer  user_data);*/

};

class CWidgetBox : public CWidgetCtrlBase
{
public:
	CWidgetBox();
	~CWidgetBox();

	virtual bool Create(bool bHorz, bool bHomogeneous, int nSpacing, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	// pack 
	void PackStart(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding);

	void PackEnd(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding);

};



#endif // _ML_GEN_CTRL_INC_
