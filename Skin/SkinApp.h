#pragma once

#ifdef _WIN32
#include "win32/EventsDispatcher.h"
#endif

class CEventsDispatcher;
class CSkinFactory;

class CSkinApp : public IEventHandler
{
public:
	CSkinApp(void);
	virtual ~CSkinApp(void);

	static CSkinApp *GetInstance();

	virtual bool Init();
	virtual void Quit();

	virtual void OnEvent(const IEvent *pEvent) { }

	virtual void PostQuitMessage();

	virtual int LoadDefaultSkin(LPCTSTR szDefaultSkin, bool bCreateSkinWnd = true);

	tstring GetDefaultSkin();
	void WriteDefaultSkin(LPCTSTR szDefaultSkin);

	CEventsDispatcherBase *GetEventPatcher() { return m_pEventDispatcher; }
	CSkinFactory *GetSkinFactory() { return m_pSkinFactory; }

	int ShowDialog(CBaseWnd *pWndParent, LPCTSTR szDialogName);

protected:
	virtual CEventsDispatcher *NewEventPatcher();
	virtual CSkinFactory *NewSkinFactory();

protected:
	static Mutex				m_mutex;
	static CSkinApp				*m_pInstance;
	CEventsDispatcherBase		*m_pEventDispatcher;
	CSkinFactory				*m_pSkinFactory;

};
