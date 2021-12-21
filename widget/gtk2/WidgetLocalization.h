// WidgetLocalization.h: interface for the CWidgetLocalization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIDGETLOCALIZATION_H__9C311C26_12B8_4F14_AB80_0CD2040E45C5__INCLUDED_)
#define AFX_WIDGETLOCALIZATION_H__9C311C26_12B8_4F14_AB80_0CD2040E45C5__INCLUDED_

#pragma once

class CMLMenu;
class CBaseWnd;

BOOL ToLocalMenu(CMLMenu *pMenu);

class CLangTreeCtrlHelper
{
public:
	CLangTreeCtrlHelper();
	~CLangTreeCtrlHelper();

	void Init(CBaseWnd *pWndParent, UINT nIDTreeCtrl);
	void Destroy();

	void ToLocal();

};

/*

class CLangListCtrlHelper
{
public:
	CLangListCtrlHelper();
	~CLangListCtrlHelper();

	void Init(HWND hWnd);

	void ToLocal();

public:
	HWND			m_hWnd;
	vector<tstring>	m_vStrHeader;
};
*/
class CLangDialogHelper
{
public:
	CLangDialogHelper();
	~CLangDialogHelper();

public:
	void AddDlgItem(int nID);
	// void AddDlgItem(HWND hWndItem);

	// void RemoveDlgItem(HWND hWndItem);
	void RemoveDlgItem(int nID);

	void ToLocal();

public:
	void Destroy();
	void InitAddAll(CBaseWnd *pWndParent);
// 	struct DlgItem
// 	{
// 		HWND	hWnd;			// 子窗口句柄
// 		BOOL	bLoacalText;	// 是否需要转换窗口显示的文字到本地
// 		tstring	strInit;		// 子窗口的初始字符串
// 	};

	void Init(CBaseWnd *pWnd);

	// 需要切换语言的控件
	// vector<DlgItem>			m_vDlgItem;

	// 需要切换语言的ListCtrl控件
	// vector<CLangListCtrlHelper>	m_vListCtrlHelper;

//	HWND					m_hWndParent;
	tstring					m_strParent;
};

#endif // !defined(AFX_WidgetLocalization_H__9C311C26_12B8_4F14_AB80_0CD2040E45C5__INCLUDED_)
