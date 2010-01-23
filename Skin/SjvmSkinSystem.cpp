#include "stdafx.h"
#include "MLSkin.h"

#include "SjvmSkinSystem.h"

#define TID_SKIN_WND_NOTIFY_EVENT	CSjvmSkinWndEventObj::m_sTypeId
#define TID_SKIN_WND				CSjvmSkinWnd::m_sTypeId
#define TID_UIOBJECT				CSjvmUIObject::m_sTypeId

//////////////////////////////////////////////////////////////////////////


class CSjvmSkinWnd : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmSkinWnd)
public:
	CSjvmSkinWnd() : CJObject(m_sTypeId, 0)
	{
		m_pSkinWnd = NULL;
	}
	virtual ~CSjvmSkinWnd()
	{
		for (SetInt::iterator it = m_setTimers.begin(); it != m_setTimers.end(); ++it)
		{
			m_pSkinWnd->KillTimer(*it);
		}
		m_setTimers.clear();
	}

	static void RegisterJNI()
	{
		int		nRet;

		// Add class members definition.
		CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
		ASSERT(pClass);

		pClass->AddNativeDefaultConstructor();

		nRet = pClass->AddNativeMethod(false, "getUIObject", getUIObject, TID_UIOBJECT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "setUIObjVisible", setUIObjVisible, TID_VOID, TID_INT, TID_BOOLEAN, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "getUIObjVisible", getUIObjVisible, TID_BOOLEAN, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "setUIObjProperty", setUIObjProperty, TID_VOID, TID_INT, TID_STRING, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "setProperty", setProperty, TID_VOID, TID_STRING, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "exeCommand", exeCommand, TID_VOID, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "getWndPositionX", getWndPositionX, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "getWndPositionY", getWndPositionY, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "getWndWidth", getWndWidth, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "getWndHeight", getWndHeight, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "setWndPosition", setWndPosition, TID_VOID, TID_INT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "moveWindow", moveWindow, TID_VOID, TID_INT, TID_INT, TID_INT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "registerTimer", registerTimer, TID_INT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(false, "unregisterTimer", unregisterTimer, TID_VOID, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);
	}

public:
	static jobject SJNI_API getUIObject(jobject thiz, int nId)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return JOBJ_NULL;

		CUIObject	*pObj = pSkinWnd->GetUIObjectById(nId);
		if (!pObj)
			return JOBJ_NULL;

		jobject		jobj;
		CSjvmUIObject *pJObj = (CSjvmUIObject*)g_sjvm->NewJObject(TID_UIOBJECT, jobj);
		ASSERT(pJObj && pJObj->m_typeId == TID_UIOBJECT);

		pJObj->m_pUIObject = pObj;
		pJObj->m_pSkinWnd = pSkinWnd;

		return jobj;
	}

	static void SJNI_API setUIObjVisible(jobject thiz, int nId, bool bVisible)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		CUIObject *pObj = pSkinWnd->GetUIObjectById(nId);
		if (pObj)
			pObj->SetVisible(bVisible);
	}

	static bool SJNI_API getUIObjVisible(jobject thiz, int nId)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return false;

		CUIObject *pObj = pSkinWnd->GetUIObjectById(nId);
		if (pObj)
			return pObj->IsVisible();

		return false;
	}

	static void SJNI_API setUIObjProperty(jobject thiz, int nId, LPCTSTR name, LPCTSTR value)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		CUIObject *pObj = pSkinWnd->GetUIObjectById(nId);
		if (pObj)
			pObj->SetProperty(name, value);
	}

	static void SJNI_API setProperty(jobject thiz, LPCTSTR name, LPCTSTR value)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		pSkinWnd->SetProperty(name, value);
	}

	static void SJNI_API exeCommand(jobject thiz, int nCmdId)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		pSkinWnd->OnCustomCommand(nCmdId);
	}

	static int SJNI_API getWndPositionX(jobject thiz)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return 0;

		return pSkinWnd->m_rcBoundBox.left;
	}

	static int SJNI_API getWndPositionY(jobject thiz)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return 0;

		return pSkinWnd->m_rcBoundBox.top;
	}

	static int SJNI_API getWndWidth(jobject thiz)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return 0;

		return pSkinWnd->m_rcBoundBox.Width();
	}

	static int SJNI_API getWndHeight(jobject thiz)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return 0;

		return pSkinWnd->m_rcBoundBox.Height();
	}

	static void SJNI_API setWndPosition(jobject thiz, int x, int y)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		pSkinWnd->SetWindowPos(x, y);
	}

	static void SJNI_API moveWindow(jobject thiz, int x, int y, int width, int height)
	{
		CSkinWnd	*pSkinWnd = GetSkinWndOfThis(thiz);
		ASSERT(pSkinWnd);
		if (!pSkinWnd)
			return;

		pSkinWnd->MoveWindow(x, y, width, height, true);
	}

	static int SJNI_API registerTimer(jobject thiz, int nTimeOut)
	{
		CSjvmSkinWnd	*pThis = GetThis(thiz);
		ASSERT(pThis);
		if (!pThis)
			return 0;

		CSkinWnd		*pSkinWnd = pThis->m_pSkinWnd;

		int		nTimerId = pSkinWnd->m_nTimerIDMax;
		pSkinWnd->m_nTimerIDMax++;
		pThis->m_setTimers.insert(nTimerId);

		pSkinWnd->SetTimer(nTimerId, nTimeOut);

		return nTimerId;
	}

	static void SJNI_API unregisterTimer(jobject thiz, int nTimerId)
	{
		CSjvmSkinWnd	*pThis = GetThis(thiz);
		ASSERT(pThis);
		if (!pThis)
			return;

		CSkinWnd		*pSkinWnd = pThis->m_pSkinWnd;

		pThis->m_setTimers.erase(nTimerId);

		pSkinWnd->KillTimer(nTimerId);
	}

