// Bmp.h: interface for the CBmp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MLCOLOR_INC_
#define _MLCOLOR_INC_


class MLColor
{
public:
	MLColor(COLORREF clr)
	{
		m_clr = clr;
		m_alpha = 0xFF;
	}
	MLColor()
	{
		m_clr = 0;
		m_alpha = 0xFF;
	}
	void set(int r, int g, int b)
	{
		m_clr = RGB(r, g, b);
	}
	void set(COLORREF clr)
	{
		m_clr = clr;
	}
	void get(COLORREF &clr) const
	{
		clr = m_clr;
	}
	int r() const
	{
		return GetRValue(m_clr);
	}
	int g() const
	{
		return GetGValue(m_clr);
	}
	int b() const
	{
		return GetBValue(m_clr);
	}
	int a() const { return m_alpha; }

	COLORREF get() const { return m_clr; }

	void setAlpha(byte alpha) { m_alpha = alpha; }
	byte getAlpha() const { return m_alpha; }

	bool operator==(MLColor right)
	{
		return m_clr == right.m_clr;
	}

protected:
	COLORREF		m_clr;
	byte			m_alpha;

};


inline void PreMultiplyColor(MLColor &clr, byte alpha)
{
	clr.set(clr.r() * alpha / 255, clr.g() * alpha / 255, clr.b() * alpha / 255);
}

#endif // _MLCOLOR_INC_