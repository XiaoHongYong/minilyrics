#include "stdafx.h"
#include "Utils.h"
#include "Looper.h"

CRunnableQueue::CRunnableQueue() : m_event(false, false)
{
}

void CRunnableQueue::Add(IRunnable *pCallback)
{
	m_mutex.Acquire();

	m_listRunnable.push_back(pCallback);
	m_event.Set();

	m_mutex.Release();
}

IRunnable *CRunnableQueue::Next()
{
	// Is empty? If so, reset event
	m_mutex.Acquire();
	if (m_listRunnable.empty())
		m_event.Reset();
	m_mutex.Release();

	// Wait for event
	m_event.Acquire();

	// Get one runnable.
	m_mutex.Acquire();
	ASSERT(m_listRunnable.size() > 0);
	IRunnable *pCallback = m_listRunnable.front();
	m_listRunnable.pop_front();
	m_mutex.Release();

	return pCallback;
}

//////////////////////////////////////////////////////////////////////////

CLooper::CLooper()
{
	m_bLog = true;
	m_bIdle = true;
}

void CLooper::Loop()
{
	if (m_bLog)
		DBGLOG0(_T("CLooper: Enter Loop"));

	while (true)
	{
		IRunnable *pCallback = m_queue.Next();
		if (m_bLog)
			DBGLOG1(_T("CLooper: retrieved a runnable: %x"), pCallback);

		m_bIdle = false;
		if (pCallback == NULL)
		{
			if (m_bLog)
				DBGLOG0(_T("CLooper: Leave Loop"));
			return;
		}

		pCallback->Run();

		m_bIdle = true;
		if (pCallback->m_bFreeAfterRun)
			delete pCallback;
	}
}

void CLooper::Post(IRunnable *pCallback)
{
	m_queue.Add(pCallback);
}

void CLooper::PostFreeAfterRun(IRunnable *pCallback)
{
	pCallback->m_bFreeAfterRun = true;
	m_queue.Add(pCallback);
}

void CLooper::Quit()
{
	m_queue.Add(NULL);
}

//////////////////////////////////////////////////////////////////////////

bool CLooperThread::Start()
{
	return m_thread.Create(ThreadLoop, this);
}

void CLooperThread::ThreadLoop(void *lpParam)
{
	CLooperThread *pThis = (CLooperThread *)lpParam;

	pThis->m_looper.Loop();
}
