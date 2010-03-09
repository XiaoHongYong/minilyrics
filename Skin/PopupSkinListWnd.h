
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

	virtual void CloseSkin();

	virtual void OnUIObjNotify(IUIObjNotify *pNotify);

	class CDataSkinListCtrl : public CSkinListCtrl
	{
	public:
		virtual void Invalidate() { }

	};

public:
	void AddColumn(LPCTSTR szCol, int nWidth)
		{ m_listCtrlData.AddColumn(szCol, nWidth); }
	void AddImageColumn(LPCTSTR szCol, int nWidth)
		{ m_listCtrlData.AddImageColumn(szCol, nWidth); }

	int GetLineHeight() { return m_nLineHeight; }
	void SetLineHeight(int nLineHeight)
		{ m_nLineHeight = nLineHeight; }

	int GetItemCount() const;

	virtual int GetCurSel();
	void SetCurSel(int nIndex);

	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();

	int InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex = 0, UINT nItemData = 0);
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	BOOL SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage);

	void SetItemData(int nItem, UINT nItemData);
	UINT GetItemData(int nItem);

	tstring GetItemText(int nItem, int nSubItem) const;

protected:
	CDataSkinListCtrl			m_listCtrlData;
	int							m_nLineHeight;

	CSkinListCtrl				*m_pListCtrl;

	int							m_nCurSel;

};
