/////////////////////////////////////////////////////////////////////////////
//
// wmpLyrics.cpp : Implementation of CWmpLyrics
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "wmpLyrics.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#endif


/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::CWmpLyrics
// Constructor

CWmpLyrics::CWmpLyrics()
{
    m_dwAdviseCookie = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::~CWmpLyrics
// Destructor

CWmpLyrics::~CWmpLyrics()
{
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT CWmpLyrics::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CWmpLyrics::FinalRelease()
{
    ReleaseCore();
	g_MLService.Quit();
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::SetCore
// Set WMP core interface

HRESULT CWmpLyrics::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

	g_Player.SetWMPCore(pCore);

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    // connect up the event interface
    CComPtr<IConnectionPointContainer>  spConnectionContainer;

    hr = m_spCore->QueryInterface( &spConnectionContainer );

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::ReleaseCore
// Release WMP core interfaces

void CWmpLyrics::ReleaseCore()
{
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

    if (m_spCore)
    {
        m_spCore = NULL;
    }

	g_Player.ReleaseWMPCore();
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::Create
// Create window for plugin

//#include "CPluginWindow.h"
HRESULT CWmpLyrics::Create(HWND hwndParent, HWND *phwndWindow)
{
	g_hInstance = _Module.GetResourceInstance();

	g_hwndPlayer = FindWindow("WMPlayerApp", NULL);

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return FALSE;

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start Windows Media Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	return 0;

	*phwndWindow = (HWND)1;//g_wndLyricShow.GetHWnd();

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::Destroy
// Destroy window for plugin

HRESULT CWmpLyrics::Destroy()
{
	g_MLService.Quit();

    return S_OK;
}
/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::TranslateAccelerator
// Handle TranslateAccelerator for plugin

HRESULT CWmpLyrics::TranslateAccelerator(LPMSG lpmsg)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::GetProperty
// Get plugin property

HRESULT CWmpLyrics::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    if (0 == lstrcmpiW(pwszName, PLUGIN_SEPARATEWINDOW_DEFAULTWIDTH ))
    {
        pvarProperty->vt = VT_I4;
        pvarProperty->lVal = 100;
    }
    else if (0 == lstrcmpiW(pwszName, PLUGIN_SEPARATEWINDOW_DEFAULTHEIGHT))
    {
        pvarProperty->vt = VT_I4;
        pvarProperty->lVal = 100;
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CWmpLyrics::SetProperty
// Set plugin property

HRESULT CWmpLyrics::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}

IWMPCore* CWmpLyrics::GetCore(void)
{
	return m_spCore;
}
