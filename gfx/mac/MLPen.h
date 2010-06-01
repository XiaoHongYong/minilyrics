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

	bool IsValid() const { return false; }

	bool CreateSolidPen(int nWidth, MLColor &color);

	void Destroy() { }

};

#endif // !defined(_MLPEN_INC_)
