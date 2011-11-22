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

	bool LoadMenu(int nID);

	void Destroy();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	virtual void TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	void EnableItem(int nID, bool bEnable);
	void CheckItem(int nID, bool bCheck);
	void CheckRadioItem(int nID, int nStartID, int nEndID);

	//
	// Create menu...
	//
	bool CreatePopupMenu();

	virtual void AppendItem(UINT nID, LPCTSTR szText);
	void AppendSeperator();
	virtual void InsertItem(int nPos, UINT nID, LPCTSTR szText);
	virtual void InsertItemByID(UINT nPosID, UINT nID, LPCTSTR szText);
	void InsertSeperator(int nPos);
	void RemoveItem(int nPos);

	bool GetMenuItemText(UINT nItem, tstring &strText, bool bByPosition);

	bool GetMenuItemInfo(UINT nItem, tstring &strText, UINT &nID, bool &bSubMenu, bool bByPosition);

public:
	bool LoadPopupMenu(int nID, int nSubMenu);

	virtual void OnLoadMenu() { }
	
protected:
	LPVOID			m_hMenu;
	int				m_nSubMenu;

};

BOOL ToLocalMenu(CMLMenu *pMenu);

#endif // !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
