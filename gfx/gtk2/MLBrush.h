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

	bool IsValid() const { return true; }

	bool CreateSolidBrush(const MLColor &color);

	void Destroy() { }

public:
	GdkColor		m_color;

};

#endif // !defined(_MLBURSH_INC_)
