#pragma once

#include "Thread.h"

class CThreadAutoReleasePool : public CThread
{
public:
    bool Create(FUNThread function, void* lpData);

protected:
	static void *ThreadFunction(void *lpParam);

};
