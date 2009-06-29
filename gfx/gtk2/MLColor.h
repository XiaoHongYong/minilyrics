// MLColor.h: interface for the CBmp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MLCOLOR_INC_
#define _MLCOLOR_INC_

class MLColor
{
public:
	MLColor(COLORREF clr)
	{
		set(clr);
	}
	MLColor():red(0), green(0),blue(0)
	{
	}
	void set(int r, int g, int b)
	{
		red = r * 257;
		green = g * 257;
		blue = b * 257;
	}
	void set(COLORREF clr)
	{
		red = GetRValue(clr) * 257;
		green = GetGValue(clr) * 257;
		blue = GetBValue(clr) * 257;
	}
	void get(COLORREF &clr)
	{
		clr = RGB(red / 257, green / 257, blue / 257);
	}
	int r() const
	{
		return red / 257;
	}
	int g() const
	{
		return green / 257;
	}
	int b() const
	{
		return blue / 257;
	}

	COLORREF get() const { return RGB(red / 257, green / 257, blue / 257); }

	bool operator==(MLColor right) const
	{
		return red == right.red && green == right.green && blue == right.blue;
	}

	uint16	red;
	uint16	green;
	uint16	blue;

};

#endif // _MLCOLOR_INC_