protected:
	static CSkinWnd *GetSkinWndOfThis(jobject thiz)
	{
		CSjvmSkinWnd	*pThis = (CSjvmSkinWnd*)g_sjvm->GetJObject(thiz);
		ASSERT(pThis && pThis->m_sTypeId == TID_SKIN_WND);
		if (!pThis)
			return NULL;

		ASSERT(pThis->m_pSkinWnd && CSjvmSkinSystem::m_pSkinFactory->IsValidSkinWnd(pThis->m_pSkinWnd));
		if (!CSjvmSkinSystem::m_pSkinFactory->IsValidSkinWnd(pThis->m_pSkinWnd))
			return NULL;

		return pThis->m_pSkinWnd;
	}

	static CSjvmSkinWnd *GetThis(jobject thiz)
	{
		CSjvmSkinWnd	*pThis = (CSjvmSkinWnd*)g_sjvm->GetJObject(thiz);
		ASSERT(pThis && pThis->m_sTypeId == TID_SKIN_WND);
		if (!pThis)
			return NULL;

		ASSERT(pThis->m_pSkinWnd && CSjvmSkinSystem::m_pSkinFactory->IsValidSkinWnd(pThis->m_pSkinWnd));
		if (!CSjvmSkinSystem::m_pSkinFactory->IsValidSkinWnd(pThis->m_pSkinWnd))
			return NULL;

		return pThis;
	}

public:
	CSkinWnd				*m_pSkinWnd;
	SetInt					m_setTimers;

};

IMPLENENT_JNI_OBJ(CSjvmSkinWnd, "SkinWnd")

//////////////////////////////////////////////////////////////////////////

