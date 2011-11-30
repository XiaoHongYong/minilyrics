#include "StdAfx.h"
#include "SJVM.h"
#include "SjvmLib.h"

//////////////////////////////////////////////////////////////////////////

class CSjobjUnitTest : public CJObject
{
	DECLARE_JNI_OBJ(CSjobjUnitTest)
public:
	CSjobjUnitTest() : CJObject(m_sTypeId, 0) { }

	static void RegisterJNI()
	{
		int		nRet;

		// Add class members definition.
		CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
		ASSERT(pClass);

		nRet = pClass->AddNativeMethod(true, "failedMessage", (LPVOID)failedMessage, TID_VOID, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "message", (LPVOID)messageStr, TID_VOID, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "message", (LPVOID)messageInt, TID_VOID, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "getSp", (LPVOID)getSp, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "getBp", (LPVOID)getBp, TID_INT, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "printJVMStatus", (LPVOID)printJVMStatus, TID_VOID, TID_INVALID);
		ASSERT(nRet != -1);
	}

	static void SJNI_API messageStr(LPCTSTR szMsg)
	{
		_tprintf(_T("%s\n"), szMsg);
	}

	static void SJNI_API messageInt(int nValue)
	{
		_tprintf(_T("%d\n"), nValue);
	}

	static void SJNI_API failedMessage(LPCTSTR szMsg)
	{
#ifdef _WIN32
		MessageBox(NULL, szMsg, _T("SimpleJVM UnitTest"), MB_OK | MB_TOPMOST);
		ASSERT(0);
#else
		_tprintf(_T("%s\n"), szMsg);
#endif
	}

	static int SJNI_API getSp()
	{
		return (int)g_sjvm->m_stack.size();
	}

	static int SJNI_API getBp()
	{
		return (int)g_sjvm->m_bp;
	}

	static void SJNI_API printJVMStatus()
	{
		_tprintf(_T(">>>SimpleJVM internal status:\n"));
		_tprintf(_T("BP: %d, SP: %d,	Static variable count: %d,	\n"), (int)g_sjvm->m_bp, (int)g_sjvm->m_stack.size(), (int)g_sjvm->m_vStaticVars.size());
		_tprintf(_T("JObject manager size: %d,	Free: %d\n"), (int)g_sjvm->m_jobjMgr.m_vJObjects.size(), (int)g_sjvm->m_jobjMgr.m_vFreeJObjs.size());
		_tprintf(_T("<<<End of status:\n"));
	}

public:

};

IMPLENENT_JNI_OBJ(CSjobjUnitTest, "UnitTest")


IMPLENENT_JNI_OBJ(CSjobjException, "Exception")

void CSjobjException::RegisterJNI()
{
	int		nRet;

	// Add class members definition.
	CClass *pClass = g_sjvm->GetClass(CLASS_NAME);
	ASSERT(pClass);

	nRet = pClass->AddNativeMethod(false, "Exception", (LPVOID)ExceptionConstructor, TID_VOID, TID_STRING, TID_INVALID);
	ASSERT(nRet != -1);

	nRet = pClass->AddNativeMethod(false, "toString", (LPVOID)ToString, TID_STRING, TID_INVALID);
	ASSERT(nRet != -1);
}

void SJNI_API CSjobjException::ExceptionConstructor(jobject jObj, LPCTSTR szException)
{
	CSjobjException	*pException = (CSjobjException *)g_sjvm->GetJObject(jObj);
	ASSERT(pException->m_typeId == m_sTypeId || g_sjvm->GetClassTable().IsSubClassOf(pException->m_typeId, m_sTypeId));
	pException->strException = szException;
}

jobject SJNI_API CSjobjException::ToString(jobject jObj)
{
	CSjobjException	*pException = (CSjobjException *)g_sjvm->GetJObject(jObj);
	ASSERT(pException->m_typeId == m_sTypeId || g_sjvm->GetClassTable().IsSubClassOf(pException->m_typeId, m_sTypeId));
	return g_sjvm->NewString(pException->strException.c_str(), true);
}

void InitSjvmLib(CSJVM *sjvm)
{
	CSjobjUnitTest::RegisterClassType(sjvm);
	CSjobjException::RegisterClassType(sjvm);

	CSjobjUnitTest::RegisterJNI();
	CSjobjException::RegisterJNI();
}
