// MPSkinMainWnd.h: interface for the CMPSkinMainWndBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_)
#define AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinWnd.h"

void ShowRegisterNoticeSafely();

class CMPSkinMainWndBase : public CMPSkinWnd
{
public:
	CMPSkinMainWndBase();
	virtual ~CMPSkinMainWndBase();

	virtual bool OnCreate();
	virtual void OnDestroy();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	void OnTimer(UINT nIDEvent);

protected:

};

#ifdef _WIN32
#include "../MPShared/win32/MPSkinMainWndWin32.h"
#endif

#ifdef _LINUX_GTK2
#include "../MPShared/gtk2/MPSkinMainWndGtk2.h"
#endif

#endif // !defined(AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_)
