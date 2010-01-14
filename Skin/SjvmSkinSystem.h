
#pragma once

#include "../SimpleJVM/SJVM.h"

class CUIObject;
class CSkinFactory;
class CSkinWnd;


class SjvmSkinWndEvent
{
public:
	SjvmSkinWndEvent() { m_jobject = JOBJ_NULL; }
	virtual ~SjvmSkinWndEvent();

	// Return true, the command will not be processed down.
	boolean onCommand(int nCmdId);
	void onCreate();
	void onDestroy();
	void onSize(int cx, int cy);
	void onMove(int x, int y);
	void onActivate(boolean bActived);

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

class CSjvmSkinWnd : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmSkinWnd)
public:
	CSjvmSkinWnd();

	static void RegisterJNI();

public:
	static jobject SJNI_API getUIObject(jobject thiz, int nId);
	static void SJNI_API setUIObjVisible(jobject thiz, int nId, bool bVisible);
	static bool SJNI_API getUIObjVisible(jobject thiz, int nId);

	static void SJNI_API setUIObjProperty(jobject thiz, int nId, LPCTSTR name, LPCTSTR value);

	static void SJNI_API setProperty(jobject thiz, LPCTSTR name, LPCTSTR value);

	static void SJNI_API exeCommand(jobject thiz, int nCmdId);

	static int SJNI_API getWndPositionX(jobject thiz);
	static int SJNI_API getWndPositionY(jobject thiz);
	static int SJNI_API getWndWidth(jobject thiz);
	static int SJNI_API getWndHeight(jobject thiz);

	static void SJNI_API setWndPosition(jobject thiz, int x, int y);
	static void SJNI_API moveWindow(jobject thiz, int x, int y, int width, int height);

protected:
	static CSkinWnd *GetSkinWndOfThis(jobject thiz);

public:
	CSkinWnd				*m_pSkinWnd;

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

protected:
	static ListSjvmSkinWndEvent		m_listSkinWndEvent;
	static ListSkinSystemEvent		m_listSkinSystemEvent;

public:
	static CSkinFactory				*m_pSkinFactory;

};
