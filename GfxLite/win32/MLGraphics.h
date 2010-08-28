#pragma once

class MLColor;
class CFontInfo;

class CMLGraphics
{
public:
	CMLGraphics();
	virtual ~CMLGraphics();

public:
	//
	// Win32 interface
	//
	virtual void Attach(HDC hdc);
	virtual HDC Detach();

	HDC GetHandle() { return m_hdc; }

	virtual void SetFont(CFontInfo *font) { };
	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen) { return false; };
	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize) { return false; };
	virtual void SetTextColor(const MLColor &color) { };
	virtual void SetBgColor(const MLColor &color) { };

protected:
	HDC				m_hdc;

};
