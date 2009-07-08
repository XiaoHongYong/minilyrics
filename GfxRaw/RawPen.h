// RawGraph.h: interface for the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_RAW_PEN_H_)
#define _RAW_PEN_H_

#pragma once

class CRawPen
{
public:
	CRawPen();
	virtual ~CRawPen();

	bool IsValid() const { return true; }

	bool CreateSolidPen(int nWidth, MLColor &color);

	void Destroy() { }

public:
	MLColor			m_clrPen;
	int				m_nWidth;

};

#endif // !defined(_RAW_PEN_H_)
