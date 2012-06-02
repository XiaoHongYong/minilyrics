// Menu.h: interface for the CMLMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
#define AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_

class CBaseWnd;

class CMLMenu  
{
public:
	CMLMenu();
	virtual ~CMLMenu();

	bool IsValid() const { return m_hMenu != NULL; }

	bool CreatePopupMenu();

	virtual void OnLoadMenu() { }

	void Destroy();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	virtual void TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	int GetItemCount();

	void EnableItem(int nID, bool bEnable);
	void CheckItem(int nID, bool bCheck);
	void CheckRadioItem(int nID, int nStartID, int nEndID);

	virtual void AppendItem(UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey = _T(""));
	virtual void InsertItem(int nPos, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey = _T(""));
	virtual void InsertItemByID(UINT nPosID, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey = _T(""));

	void AppendSeperator();
	void InsertSeperator(int nPos);

	CMLMenu AppendSubmenu(LPCTSTR szText);
	CMLMenu InsertSubmenu(int nPos, LPCTSTR szText);

	void RemoveItem(int nPos);

	bool GetMenuItemText(UINT nItem, tstring &strText, bool bByPosition);

	bool GetMenuItemInfo(UINT nItem, tstring &strText, UINT &nID, bool &bSubMenu, bool bByPosition);

	bool HasItem(int nPos);
	bool HasSubmenu(int nPos);
	CMLMenu GetSubmenu(int nPos);

	CMLMenu & operator = (CMLMenu &menu);

public:
	//
	// APIs for Win32
	//

	bool LoadMenu(int nID);
	bool LoadPopupMenu(int nID, int nSubMenu);

	void Attach(HMENU hMenu) { m_hMenu = hMenu; }
	HMENU GetSubMenuHandle(int nPos);
	HMENU GetHandle();

	HMENU FindMenuPos(HMENU hMenu, UINT nIDMenu);

protected:
	HMENU			m_hMenu;
	int				m_nSubMenu;
	bool			m_bFree;

};

BOOL ToLocalMenu(CMLMenu *pMenu);
BOOL ToLocalMenu(HMENU hMenu);

#endif // !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
