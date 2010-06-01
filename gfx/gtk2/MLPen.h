// MLBrush.h: interface for the CMLPen class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MLPEN_INC_
#define _MLPEN_INC_

class CMLPen
{
public:
	CMLPen();
	virtual ~CMLPen();

	bool IsValid() const { return true; }

	bool CreateSolidPen(int nWidth, MLColor &color);

	void Destroy() { }

public:
	int				m_nWidth;
	GdkLineStyle	m_LineStyle;
	GdkCapStyle		m_CapStyle;
	GdkJoinStyle	m_JionStyle;
	GdkColor		m_clr;

};

#endif // !defined(_MLPEN_INC_)
