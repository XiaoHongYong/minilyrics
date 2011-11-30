// SplashScreenWnd.h: interface for the CSplashScreenWnd class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CSplashScreenWnd : public CBaseWnd  
{
public:
	CSplashScreenWnd();
	virtual ~CSplashScreenWnd();

public:
	bool Show(LPCTSTR szImageFile);

};

