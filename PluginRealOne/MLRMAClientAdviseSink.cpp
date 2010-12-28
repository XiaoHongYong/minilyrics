// MLRMAClientAdviseSink.cpp: implementation of the CMLRMAClientAdviseSink class.
//
//////////////////////////////////////////////////////////////////////

#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapsink.h"
#include "rmaclsnk.h"

#include "MLRMAClientAdviseSink.h"

#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLRMAClientAdviseSink::CMLRMAClientAdviseSink() : m_RefCount(0)
{

}

CMLRMAClientAdviseSink::~CMLRMAClientAdviseSink()
{

}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPosLength(UINT32	  ulPosition,
					UINT32	  ulLength)
{
	if (g_Player.m_dwLength != ulLength)
	{
		g_Player.m_dwLength = ulLength;
		if (g_MLService.IsValid())		
			g_Player.OnMediaChanged();
	}
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPresentationOpened()
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPresentationClosed()
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnStatisticsChanged()
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPreSeek(ULONG32		    ulOldTime,
				ULONG32		    ulNewTime)
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPostSeek(ULONG32		    ulOldTime,
				ULONG32		    ulNewTime)
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnStop()
{
	g_Player.m_playerState = PS_STOPED;
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnPause(ULONG32		    ulTime)
{
	g_Player.m_playerState = PS_PAUSED;
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnBegin(ULONG32 ulTime)
{
	if (g_MLService.IsValid())		
		g_Player.OnMediaChanged();

	g_Player.m_playerState = PS_PLAYING;
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnBuffering(ULONG32		    ulFlags,
				UINT16		    unPercentComplete)
{
	return PNR_OK;
}

STDMETHODIMP
CMLRMAClientAdviseSink::OnContacting(const char*	pHostName)
{
	return PNR_OK;
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
CMLRMAClientAdviseSink::AddRef(void)
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
CMLRMAClientAdviseSink::Release(void)
{
	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	delete this;
	return 0;
}


/****************************************************************************
 *  IUnknown::QueryInterface                                    ref:  pncom.h
 *
 *  This routine indicates which interfaces this object supports. If a given
 *  interface is supported, the object's reference count is incremented, and
 *  a reference to that interface is returned. Otherwise a NULL object and
 *  error code are returned. This method is called by other objects to
 *  discover the functionality of this object.
 */
STDMETHODIMP
CMLRMAClientAdviseSink::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IRMAClientAdviseSink*)this;
		return PNR_OK;
	}

	// IRMAPlugin interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAClientAdviseSink))
	{
		AddRef();
		*ppInterfaceObj = (IRMAClientAdviseSink*)this;
		return PNR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return PNR_NOINTERFACE;
}

