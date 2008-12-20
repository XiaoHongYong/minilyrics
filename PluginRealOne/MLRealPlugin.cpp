/****************************************************************************
 *
 *  $Id: MLRealPlugin.cpp,v 1.15 1997/10/21 20:42:32 christof Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 *
 *  MLRealPlugin.cpp
 *
 *  Example plug-in which illustrates basic features of RMA.
 */

/****************************************************************************
 *
 *  This is an example of a plug-in which illustrates some very fundamental
 *  features of the RealMedia Architecture (RMA). The example shows how to
 *  discover the functionality of an object, by using COM's QueryInterface
 *  method. It also shows you how to call a method from an interface
 *  obtained in this manner.
 *
 *  Example: MLRealPlugin.cpp
 *
 *  This example simply prints out "hello world" to your screen when the
 *  plug-in is loaded by the RMA core.
 */



/****************************************************************************
 * Defines
 */
#define   INITGUID     /* Interface ID's */


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

#include "MLRealPlugin.h"  /* CMLRealPlugin */
#include "MLRMAPlayerCreationSink.h"

#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

#define DBGLOG0(p)		()

BOOL	g_Inited = FALSE;

/*

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	//	g_hInst = (HINSTANCE)hInst;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_hInstance = hInst;
	}

	return TRUE;
}
*/

/****************************************************************************
 *  RMACreateInstance                                        ref:  rmaplugn.h
 *
 *  This routine creates a new instance of the CMLRealPlugin class.
 *  It is called when the RMA core application is launched.
 */
STDAPI
RMACreateInstance(IUnknown** ppMyHelloWorldObj)
{
//	DBGLOG0("RMACreateInstance");
	*ppMyHelloWorldObj = (IUnknown*)(IRMAPlugin*)new CMLRealPlugin();
	if (*ppMyHelloWorldObj != NULL)
	{
		(*ppMyHelloWorldObj)->AddRef();
		return PNR_OK;
	}
	return PNR_OUTOFMEMORY;
}


/****************************************************************************
 *  CMLRealPlugin static variables                           ref:  MLRealPlugin.h
 *
 *  These variables are passed to the RMA core to provide information about
 *  this plug-in. They are required to be static in order to remain valid
 *  for the lifetime of the plug-in.
 */
const char* CMLRealPlugin::zm_pDescription = MY_DESCRIPTION;
const char* CMLRealPlugin::zm_pCopyright   = MY_COPYRIGHT;
const char* CMLRealPlugin::zm_pMoreInfoURL = MY_MORE_INFO_URL;


/****************************************************************************
 *  CMLRealPlugin::CMLRealPlugin                             ref:  MLRealPlugin.h
 *
 *  Constructor
 */
CMLRealPlugin::CMLRealPlugin()
	: m_RefCount(0)
{
}


// IRMAPlugin Interface Methods

/****************************************************************************
 *  IRMAPlugin::GetPluginInfo                                ref:  rmaplugn.h
 *
 *  This routine returns descriptive information about the plug-in, most
 *  of which is used in the About box of the user interface. It is called
 *  when the RMA core application is launched.
 */
STDMETHODIMP
CMLRealPlugin::GetPluginInfo
(
	REF(BOOL)        bLoadMultiple,
	REF(const char*) pDescription,
	REF(const char*) pCopyright,
	REF(const char*) pMoreInfoURL,
    REF(UINT32)      versionNumber
)
{
//	DBGLOG0("GetPluginInfo");
	bLoadMultiple = FALSE;
	pDescription  = zm_pDescription;
	pCopyright	  = zm_pCopyright;
	pMoreInfoURL  = zm_pMoreInfoURL;
	versionNumber = MY_PLUGIN_VERSION;

	return PNR_OK;
}

/****************************************************************************
 *  IRMAPlugin::InitPlugin                                   ref:  rmaplugn.h
 *
 *  This routine performs initialization steps such as determining whether
 *  required interfaces are available. It is called when the RMA core
 *  application is launched.
 */
STDMETHODIMP 
CMLRealPlugin::InitPlugin(IUnknown* pRMACore)
{
//	DBGLOG0("InitPlugin");
	// Determine if the IRMAErrorMessages interface is available
	IRMAErrorMessages* pErrorMessages = NULL;
	pRMACore->QueryInterface(IID_IRMAErrorMessages, (void**)&pErrorMessages);

	// If the interface is supported
	if (pErrorMessages != NULL)
	{
		// Print out "Hello World"
		pErrorMessages->Report(PNLOG_INFO, 0, 0, "MiniLyrics for RealPlayer", 0);

		/* 
		 * Note that QueryInterface() takes care of adding a reference to the
		 * interface for us. You are however responsible for releasing the
		 * reference to the interface when you are done using it by calling
		 * the Release() routine.
		 */
		pErrorMessages->Release();
	}

	PN_RESULT res;
//	res = pRMACore->QueryInterface(IID_IRMAPlayer, 
//		(void**)&g_pRMPlayer);
//	DBGLOG1("IID_IRMAPlayer: %d", g_pRMPlayer);

	IRMAPlayerSinkControl	*g_pPlayerSinkCtrl = NULL;

	res = pRMACore->QueryInterface(IID_IRMAPlayerSinkControl, 
		(void**)&g_pPlayerSinkCtrl);
	if (g_pPlayerSinkCtrl)
	{
		CMLRMAPlayerCreationSink	*pSink;
		pSink = new CMLRMAPlayerCreationSink;
		res = g_pPlayerSinkCtrl->AddSink((IRMAPlayerCreationSink*)pSink);
		if (res != PNR_OK)
		{
			MessageBox(NULL, "AddSink IRMAPlayerCreationSink FAILED!", "MiniLyrics", MB_OK | MB_ICONWARNING);
			delete pSink;
		}
		g_pPlayerSinkCtrl->Release();
	}

	if (g_Inited)
	{
		//
		// 已经初始化过了……
	}
	else
	{
		if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
			return PNR_OK;

		g_Inited = TRUE;

		// now we can use other g_MLService functions.
		char	szMsg[256];

		wsprintf(szMsg, "Start RealPlayer plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
		g_MLService.Log(szMsg);
	}

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
CMLRealPlugin::AddRef(void)
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
CMLRealPlugin::Release(void)
{
	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	if (g_Inited)
	{
		g_MLService.Quit();
		g_Inited = FALSE;
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
CMLRealPlugin::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IRMAPlugin*)this;
		return PNR_OK;
	}

	// IRMAPlugin interface is supported
	else if (IsEqualIID(interfaceID, IID_IRMAPlugin))
	{
		AddRef();
		*ppInterfaceObj = (IRMAPlugin*)this;
		return PNR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return PNR_NOINTERFACE;
}

