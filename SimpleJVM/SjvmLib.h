#pragma once

void InitSjvmLib(CSJVM *sjvm);

class CSjobjException : public CJObject
{
	DECLARE_JNI_OBJ(CSjobjException)
public:
	CSjobjException() : CJObject(m_sTypeId, 0) { }

	static void RegisterJNI();

	static void SJNI_API ExceptionConstructor(jobject jObj, LPCTSTR szException);

	static jobject SJNI_API ToString(jobject jObj);

public:
	tstring			strException;

};
