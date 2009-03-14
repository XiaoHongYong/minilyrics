// PluginMJ.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f PluginMJps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "PluginMJ.h"

#include "PluginMJ_i.c"
#include "PluginMJCtrl.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_PluginMJCtrl, CPluginMJCtrl)
END_OBJECT_MAP()

class CPluginMJApp : public CWinApp
{
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginMJApp)
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPluginMJApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CPluginMJApp, CWinApp)
	//{{AFX_MSG_MAP(CPluginMJApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPluginMJApp theApp;

BOOL CPluginMJApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance, &LIBID_PluginMJLib);
    return CWinApp::InitInstance();
}

int CPluginMJApp::ExitInstance()
{
    _Module.Term();
    return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

void RegJRMediaPlugin(HKEY hKeyParent, LPCTSTR szReg)
{
	char cRegistryPath[1024];
	strcpy (cRegistryPath, szReg);
	strcat(cRegistryPath, "MiniLyrics");
	
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Create(hKeyParent, cRegistryPath))
	{		
		// general plugin information
		DWORD nIVersion       = 1;
		DWORD nPluginMode	  = 1;
		char  strCompany[128] = {"Crintsoft.com"};
		char  strVersion[10]  = {"1.0.0"};
		char  strURL[128]     = {"http://www.crintsoft.com"};
		char  strCopyright[255] = {"Copyright (c) 2009, Crintsoft.com"};
		LPWSTR strID; StringFromCLSID(CLSID_PluginMJCtrl, &strID);
		
		reg.SetValue(_bstr_t(strID), "CLSID");
		reg.SetValue(nIVersion, "IVersion");
		reg.SetValue(strCompany, "Company");
		reg.SetValue(strVersion, "Version");
		reg.SetValue(strURL, "URL");
		reg.SetValue(strCopyright, "Copyright");
		reg.SetValue(nPluginMode, "PluginMode");
	}
}

#define REGISTRY_PATH_MC_PLUGINS_INTERFACE	"Software\\J. River\\Media Jukebox\\Plugins\\Interface\\"

STDAPI DllRegisterServer(void)
{
 	// register with Media Jukebox
	RegJRMediaPlugin(HKEY_CURRENT_USER, REGISTRY_PATH_MJ_PLUGINS_INTERFACE);
	RegJRMediaPlugin(HKEY_LOCAL_MACHINE, REGISTRY_PATH_MC_PLUGINS_INTERFACE);

	// registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER, REGISTRY_PATH_MJ_PLUGINS_INTERFACE))
	{
		reg.DeleteSubKey("MiniLyrics");
		reg.Close();
	}
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, REGISTRY_PATH_MC_PLUGINS_INTERFACE))
	{
		reg.DeleteSubKey("MiniLyrics");
		reg.Close();
	}

    return _Module.UnregisterServer(TRUE);
}


