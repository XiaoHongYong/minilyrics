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

};

#endif // !defined(_MLBURSH_INC_)
