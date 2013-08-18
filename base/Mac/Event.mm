// Event.cpp: implementation of the Event class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Event.h"
#import "base.h"
#import "misc.h"
#import "Thread.h"
#import <pthread.h>
#import <sys/time.h>

class _NSConditionInternal
{
public:
	_NSConditionInternal(bool isSingaled)
	{
		pthread_cond_init(&condition, NULL);
		pthread_mutex_init(&mutex, NULL);
		read_to_go = isSingaled;
	}

	pthread_cond_t	condition;
	pthread_mutex_t mutex;
	volatile bool read_to_go;

};

//////////////////////////////////////////////////////////////////////

Event::Event(bool bManualSet, bool isSingaled)
{
	m_nsCondition = new _NSConditionInternal(isSingaled);

	m_bMaualset = bManualSet;
}

Event::~Event()
{
}

bool Event::Set()
{
	pthread_mutex_lock(&m_nsCondition->mutex);
	m_nsCondition->read_to_go = true;
	pthread_cond_signal(&m_nsCondition->condition);
	pthread_mutex_unlock(&m_nsCondition->mutex);
	return true;
}

bool Event::Reset()
{
	m_nsCondition->read_to_go = false;
	return true;
}

bool Event::Acquire(uint32 nTimeOut)
{
	if (nTimeOut == 0)
		return m_nsCondition->read_to_go;

	bool ret = false;
	pthread_mutex_lock(&m_nsCondition->mutex);
	if (nTimeOut == WAIT_FOREVER)
	{
		while (!m_nsCondition->read_to_go)
			pthread_cond_wait(&m_nsCondition->condition, &m_nsCondition->mutex);
		if (!m_bMaualset) // For manual set event, need to call reset() manually.
			m_nsCondition->read_to_go = false;
		pthread_mutex_unlock(&m_nsCondition->mutex);
		ret = true;
	}
	else
	{
		struct timeval tv;
		struct timespec ts;
		gettimeofday(&tv, NULL);
		ts.tv_sec = time(NULL) + nTimeOut / 1000;
		ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (nTimeOut % 1000);
		ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
		ts.tv_nsec %= (1000 * 1000 * 1000);

		int n = 0;
		if (!m_nsCondition->read_to_go)
			n = pthread_cond_timedwait(&m_nsCondition->condition, &m_nsCondition->mutex, &ts);

		if (n == 0)
		{
			ret = true;
			if (!m_bMaualset) // For manual set event, need to call reset() manually.
				m_nsCondition->read_to_go = false;
		}
		pthread_mutex_unlock(&m_nsCondition->mutex);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CEvent)

class CTestCaseCEvent : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCEvent);
	CPPUNIT_TEST(TestAcquire);
	CPPUNIT_TEST(TestAll);
	CPPUNIT_TEST_SUITE_END();
	
protected:
	void TestAcquire()
	{
		Event e(false, false);
		
		{
			// Step 1
			uint32 start = GetTickCount();
			CThread thread;
			thread.Create(ThreadSetEvent, &e);
			ASSERT(e.Acquire());
			uint32 cost = GetTickCount() - start;
			ASSERT(cost >= 1000 && cost < 1500);
		}
		
		{
			// Step 2
			uint32 start = GetTickCount();
			CThread thread;
			thread.Create(ThreadSetEvent, &e);
			ASSERT(e.Acquire(0) == false);
			ASSERT(e.Acquire(500) == false);
			ASSERT(e.Acquire(1000));
			uint32 cost = GetTickCount() - start;
			ASSERT(cost >= 1000 && cost < 2000);
		}
		
		
		{
			// Step 3: Compete
			uint32 start = GetTickCount();
			CThread thread;
			thread.Create(ThreadSetEvent, &e);
			CThread thread2;
			thread2.Create(ThreadCompete, &e);
			
			ASSERT(e.Acquire(0) == false);
			ASSERT(e.Acquire(500) == false);
			uint32 cost = GetTickCount() - start;
			ASSERT(cost >= 500 && cost < 1000);
			thread2.Join();
			cost = GetTickCount() - start;
			ASSERT(cost >= 1000 && cost < 2000);
		}
	}
	
	static void ThreadSetEvent(void *param)
	{
		Event *e = (Event *)param;
		Sleep(1000);
		e->Set();
	}
	
	static void ThreadCompete(void *param)
	{
		Event *e = (Event *)param;
		ASSERT(e->Acquire());
	}
	
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCEvent);

#endif // _CPPUNIT_TEST

