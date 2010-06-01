#pragma once

class CMemGraphics :
	public CMLGraphics
{
public:
	CMemGraphics(void);
	virtual ~CMemGraphics(void);

public:
	bool Create(int cx, int cy, IMLGraphicsBase *pGraph);

	void Destroy();

protected:

};