class CSjvmSkinWndEventObj : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmSkinWndEventObj)
public:
	CSjvmSkinWndEventObj() : CJObject(m_sTypeId, 0) { }

	static bool SJNI_API onCommand(jobject thiz, int nCmdId) { return false; }
	static void SJNI_API onCreate(jobject thiz) { }
	static void SJNI_API onDestroy(jobject thiz) { }
	static void SJNI_API onSize(jobject thiz, int cx, int cy) { }
	static void SJNI_API onMove(jobject thiz, int x, int y) { }
	static void SJNI_API onActivate(jobject thiz, boolean bActived) { }
	static void SJNI_API onTimer(jobject thiz, int nTimerId) { }

	static void RegisterJNI()
	{
		int		nRet;

		// Add class members definition.
		CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
		ASSERT(pClass);

		pClass->AddNativeDefaultConstructor();

		SjvmSkinWndEvent::m_sOnCommand = nRet = pClass->AddNativeMethod(false, "onCommand", onCommand, TID_BOOLEAN, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnCreate = nRet = pClass->AddNativeMethod(false, "onCreate", onCreate, TID_VOID, TID_SKIN_WND, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnDestroy = nRet = pClass->AddNativeMethod(false, "onDestroy", onDestroy, TID_VOID, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnSize = nRet = pClass->AddNativeMethod(false, "onSize", onSize, TID_VOID, TID_INT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnMove = nRet = pClass->AddNativeMethod(false, "onMove", onMove, TID_VOID, TID_INT, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnActivate = nRet = pClass->AddNativeMethod(false, "onActivate", onActivate, TID_VOID, TID_BOOLEAN, TID_INVALID);
		ASSERT(nRet != -1);

		SjvmSkinWndEvent::m_sOnTimer = nRet = pClass->AddNativeMethod(false, "onTimer", onTimer, TID_VOID, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);
	}

};

IMPLENENT_JNI_OBJ(CSjvmSkinWndEventObj, "SkinWndEvent")

//////////////////////////////////////////////////////////////////////////

int		SjvmSkinWndEvent::m_sOnCommand = 0;
int		SjvmSkinWndEvent::m_sOnCreate = 0;
int		SjvmSkinWndEvent::m_sOnDestroy = 0;
int		SjvmSkinWndEvent::m_sOnSize = 0;
int		SjvmSkinWndEvent::m_sOnMove = 0;
int		SjvmSkinWndEvent::m_sOnActivate = 0;
int		SjvmSkinWndEvent::m_sOnTimer = 0;

// Return true, the command will not be processed down.
bool SjvmSkinWndEvent::onCommand(int nCmdId)
{
	if (m_sOnCommand >= 0)
	{
		return Tobool(g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnCommand, TID_INT, nCmdId));
	}

	return false;
}

void SjvmSkinWndEvent::onCreate(CSkinWnd *pWnd)
{
	if (m_sOnCreate >= 0)
	{
		jobject			jobj;
		CSjvmSkinWnd	*pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->NewJObject(TID_SKIN_WND, jobj);
		ASSERT(pJObjSkinWnd);
		pJObjSkinWnd->m_pSkinWnd = pWnd;
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnCreate, TID_SKIN_WND, jobj);
	}
}

void SjvmSkinWndEvent::onDestroy()
{
	if (m_sOnDestroy >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnDestroy);
	}
}

void SjvmSkinWndEvent::onSize(int cx, int cy)
{
	if (m_sOnSize >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnSize, TID_INT, cx, TID_INT, cy);
	}
}

void SjvmSkinWndEvent::onMove(int x, int y)
{
	if (m_sOnMove >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnMove, TID_INT, x, TID_INT, y);
	}
}

void SjvmSkinWndEvent::onActivate(boolean bActived)
{
	if (m_sOnActivate >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnActivate, TID_BOOLEAN, bActived);
	}
}

void SjvmSkinWndEvent::onTimer(int nTimerId)
{
	if (m_sOnTimer >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_sOnTimer, TID_INT, nTimerId);
	}
}

SjvmSkinWndEvent::~SjvmSkinWndEvent()
{
	g_sjvm->JniReleaseJObjRef(m_jobject);

	for (size_t i = 0; i < m_vSkinWnds.size(); i++)
		m_vSkinWnds[i]->SjvmSetNotifyEvent(NULL);
	m_vSkinWnds.clear();
}

void SjvmSkinWndEvent::AddSkinWnd(CSkinWnd *pWnd)
{
	m_vSkinWnds.push_back(pWnd);
}

