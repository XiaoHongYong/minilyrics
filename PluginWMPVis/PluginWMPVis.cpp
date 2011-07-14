/////////////////////////////////////////////////////////////////////////////
//
// PluginWMPVis.cpp : Implementation of CPluginWMPVis
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PluginWMPVis.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"


/////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::CPluginWMPVis
// Constructor

#define DBGLOG0(str)		

CPluginWMPVis::CPluginWMPVis() :
m_hwndParent(NULL),
m_clrForeground(0x0000FF),
m_nPreset(0)
{
    m_dwAdviseCookie = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::~CPluginWMPVis
// Destructor

CPluginWMPVis::~CPluginWMPVis()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis:::FinalConstruct
// Called when an effect is first loaded. Use this function to do one-time
// intializations that could fail (i.e. creating offscreen buffers) instead
// of doing this in the constructor, which cannot return an error.

HRESULT CPluginWMPVis::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis:::FinalRelease
// Called when an effect is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CPluginWMPVis::FinalRelease()
{
    ReleaseCore();
}


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::Render
// Called when an effect should render itself to the screen.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::Render(TimedLevel *pLevels, HDC hdc, RECT *prc)
{
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::MediaInfo
// Everytime new media is loaded, this method is called to pass the
// number of channels (mono/stereo), the sample rate of the media, and the
// title of the media
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::MediaInfo(LONG lChannelCount, LONG lSampleRate, BSTR bstrTitle )
{
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::GetCapabilities
// Returns the capabilities of this effect. Flags that can be returned are:
//	EFFECT_CANGOFULLSCREEN		-- effect supports full-screen rendering
//	EFFECT_HASPROPERTYPAGE		-- effect supports a property page
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::GetCapabilities(DWORD * pdwCapabilities)
{
    if (NULL == pdwCapabilities)
        return E_POINTER;

    *pdwCapabilities = EFFECT_HASPROPERTYPAGE;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::DisplayPropertyPage
// Invoked when a host wants to display the property page for the effect
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::DisplayPropertyPage(HWND hwndOwner)
{
	g_MLService.Config(hwndOwner);

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::GetTitle
// Invoked when a host wants to obtain the title of the effect
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::GetTitle(BSTR* bstrTitle)
{
    if (NULL == bstrTitle)
        return E_POINTER;

    CComBSTR bstrTemp;
    bstrTemp = "MiniLyrics / Lyrics Plugin";

    if ((!bstrTemp) || (0 == bstrTemp.Length()))
        return E_FAIL;

    *bstrTitle = bstrTemp.Detach();

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::GetPresetTitle
// Invoked when a host wants to obtain the title of the given preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::GetPresetTitle(LONG nPreset, BSTR *bstrPresetTitle)
{
    if (NULL == bstrPresetTitle)
        return E_POINTER;

    if ((nPreset < 0) || (nPreset >= PRESET_COUNT))
        return E_INVALIDARG;

    CComBSTR bstrTemp;

    switch (nPreset)
    {
    case PRESET_LYRICS:
        bstrTemp = "MiniLyrics"; 
        break;
    }

    if ((!bstrTemp) || (0 == bstrTemp.Length()))
        return E_FAIL;

    *bstrPresetTitle = bstrTemp.Detach();

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::GetPresetCount
// Invoked when a host wants to obtain the number of supported presets
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::GetPresetCount(LONG *pnPresetCount)
{
    if (NULL == pnPresetCount)
        return E_POINTER;

    *pnPresetCount = PRESET_COUNT;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::SetCurrentPreset
// Invoked when a host wants to change the index of the current preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::SetCurrentPreset(LONG nPreset)
{
    if ((nPreset < 0) || (nPreset >= PRESET_COUNT))
        return E_INVALIDARG;

    m_nPreset = nPreset;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::GetCurrentPreset
// Invoked when a host wants to obtain the index of the current preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::GetCurrentPreset(LONG *pnPreset)
{
    if (NULL == pnPreset)
        return E_POINTER;

    *pnPreset = m_nPreset;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::SetCore
// Set WMP core interface
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::SetCore(IWMPCore * pCore)
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

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::Create
// Invoked when the visualization should be initialized.
//
// If hwndParent != NULL, RenderWindowed() will be called and the visualization
// should draw into the window specified by hwndParent. This will be the
// behavior when the visualization is hosted in a window.
//
// If hwndParent == NULL, Render() will be called and the visualization
// should draw into the DC passed to Render(). This will be the behavior when
// the visualization is hosted windowless (like in a skin for example).
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::Create(HWND hwndParent)
{
    m_hwndParent = hwndParent;

	g_Player.m_hWndVis = hwndParent;

	WmpPluginInit(_Module.GetResourceInstance());

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::Destroy
// Invoked when the visualization should be released.
//
// Any resources allocated for rendering should be released.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::Destroy()
{
	WmpPluginQuit();

	m_hwndParent = NULL;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::NotifyNewMedia
// Invoked when a new media stream begins playing.
//
// The visualization can inspect this object for properties (like name or artist)
// that might be interesting for visualization.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::NotifyNewMedia(IWMPMedia *pMedia)
{
	g_Player.OnMediaChanged();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::OnWindowMessage
// Window messages sent to the parent window.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::OnWindowMessage(UINT msg, WPARAM WParam, LPARAM LParam, LRESULT *plResultParam )
{
    // return S_OK only if the plugin has handled the window message
    // return S_FALSE to let the defWindowProc handle the message
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::RenderWindowed
// Called when an effect should render itself to the screen.
//
// The fRequiredRender flag specifies if an update is required, otherwise the
// update is optional. This allows visualizations that are fairly static (for example,
// album art visualizations) to only render when the parent window requires it,
// instead of n times a second for dynamic visualizations.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::RenderWindowed(TimedLevel *pLevels, BOOL fRequiredRender )
{
	HWND	hWndML;

	hWndML = g_MLService.GetMainWindow();
	if (GetParent(hWndML) != m_hwndParent)
	{
		if (fRequiredRender)
		{
			//
			// Update background to black if MiniLyrics window not embedded in.
			//
			HDC hdc = ::GetDC(m_hwndParent);

			if (NULL == hdc)
				return E_FAIL;

			RECT rParent = { 0 };
			::GetClientRect(m_hwndParent, &rParent);

			HBRUSH hNewBrush = ::CreateSolidBrush(0);
			::FillRect(hdc, &rParent, hNewBrush);
			DeleteObject(hNewBrush);

			::ReleaseDC(m_hwndParent, hdc);
		}

		return S_OK;
	}

	//
	// Resize MiniLyrics window to cover the Vis area.
	//
	RECT	rcML, rc;

	hWndML = g_MLService.GetMainWindow();
	GetWindowRect(hWndML, &rcML);
	ScreenToClient(m_hwndParent, (POINT*)&rcML.left);
	ScreenToClient(m_hwndParent, (POINT*)&rcML.right);
	GetClientRect(m_hwndParent, &rc);
	if (rc.left != rcML.left || rc.top != rcML.top 
		|| rc.right != rcML.right || rc.bottom != rcML.bottom)
	{
		MoveWindow(hWndML, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::ReleaseCore
// Release WMP core interfaces
//////////////////////////////////////////////////////////////////////////////
void CPluginWMPVis::ReleaseCore()
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

//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::get_foregroundColor
// Property get to retrieve the foregroundColor prop via the public interface.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::get_foregroundColor(BSTR *pVal)
{
	DBGLOG0("get_foregroundColor");
	return ColorToWz( pVal, m_clrForeground);
}


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::put_foregroundColor
// Property put to set the foregroundColor prop via the public interface.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPluginWMPVis::put_foregroundColor(BSTR newVal)
{
	DBGLOG0("put_foregroundColor");
	return WzToColor(newVal, &m_clrForeground);
}


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::WzToColor
// Helper function used to convert a string into a COLORREF.
//////////////////////////////////////////////////////////////////////////////
HRESULT CPluginWMPVis::WzToColor(const WCHAR *pwszColor, COLORREF *pcrColor)
{ 
    if (NULL == pwszColor)
    {
        //NULL color string passed in
        return E_POINTER;
    }

    if (0 == lstrlenW(pwszColor))
    {
        //Empty color string passed in
        return E_INVALIDARG;
    }

    if (NULL == pcrColor)
    {
        //NULL output color DWORD passed in
        return E_POINTER;
    }
    
    if (lstrlenW(pwszColor) != 7)
    {
        //hex color string is not of the correct length
        return E_INVALIDARG;
    }

    DWORD dwRet = 0;
    for (int i = 1; i < 7; i++)
    {
        // shift dwRet by 4
        dwRet <<= 4;
        // and add in the value of this string

        if ((pwszColor[i] >= L'0') && (pwszColor[i] <= L'9'))
        {
            dwRet += pwszColor[i] - '0';
        }
        else if ((pwszColor[i] >= L'A') && (pwszColor[i] <= L'F'))
        {
            dwRet += 10 + (pwszColor[i] - L'A');
        }
        else if ((pwszColor[i] >= L'a') && (pwszColor[i] <= L'f'))
        {
            dwRet += 10 + (pwszColor[i] - L'a');
        }
        else
        {
           //Invalid hex digit in color string
            return E_INVALIDARG;
        }
    }

    *pcrColor = SwapBytes(dwRet);

    return S_OK;
} 


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::ColorToWz
// Helper function used to convert a COLORREF to a BSTR.
//////////////////////////////////////////////////////////////////////////////
HRESULT CPluginWMPVis::ColorToWz( BSTR* pbstrColor, COLORREF crColor)
{
    _ASSERT( NULL != pbstrColor );
    _ASSERT( (crColor & 0x00FFFFFF) == crColor );

    *pbstrColor = NULL;

    WCHAR wsz[8];
    HRESULT hr  = S_OK;

    wsprintfW( wsz, L"#%06X", SwapBytes(crColor) );
    
    *pbstrColor = ::SysAllocString( wsz );

    if (!pbstrColor)
    {
        hr = E_FAIL;
    }

    return hr;
}


//////////////////////////////////////////////////////////////////////////////
// CPluginWMPVis::SwapBytes
// Used to convert between a DWORD and COLORREF.  Simply swaps the lowest 
// and 3rd order bytes.
//////////////////////////////////////////////////////////////////////////////
inline DWORD CPluginWMPVis::SwapBytes(DWORD dwRet)
{
    return ((dwRet & 0x0000FF00) | ((dwRet & 0x00FF0000) >> 16) | ((dwRet & 0x000000FF) << 16));
}

