// Mutex.cpp: implementation of the Mutex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mutex.h"
#include <Foundation/Foundation.h>

class _NSLockInternal
{
public:
	_NSLockInternal()
	{
		lock = [NSLock alloc];
	}
	
	~_NSLockInternal()
	{
		[lock release];
	}

	NSLock	*lock;
	
};


Mutex::Mutex()
{
	nslock = new _NSLockInternal();
}

Mutex::~Mutex()
{
	delete nslock;
}

bool Mutex::Acquire(uint32 nTimeOut)
{
	if (nTimeOut == WAIT_FOREVER)
		[nslock->lock lock];
	else if (nTimeOut == 0)
		return [nslock->lock tryLock] == YES;
	else
	{
		NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: (double)nTimeOut / 1000];
		return [nslock->lock lockBeforeDate:limit] == YES;
	}
	return true;
}

void Mutex::Release()
{
	[nslock->lock unlock];
}