void SjvmSkinWndEvent::RemoveSkinWnd(CSkinWnd *pWnd)
{
	for (size_t i = 0; i < m_vSkinWnds.size(); i++)
	{
		if (m_vSkinWnds[i] == pWnd)
		{
			m_vSkinWnds.erase(m_vSkinWnds.begin() + i);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

IMPLENENT_JNI_OBJ(CSjvmUIObject, "UIObject")

void CSjvmUIObject::RegisterJNI()
{
}


//////////////////////////////////////////////////////////////////////////


class CSjvmSkinSystemEventObj : public CJObject
{
	DECLARE_JNI_OBJ(CSjvmSkinSystemEventObj)
public:
	CSjvmSkinSystemEventObj() : CJObject(m_sTypeId, 0) { }

public:
	static void SJNI_API onInit() { }
	static void SJNI_API onDestory() { }
	static void SJNI_API onSkinWndCreate(jobject skinWnd) { }
	static void SJNI_API onSkinWndDestory(jobject skinWnd) { }

	static void RegisterJNI()
	{
		int		nRet;

		// Add class members definition.
		CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
		ASSERT(pClass);

		pClass->AddNativeDefaultConstructor();

		CSjvmSkinSystemEvent::m_onInit = nRet = pClass->AddNativeMethod(false, "onInit", onInit, TID_VOID, TID_INVALID);
		ASSERT(nRet != -1);

		CSjvmSkinSystemEvent::m_onDestory= nRet = pClass->AddNativeMethod(false, "onDestory", onDestory, TID_VOID, TID_INVALID);
		ASSERT(nRet != -1);

		CSjvmSkinSystemEvent::m_onSkinWndCreate = nRet = pClass->AddNativeMethod(false, "onSkinWndCreate", onSkinWndCreate, TID_VOID, TID_SKIN_WND, TID_INVALID);
		ASSERT(nRet != -1);

		CSjvmSkinSystemEvent::m_onSkinWndDestory = nRet = pClass->AddNativeMethod(false, "onSkinWndDestory", onSkinWndDestory, TID_VOID, TID_SKIN_WND, TID_INVALID);
		ASSERT(nRet != -1);
	}

};

IMPLENENT_JNI_OBJ(CSjvmSkinSystemEventObj, "SkinSystemEvent")

//////////////////////////////////////////////////////////////////////////

int		CSjvmSkinSystemEvent::m_onInit = 0;
int		CSjvmSkinSystemEvent::m_onDestory = 0;
int		CSjvmSkinSystemEvent::m_onSkinWndCreate = 0;
int		CSjvmSkinSystemEvent::m_onSkinWndDestory = 0;

void CSjvmSkinSystemEvent::onInit()
{
	if (m_onInit >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_onInit);
	}
}

void CSjvmSkinSystemEvent::onDestory()
{
	if (m_onDestory >= 0)
	{
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_onDestory);
	}
}

void CSjvmSkinSystemEvent::onSkinWndCreate(CSkinWnd *pSkinWnd)
{
	if (m_onSkinWndCreate >= 0)
	{
		jobject			jobj;
		CSjvmSkinWnd	*pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->NewJObject(TID_SKIN_WND, jobj);
		ASSERT(pJObjSkinWnd);
		pJObjSkinWnd->m_pSkinWnd = pSkinWnd;
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_onSkinWndCreate, TID_SKIN_WND, jobj);
	}
}

void CSjvmSkinSystemEvent::onSkinWndDestory(CSkinWnd *pSkinWnd)
{
	if (m_onSkinWndDestory >= 0)
	{
		jobject			jobj;
		CSjvmSkinWnd	*pJObjSkinWnd = (CSjvmSkinWnd*)g_sjvm->NewJObject(TID_SKIN_WND, jobj);
		ASSERT(pJObjSkinWnd);
		pJObjSkinWnd->m_pSkinWnd = pSkinWnd;
		g_sjvm->JniCallMethod(TID_INVALID, m_jobject, m_onSkinWndDestory, TID_SKIN_WND, jobj);
	}
}

//////////////////////////////////////////////////////////////////////////


IMPLENENT_JNI_OBJ(CSjvmSkinSystem, "SkinSystem")

ListSjvmSkinWndEvent	CSjvmSkinSystem::m_listSkinWndEvent;
ListSkinSystemEvent		CSjvmSkinSystem::m_listSkinSystemEvent;
CSkinFactory			*CSjvmSkinSystem::m_pSkinFactory = NULL;

CSjvmSkinSystem::CSjvmSkinSystem() : CJObject(m_sTypeId, 0)
{

}

