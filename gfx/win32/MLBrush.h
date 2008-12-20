// MLBrush.h: interface for the CMLBrush class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MLBURSH_INC_
#define _MLBURSH_INC_

class CMLBrush
{
public:
	CMLBrush();
	virtual ~CMLBrush();

	bool IsValid() const { return m_brush != NULL; }

	bool CreateSolidBrush(const MLColor &color);

	void Destroy();

	HBRUSH GetHandle() const { return m_brush; }

	const MLColor &GetColor() { return m_color; }

public:
	HBRUSH		m_brush;
	MLColor		m_color;

};

#endif // !defined(_MLBURSH_INC_)
