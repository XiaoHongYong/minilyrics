// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "android.h"
#include "Thread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread()
{
	m_funThead = NULL;
	m_lpData = NULL;
	m_threadID = 0;
}

CThread::~CThread()
{
}

bool CThread::Create(FUNThread function, void* lpData)
{
	m_funThead = function;
	m_lpData = lpData;

	return pthread_create(&m_threadID, NULL, ThreadFunction, (void *)this) == 0;
}

void CThread::Destroy()
{
	pthread_kill(m_threadID, 0);
}

void CThread::Suspend()
{
}

void CThread::Resume()
{
}

void CThread::Join()
{
	if (m_threadID)
		pthread_join(m_threadID, NULL);
}

uint32 CThread::GetPriority() const
{
	return 0;
}

uint32 CThread::SetPriority(uint32 priority)
{
	return 0;
}

bool CThread::IsRunning()
{
	return false;
}

void *CThread::ThreadFunction(void *lpParam)
{
	CThread	*pThis = (CThread *)lpParam;

	JNIEnv *env;
	if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
		// Error occurred.
	}

	pThis->m_funThead(pThis->m_lpData);
	
	g_jvm->DetachCurrentThread();

	return NULL;
}