void CSjvmSkinSystem::Init(CSkinFactory *pSkinFactory)
{
	ASSERT(g_sjvm == NULL);
	g_sjvm = new CSJVM();

	m_pSkinFactory = pSkinFactory;

	// Register class name
	CSjvmSkinWnd::RegisterClassType(g_sjvm);
	CSjvmSkinSystem::RegisterClassType(g_sjvm);
	CSjvmSkinSystemEventObj::RegisterClassType(g_sjvm);
	CSjvmSkinWndEventObj::RegisterClassType(g_sjvm);

	// Add class members definition.
	CSjvmSkinWnd::RegisterJNI();
	CSjvmSkinSystem::RegisterJNI();
	CSjvmSkinSystemEventObj::RegisterJNI();
	CSjvmSkinWndEventObj::RegisterJNI();

	// Compile system module files

	// Set flag to freeze the current SJVM status as system module.
	g_sjvm->FinishedInitSysModule();
}

void CSjvmSkinSystem::Quit()
{
	delete g_sjvm;
	g_sjvm = NULL;
}

void CSjvmSkinSystem::LoadScript(LPCTSTR szScript)
{
	V_Str		vFiles;
	int			nRet;
	TCHAR		szFile[MAX_PATH];

	StrSplit(szScript, ',', vFiles);
	for (int i = 0; i < vFiles.size(); i++)
	{
		TrimStr(vFiles[i]);

		if (!m_pSkinFactory->GetResourceMgr()->GetResourcePathName(szFile, CountOf(szFile), vFiles[i].c_str()))
		{
			ERRLOG1(_T("Script files does NOT exist: %s"), vFiles[i].c_str());
			return;
		}

		vFiles[i] = szFile;
	}

	nRet = g_sjvm->CompileFiles(vFiles);
	if (nRet != ERR_OK)
	{
		ERRLOG2(_T("Failed to compile script files: %s, error: %s"), szScript, (LPCTSTR)Error2Str(nRet));
		return;
	}

	//
	// Init and run script
	//

	g_sjvm->Run();

	for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
		it != m_listSkinSystemEvent.end(); ++it)
	{
		CSjvmSkinSystemEvent *pEvent = *it;
		pEvent->onInit();
	}
}

void CSjvmSkinSystem::CloseScript()
{
	for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
		it != m_listSkinSystemEvent.end(); ++it)
	{
		CSjvmSkinSystemEvent *pEvent = *it;
		pEvent->onDestory();
	}

	m_listSkinSystemEvent.clear();
	m_listSkinWndEvent.clear();

	g_sjvm->Reset();
}

void CSjvmSkinSystem::OnSkinWndCreate(CSkinWnd *pSkinWnd)
{
	// Notify onSkinWndCreate
	for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
		it != m_listSkinSystemEvent.end(); ++it)
	{
		CSjvmSkinSystemEvent *pEvent = *it;
		pEvent->onSkinWndCreate(pSkinWnd);
	}

	// Register notify of SjvmSkinWndEvent
	for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
		it != m_listSkinWndEvent.end(); ++it)
	{
		SjvmSkinWndEvent	*pEvent = *it;
		if (_tcscmp(pEvent->m_strSkinWndName.c_str(), pSkinWnd->GetSkinWndName()) == 0)
		{
			pSkinWnd->SjvmSetNotifyEvent(pEvent);
			pEvent->AddSkinWnd(pSkinWnd);
			pEvent->onCreate(pSkinWnd);
		}
	}
}

void CSjvmSkinSystem::OnSkinWndDestory(CSkinWnd *pSkinWnd)
{
	// Notify onSkinWndDestory
	for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
		it != m_listSkinSystemEvent.end(); ++it)
	{
		CSjvmSkinSystemEvent *pEvent = *it;
		pEvent->onSkinWndDestory(pSkinWnd);
	}

	// Unregister notify of SjvmSkinWndEvent
	for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
		it != m_listSkinWndEvent.end(); ++it)
	{
		SjvmSkinWndEvent	*pEvent = *it;
		if (_tcscmp(pEvent->m_strSkinWndName.c_str(), pSkinWnd->GetSkinWndName()) == 0)
		{
			pSkinWnd->SjvmSetNotifyEvent(NULL);
			pEvent->RemoveSkinWnd(pSkinWnd);
		}
	}
}

