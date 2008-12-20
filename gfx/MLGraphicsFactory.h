#ifndef _MLGRAPHICS_FACTORY_INC_
#define _MLGRAPHICS_FACTORY_INC_

#pragma once

class CMLGraphicsFactory
{
public:
	CMLGraphicsFactory(void);
	virtual ~CMLGraphicsFactory(void);

public:
	static void SetGraphicsType(MLGRAPHICS_TYPE graphType) { m_graphicsType = graphType; }
	static MLGRAPHICS_TYPE GetGraphicsType() { return m_graphicsType; }
	static IMLGraphicsBase *CreateMLGraphics(HDC hdc, MLGRAPHICS_TYPE graphType = MGT_AUTO);
	static void ReleaseMLGraphics(IMLGraphicsBase *pGraph);

protected:
	static MLGRAPHICS_TYPE		m_graphicsType;

};
#endif // _MLGRAPHICS_FACTORY_INC_
