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
		uint16					button;
		uint16					fsModifiers;
		// ATOM					idHotKey;
		uint16					idHotKey;
	};
	typedef vector<CmdAccKey>		V_ACCKEY;
	typedef V_ACCKEY::iterator		iterator;

public:
	CMPHotkey();
	virtual ~CMPHotkey();

	void Init(CBaseWnd *pWnd);

	void Quit();

	void Enable(bool bEnable);

	void RestoreDefaults();

	void OnHotKey(int nId, UINT fuModifiers, UINT uVirtKey);

	void Add(int cmd, int nKey, int fuModifiers);
	bool Set(int nIndex, int cmd, int nKey, int fuModifiers);
	bool Remove(int nIndex);
	CmdAccKey *Get(int nIndex);

	iterator begin() { return m_vAccKey.begin(); }
	iterator end() { return m_vAccKey.end(); }

protected:
	int LoadSettings();
	int SaveSettings();

	void RegisterAllHotKeys();
	void UnregisterAllHotKeys();

protected:
	bool			m_bEnabled;
	V_ACCKEY		m_vAccKey;

};

#endif // !defined(AFX_MPHOTKEY_H__3439EBF6_7B08_4BD1_8C77_F36037282BF0__INCLUDED_)
