
#pragma once

#include "PopupSkinWnd.h"
#include "../MPShared/SkinListCtrl.h"

class CPopupSkinListWnd : public CPopupSkinWnd, public IUIObjNotifyHandler
{
public:
	CPopupSkinListWnd();
	~CPopupSkinListWnd();

	void OnLoadWndSizePos();

	void OnSkinLoaded();

	virtual void OnUIObjNotify(IUIObjNotify *pNotify);

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

	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;

	int SelectString(int nStartAfter, LPCTSTR lpszString);

	void GetSelItemText(tstring &str);

protected:
	struct Item
	{
		int			nImage;
		tstring		strText;
		int			nData;

		Item(LPCTSTR szText) {
			strText = szText;
		}
		Item(LPCTSTR szText, int nImage, int nData) {
			this->strText = szText;
			this->nData = nData;
			this->nImage = nImage;
		}
	};
	typedef vector<Item>	VecItem;

	CSkinListCtrl		*m_pListCtrl;
	VecItem				m_vItems;

	int					m_nCurSel;

};
