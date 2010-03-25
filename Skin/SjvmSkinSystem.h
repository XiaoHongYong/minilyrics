
#pragma once

#include "../SimpleJVM/SJVM.h"

class CUIObject;
class CSkinFactory;
class CSkinWnd;

typedef std::set<int>		SetInt;

class SjvmSkinWndEvent
{
public:
	SjvmSkinWndEvent() { m_jobject = JOBJ_NULL; }
	virtual ~SjvmSkinWndEvent();

	// Return true, the command will not be processed down.
	bool onCommand(int nCmdId);
	void onCreate(CSkinWnd *pWnd);
	void onDestroy();
	void onSize(int cx, int cy);
	void onMove(int x, int y);
	void onActivate(boolean bActived);
	void onMouseActive(boolean bMouseActive);
	void onTimer(int nTimerId);

	void AddSkinWnd(CSkinWnd *pWnd);
	void RemoveSkinWnd(CSkinWnd *pWnd);

public:
	tstring			m_strSkinWndName;
	vector<CSkinWnd*> m_vSkinWnds;
	jobject			m_jobject;
protected:
	friend class CSjvmSkinWndEventObj;

	static int		m_sOnCommand;
	static int		m_sOnCreate;
	static int		m_sOnDestroy;
	static int		m_sOnSize;
	static int		m_sOnMove;
	static int		m_sOnActivate;
	static int		m_sOnMouseActive;
	static int		m_sOnTimer;

};

typedef list<SjvmSkinWndEvent*>		ListSjvmSkinWndEvent;

class CSjvmUIObject : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmUIObject)
public:
	static void RegisterJNI();

	CSjvmUIObject() : CJObject(m_sTypeId, 0) { m_pSkinWnd = NULL; m_pUIObject = NULL; }

public:
	CUIObject		*m_pUIObject;
	CSkinWnd		*m_pSkinWnd;

};

class CSjvmSkinSystemEvent
{
public:
	CSjvmSkinSystemEvent() { m_jobject = 0; }

	void onInit();
	void onDestory();
	void onSkinWndCreate(CSkinWnd *pSkinWnd);
	void onSkinWndDestory(CSkinWnd *pSkinWnd);

	jobject			m_jobject;

	static int		m_onInit;
	static int		m_onDestory;
	static int		m_onSkinWndCreate;
	static int		m_onSkinWndDestory;

};
typedef list<CSjvmSkinSystemEvent*>		ListSkinSystemEvent;

class CSjvmSkinSystem : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmSkinSystem)
public:
	CSjvmSkinSystem();

public:
	static void Init(CSkinFactory *pSkinFactory);
	static void Quit();

	static void LoadScript(LPCTSTR szScript);
	static void CloseScript();

	static void OnSkinWndCreate(CSkinWnd *pSkinWnd);
	static void OnSkinWndDestory(CSkinWnd *pSkinWnd);

protected:
	static void RegisterJNI();

protected:
	static bool SJNI_API registerSkinSystemEvent(jobject event);
	static void SJNI_API unRegisterSkinSystemEvent(jobject event);

	static bool SJNI_API registerSkinWndEvent(LPCTSTR szWndName, jobject event);
	static void SJNI_API unRegisterSkinWndEvent(jobject event);

	static jobject SJNI_API getSkinWnd(LPCTSTR skinWndName);

	static int SJNI_API getCommandID(LPCTSTR string);

	static void SJNI_API writeProfileInt(LPCTSTR szValueName, int nValue);
	static void SJNI_API writeProfileString(LPCTSTR szValueName, LPCTSTR szValue);

	static int SJNI_API getProfileInt(LPCTSTR szValueName, int nDefault);
	static jobject SJNI_API getProfileString(LPCTSTR szValueName, LPCTSTR szDefault);

protected:
	static ListSjvmSkinWndEvent		m_listSkinWndEvent;
	static ListSkinSystemEvent		m_listSkinSystemEvent;

public:
	static CSkinFactory				*m_pSkinFactory;

};
