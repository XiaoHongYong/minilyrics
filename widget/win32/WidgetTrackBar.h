#pragma once

#include "WidgetCtrlBase.h"

class CWidgetTrackBar : public CWidgetCtrlBase
{
public:
	void SetRange(int nMin, int nMax);
	void SetLineSize(int nLine);
	void SetPageSize(int nPage);
	void SetTickFreq(int nFreq);

	void SetPos(int nPos);
	int GetPos();
	void SetTick();

};
