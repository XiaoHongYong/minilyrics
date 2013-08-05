// Event.cpp: implementation of the Event class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Event.h"
#import <pthread.h>
#import <sys/time.h>

class _NSConditionInternal
{
public:
	_NSConditionInternal()
	{
		_condition = [NSCondition alloc];
		pthread_cond_init(&condition, NULL);
		pthread_mutex_init(&mutex, NULL);
		read_to_go = false;
	}

	NSCondition *_condition;
	pthread_cond_t	condition;
	pthread_mutex_t mutex;
	bool read_to_go;

};

//////////////////////////////////////////////////////////////////////

Event::Event(bool bManualSet, bool bInitialStat)
{
	m_nsCondition = new _NSConditionInternal();

	m_bMaualset = bManualSet;
	if (!bInitialStat)
		Reset();
}

Event::~Event()
{
}

bool Event::Set()
{
//	[m_nsCondition->_condition lock];
//	[m_nsCondition->_condition signal];
//	[m_nsCondition->_condition unlock];
	pthread_mutex_lock(&m_nsCondition->mutex);
	m_nsCondition->read_to_go = true;
	pthread_cond_signal(&m_nsCondition->condition);
	pthread_mutex_unlock(&m_nsCondition->mutex);
	return true;
}

bool Event::Reset()
{
//	[m_nsCondition->_condition lock];
//	NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 0];
//	[m_nsCondition->_condition waitUntilDate:limit];
//	[m_nsCondition->_condition unlock];
	return true;
}

bool Event::Acquire(uint32 nTimeOut)
{
	bool ret = false;
	pthread_mutex_lock(&m_nsCondition->mutex);
	if (nTimeOut == 0)
	{
		ret = m_nsCondition->read_to_go;
		pthread_mutex_unlock(&m_nsCondition->mutex);
	}
	else if (nTimeOut == WAIT_FOREVER)
	{
		while (!m_nsCondition->read_to_go)
			pthread_cond_wait(&m_nsCondition->condition, &m_nsCondition->mutex);
		m_nsCondition->read_to_go = false;
		pthread_mutex_unlock(&m_nsCondition->mutex);
		ret = true;
//		if (m_bMaualset)
//			Set();
	}
	else
	{
		struct timeval tv;
		struct timespec ts;
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = 1000 * nTimeOut;
		
		int n = 0;
		while (!m_nsCondition->read_to_go)
		{
			n = pthread_cond_timedwait(&m_nsCondition->condition, &m_nsCondition->mutex, &ts);
			if (ret != 0)
				break;
		}
		if (n == 0)
		{
			ret = true;
			m_nsCondition->read_to_go = false;
//			if (m_bMaualset)
//				Set();
		}
		pthread_mutex_unlock(&m_nsCondition->mutex);
	}

	return ret;
}

