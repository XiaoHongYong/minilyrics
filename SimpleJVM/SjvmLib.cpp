#include "StdAfx.h"
#include "SjvmLib.h"
#include "SJVM.h"


class CSjobjUnitTest : public CJObject
{
	DECLARE_JNI_OBJ()
public:
	class CObjNewer : public CJObjectNewer
	{
	public:
		CObjNewer() : CJObjectNewer(m_sTypeId) { }
		CJObject *New() {
			return new CSjobjUnitTest();
		}

	};

	CSjobjUnitTest() : CJObject(m_sTypeId, 0) { }

	static void RegisterJNI()
	{
		int		nRet;

		g_sjvm.RegisterJObjNewer(new CObjNewer());

		// Add class members definition.
		CClass *pClass = g_sjvm.GetClass(CLASS_NAME);
		ASSERT(pClass);

		nRet = pClass->AddNativeMethod(true, "failedMessage", failedMessage, TID_VOID, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "message", messageStr, TID_VOID, TID_STRING, TID_INVALID);
		ASSERT(nRet != -1);

		nRet = pClass->AddNativeMethod(true, "message", messageInt, TID_VOID, TID_INT, TID_INVALID);
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
		MessageBox(NULL, szMsg, _T("SimpleJVM UnitTest"), MB_OK | MB_TOPMOST);
		ASSERT(0);
	}

public:

};

IMPLENENT_JNI_OBJ(CSjobjUnitTest, "UnitTest")

void InitSjvmLib()
{
	CSjobjUnitTest::RegisterClassType();

	CSjobjUnitTest::RegisterJNI();
}
