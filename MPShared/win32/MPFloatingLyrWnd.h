// MPFloatingLyrWnd.h: interface for the CMPFloatingLyrWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
#define AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinWnd.h"

class CMPFloatingLyrWnd : public CMPSkinWnd  
{
public:
	CMPFloatingLyrWnd();
	virtual ~CMPFloatingLyrWnd();

	int Create();

	virtual bool OnCustomCommand(int nId);

protected:
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	//
	// Special settings for different kind of winodw (Floating lyrics and Normal window).
	//
	virtual bool SettingGetTopmost();
	virtual int SettingGetOpaquePercent();
	virtual bool SettingGetClickThrough();

	virtual void SettingReverseTopmost();
	virtual void SettingSetOpaquePercent(int nPercent);
	virtual void SettingReverseClickThrough();

};

extern CMPFloatingLyrWnd		g_wndFloatingLyr;

#endif // !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
