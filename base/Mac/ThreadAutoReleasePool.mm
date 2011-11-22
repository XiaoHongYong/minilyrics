// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////

#import "stdafx.h"
#import "ThreadAutoReleasePool.h"

bool CThreadAutoReleasePool::Create(FUNThread function, void* lpData)
{
	m_funThead = function;
	m_lpData = lpData;

	return pthread_create(&m_threadID, NULL, ThreadFunction, (void *)this) == 0;
}

void *CThreadAutoReleasePool::ThreadFunction(void *lpParam)
{
	CThreadAutoReleasePool	*pThis = (CThreadAutoReleasePool *)lpParam;

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	pThis->m_funThead(pThis->m_lpData);

    [pool release];

	return NULL;
}
