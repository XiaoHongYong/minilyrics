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

    bool Create(FUNThread function, void* lpData);
    void Destroy();
    void Suspend();
    void Resume();
    void Join();

    uint32 GetPriority() const;
    uint32 SetPriority(uint32 priority);

	bool IsRunning();

protected:
	FUNThread		m_funThead;
	void			*m_lpData;

};

#endif // !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
