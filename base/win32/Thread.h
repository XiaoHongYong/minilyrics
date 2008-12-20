// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
#define AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef void (*FUNThread)(void * lpParam);

class CThread
{
public:
	CThread();
	virtual ~CThread();

    virtual bool Create(FUNThread function, void* lpData, 
                        bool detach = false);
    virtual void Destroy();
    virtual void Suspend();
    virtual void Resume();
    virtual void Join();

    virtual uint32 GetPriority() const;
    virtual uint32 SetPriority(uint32 priority);

protected:
	FUNThread		m_funThead;
	void			*m_lpData;

protected:
	static DWORD WINAPI ThreadFunction(LPVOID lpParam);

protected:
	HANDLE			m_hThread;

};

#endif // !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