void CSjvmSkinSystem::RegisterJNI()
{
	int		nRet;

	// Add class members definition.
	CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
	ASSERT(pClass);

	pClass->AddNativeDefaultConstructor();

	nRet = pClass->AddNativeMethod(true, "registerSkinSystemEvent", registerSkinSystemEvent, TID_BOOLEAN, CSjvmSkinSystemEventObj::m_sTypeId, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(true, "unRegisterSkinSystemEvent", unRegisterSkinSystemEvent, TID_VOID, CSjvmSkinSystemEventObj::m_sTypeId, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(true, "registerSkinWndEvent", registerSkinWndEvent, TID_BOOLEAN, TID_STRING, TID_SKIN_WND_NOTIFY_EVENT, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(true, "unRegisterSkinWndEvent", unRegisterSkinWndEvent, TID_VOID, TID_SKIN_WND_NOTIFY_EVENT, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(true, "getSkinWnd", getSkinWnd, TID_VOID, TID_SKIN_WND, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(true, "getCommandID", getCommandID, TID_INT, TID_STRING, TID_INVALID);
	ASSERT(nRet != -1);
}

bool SJNI_API CSjvmSkinSystem::registerSkinSystemEvent(jobject event)
{
	ASSERT(!(event == JOBJ_NULL));
	if (event == JOBJ_NULL)
		return false;

	CSjvmSkinSystemEvent	*pEvent = new CSjvmSkinSystemEvent();

	g_sjvm->JniAddJObjRef(event);
	pEvent->m_jobject = event;
	m_listSkinSystemEvent.push_back(pEvent);

	return true;
}

void SJNI_API CSjvmSkinSystem::unRegisterSkinSystemEvent(jobject event)
{
	// Is it exist ?
	for (ListSkinSystemEvent::iterator it = m_listSkinSystemEvent.begin(); 
		it != m_listSkinSystemEvent.end(); ++it)
	{
		CSjvmSkinSystemEvent *pEvent = *it;
		if (pEvent->m_jobject == event)
		{
			g_sjvm->JniReleaseJObjRef(event);
			m_listSkinSystemEvent.erase(it);
			delete pEvent;
			return;
		}
	}

	ERRLOG1(_T("SimpleJVM:: Not found registered SjvmSkinSystemEvent: %d"), event);
}

bool SJNI_API CSjvmSkinSystem::registerSkinWndEvent(LPCTSTR szWndName, jobject event)
{
	ASSERT(!(szWndName == NULL || event == JOBJ_NULL));
	if (szWndName == NULL || event == JOBJ_NULL)
		return false;

	SjvmSkinWndEvent	*pEvent = new SjvmSkinWndEvent();

	g_sjvm->JniAddJObjRef(event);

	pEvent->m_jobject = event;
	pEvent->m_strSkinWndName = szWndName;
	m_listSkinWndEvent.push_back(pEvent);

	return true;
}

void SJNI_API CSjvmSkinSystem::unRegisterSkinWndEvent(jobject event)
{
	// Is it exist ?
	for (ListSjvmSkinWndEvent::iterator it = m_listSkinWndEvent.begin();
		it != m_listSkinWndEvent.end(); ++it)
	{
		SjvmSkinWndEvent	*pEvent = *it;
		if (pEvent->m_jobject == event)
		{
			g_sjvm->JniReleaseJObjRef(event);
			m_listSkinWndEvent.erase(it);
			delete pEvent;
			return;
		}
	}

	ERRLOG1(_T("SimpleJVM:: Not found registered SjvmSkinWndEvent: %d"), event);
}

jobject SJNI_API CSjvmSkinSystem::getSkinWnd(LPCTSTR skinWndName)
{
	ASSERT(m_pSkinFactory);
	CSkinWnd *pWnd = m_pSkinFactory->FindSkinWnd(skinWndName);
	if (!pWnd)
		return JOBJ_NULL;

	jobject		jobj;
	CSjvmSkinWnd *pJObj = (CSjvmSkinWnd*)g_sjvm->NewJObject(TID_SKIN_WND, jobj);
	ASSERT(pJObj && pJObj->m_typeId == CSjvmSkinWnd::m_sTypeId);

	pJObj->m_pSkinWnd = pWnd;

	return jobj;
}

int SJNI_API CSjvmSkinSystem::getCommandID(LPCTSTR string)
{
	ASSERT(m_pSkinFactory);

	return m_pSkinFactory->GetIDByName(string);
}

