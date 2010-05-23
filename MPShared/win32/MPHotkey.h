// MPHotkey.h: interface for the CMPHotkey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPHOTKEY_H__3439EBF6_7B08_4BD1_8C77_F36037282BF0__INCLUDED_)
#define AFX_MPHOTKEY_H__3439EBF6_7B08_4BD1_8C77_F36037282BF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct MPHotKeySection
{
	LPCTSTR szName;
	int		*vHotkeys;
};

extern MPHotKeySection	g_vHotkeySections[];

class CMPHotkey  
{
public:
	struct CmdAccKey
	{
		int						cmd;
		bool					bGlobal;
		uint16					button;
		uint16					fsModifiers;
		uint16					idHotKey;
	};
	typedef vector<CmdAccKey>		V_ACCKEY;
	typedef V_ACCKEY::iterator		iterator;

public:
	CMPHotkey();
	virtual ~CMPHotkey();

	void Init();

	void SetEventWnd(CBaseWnd *pWnd);

	void Quit();

	void EnableGlobalHotkey(bool bEnable);

	void RestoreDefaults();

	void OnHotKey(int nId, UINT fuModifiers, UINT uVirtKey);

	// If this key was processed, return true.
	bool OnKeyDown(CMPSkinWnd *pWnd, UINT nChar, UINT nFlags);

	void Add(int cmd, bool bGlobal, int nKey, int fuModifiers);
	bool Set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers);
	bool Remove(int nIndex);
	CmdAccKey *Get(int nIndex);
	int GetByKey(int nKey, int nModifiers);
	int GetByCmd(int cmd, int nStartFind = -1);

	bool IsDefaultKey(int cmd);
	void RestoreDefaultKey(int cmd);

	bool GetHotkeyText(int cmd, tstring &strKey);

	iterator begin() { return m_vAccKey.begin(); }
	iterator end() { return m_vAccKey.end(); }

protected:
	int LoadSettings();
	int SaveSettings();

	void RegisterAllGlobalHotKeys();
	void UnregisterAllGlobalHotKeys();

protected:
	HWND			m_hWnd;
	bool			m_bGlobalHotkeyEnabled;
	V_ACCKEY		m_vAccKey;

};


#endif // !defined(AFX_MPHOTKEY_H__3439EBF6_7B08_4BD1_8C77_F36037282BF0__INCLUDED_)
