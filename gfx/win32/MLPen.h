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

	bool IsValid() const { return m_pen != NULL; }

	bool CreateSolidPen(int nWidth, const MLColor &color);

	void Destroy() { if (m_pen) DeleteObject(m_pen); m_pen = NULL; }

	HPEN GetHandle() const { return m_pen; }

public:
	HPEN		m_pen;

};

#endif // !defined(_MLPEN_INC_)
