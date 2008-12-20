// MLRMAPlayerCreationSink.cpp: implementation of the CMLRMAPlayerCreationSink class.
//
//////////////////////////////////////////////////////////////////////

//#define   INITGUID     /* Interface ID's */


/****************************************************************************
 * Includes
 */
#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapsink.h"
#include "rmaclsnk.h"
#include "MLRMAClientAdviseSink.h"

#include "MLRMAPlayerCreationSink.h"

#define DBGLOG0(p)		()

IRMAPlayer		*g_pRMPlayer = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLRMAPlayerCreationSink::CMLRMAPlayerCreationSink() : m_RefCount(0)
{

}

CMLRMAPlayerCreationSink::~CMLRMAPlayerCreationSink()
{

}

STDMETHODIMP
CMLRMAPlayerCreationSink::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IRMAPlayerCreationSink*)this;
		return PNR_OK;
	}

	// IRMAPlayerCreationSink interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAPlayerCreationSink))
	{
		AddRef();
		*ppInterfaceObj = (IRMAPlayerCreationSink*)this;
		return PNR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return PNR_NOINTERFACE;
}



// IUnknown COM Interface Methods

/****************************************************************************
 *  IUnknown::AddRef                                            ref:  pncom.h
 *
 *  This routine increases the object reference count in a thread safe
 *  manner. The reference count is used to manage the lifetime of an object.
 *  This method must be explicitly called by the user whenever a new
 *  reference to an object is used.
 */
STDMETHODIMP_(UINT32)
CMLRMAPlayerCreationSink::AddRef(void)
{
	return InterlockedIncrement(&m_RefCount);
}


/****************************************************************************
 *  IUnknown::Release                                           ref:  pncom.h
 *
 *  This routine decreases the object reference count in a thread safe
 *  manner, and deletes the object if no more references to it exist. It must
 *  be called explicitly by the user whenever an object is no longer needed.
 */
STDMETHODIMP_(UINT32)
CMLRMAPlayerCreationSink::Release(void)
{
	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	delete this;
	return 0;
}

/*
 * IRMAPlayerCreationSink Methods
 */

/************************************************************************
 *	Method:
 *	    IRMAPlayerCreationSink::PlayerCreated
 *	Purpose:
 *	    Notification when a new player is created
 *
 */
IRMAPlayer		*g_pRMPlayer2 = NULL;

STDMETHODIMP CMLRMAPlayerCreationSink::PlayerCreated(IRMAPlayer* pPlayer)
{
//	DBGLOG1("CMLRMAPlayerCreationSink::PlayerCreated: %d", pPlayer);
	if (!g_pRMPlayer)
	{
		CMLRMAClientAdviseSink	*pClientAdviseSink;
		pClientAdviseSink = new CMLRMAClientAdviseSink;
		g_pRMPlayer = pPlayer;
		pPlayer->AddAdviseSink(pClientAdviseSink);
	}
	else if (!g_pRMPlayer2)
	{
		CMLRMAClientAdviseSink	*pClientAdviseSink;
		pClientAdviseSink = new CMLRMAClientAdviseSink;
		g_pRMPlayer2 = pPlayer;
		pPlayer->AddAdviseSink(pClientAdviseSink);
	}

	return PNR_OK;
}

/************************************************************************
 *	Method:
 *	    IRMAPlayerCreationSink::PlayerClosed
 *	Purpose:
 *	    Notification when an exisitng player is closed
 *
 */
STDMETHODIMP CMLRMAPlayerCreationSink::PlayerClosed(IRMAPlayer* pPlayer)
{
//	DBGLOG1("CMLRMAPlayerCreationSink::PlayerClosed: %d", pPlayer);
	if (g_pRMPlayer == pPlayer)
		g_pRMPlayer = NULL;
	else if (g_pRMPlayer2 == pPlayer)
		g_pRMPlayer2 = NULL;

	return PNR_OK;
}

