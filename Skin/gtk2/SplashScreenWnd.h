// SplashScreenWnd.h: interface for the CSplashScreenWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SPLASHSCREENWND_H_)
#define _SPLASHSCREENWND_H_

#pragma once

class CSplashScreenWnd : public CBaseWnd  
{
public:
	CSplashScreenWnd();
	virtual ~CSplashScreenWnd();

public:
	bool Show(LPCTSTR szImageFile);

};

#endif // !defined(_SPLASHSCREENWND_H_)
