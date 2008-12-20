// Mutex.h: interface for the Mutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_)
#define AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WAIT_FOREVER INFINITE

class Mutex  
{
public:
	Mutex();
	virtual ~Mutex();

	bool Acquire(uint32 nTimeOut = WAIT_FOREVER);
	void Release();

private:
	HANDLE m_hMutex;	

};

class CMutexAutolock
{
public:
	CMutexAutolock(Mutex &mutex) : m_pMutex(&mutex) { m_pMutex->Acquire(); }
	~CMutexAutolock() { if (m_pMutex) m_pMutex->Release(); }

	void Unlock() { if (m_pMutex) { m_pMutex->Release(); m_pMutex = NULL; } }

	Mutex		*m_pMutex;
};

#endif // !defined(AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_)
