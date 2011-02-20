#pragma once

#include "PopupSkinListWnd.h"
#include "SkinTextButton.h"

class CSkinComboBoxEventNotify : public IUIObjNotify
{
public:
	CSkinComboBoxEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_SEL_CHANGED; }
	enum Command
	{
		C_SEL_CHANGED,
	};

	Command				cmd;

};

class CSkinComboBox : public CSkinTextButton, public IPopupSkinWndNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinTextButton);
public:
	CSkinComboBox();
	virtual ~CSkinComboBox();

	void OnCreate();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	virtual void ButtonDownAction();
	virtual void ButtonUpAction();

public:
	virtual void PopupSkinWndOnSelected();
	virtual void PopupSkinWndOnDestroy();

	CPopupSkinListWnd &GetListData() { return m_popupListWnd; }

public:
	// Interface for combo box.
	int GetCount() const;

	virtual int GetCurSel();
	void SetCurSel(int nIndex);

	int AddString(LPCTSTR szStr);
	int InsertString(int nIndex, LPCTSTR lpszString);
	void SetItemData(int nIndex, UINT nItemData);
	UINT GetItemData(int nIndex);
	UINT GetItemText(int nIndex, tstring &str);

	bool DeleteString(UINT nIndex);

	void ResetContent();

	bool GetSelItemText(tstring &str);

protected:
	CPopupSkinListWnd		m_popupListWnd;

};
