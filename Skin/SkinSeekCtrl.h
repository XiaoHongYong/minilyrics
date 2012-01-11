// SkinSeekCtrl.h: interface for the CSkinSeekCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINSEEKCTRL_H__21EC47EA_1BCF_4363_95C9_6DD509F7E1F0__INCLUDED_)
#define AFX_SKINSEEKCTRL_H__21EC47EA_1BCF_4363_95C9_6DD509F7E1F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinSeekCtrlEventNotify : public IUIObjNotify
{
public:
	CSkinSeekCtrlEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_BEG_DRAG; }
	enum Command
	{
		C_BEG_DRAG,
		C_END_DRAG,
	};

	Command				cmd;

};

class CSkinSeekCtrl : public CUIObject, public IScrollBar
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	enum PUSH_DOWN_POS
	{
		PUSH_DOWN_NONE,			//
		PUSH_DOWN_TOPTRACK,		// 滚动条的空白位置，进行翻页操作
		PUSH_DOWN_BOTTOMTRACK,	// 滚动条的空白位置，进行翻页操作
		PUSH_DOWN_THUMB,		// 滚动条的拖动box(thumb)
	};

public:
	CSkinSeekCtrl();
	virtual ~CSkinSeekCtrl();

	void SetScrollInfo(int nMin, int nMax, int nPage = 0, int nPos = 0, int nLine = 1, bool bRedraw = true);
	int SetScrollPos(int nPos, bool bRedraw = true);

	int GetScrollPos() const { return m_nVirtualCurPos; }
	int GetPage() const { return 0; }
	int GetMin() const { return m_nVirtualMin; }
	int GetMax() const;
	int GetID() const { return m_nId; }

	void SetScrollNotify(IScrollNotify *pNofity);

	virtual bool IsEnabled() const { return m_bEnable; }
	virtual void DisableScrollBar() { }
	virtual bool HandleScrollCode(UINT nSBCode, int nPos) { return true; }

	void OnKeyDown(UINT nChar, UINT nFlags);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnMouseDrag(CPoint point);
	bool OnMouseMove(CPoint point);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	void OnVScroll(UINT nSBCode);

	void OnSize();

	void Draw(CRawGraph *pGraph);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
	PUSH_DOWN_POS GetPushDownPos(CPoint pt);

	int GetTrackRealWidth() { return m_rcObj.Width() - m_nWidthThumb; }

	int VirtualPosToObjectPos(int nVirtualPos);
	int ObjectPosToVirtualPos(int nThumbPos);

	void ThumbOnLButtonDown(UINT nFlags, CPoint point);
	void TrackOnLButtonDown(UINT nFlags, CPoint point);

	void ThumbOnMouseMove(CPoint point);

	void NotifyEvent(CSkinSeekCtrlEventNotify::Command cmd);

protected:
	// 基本设置
	int		m_nWidthEnd;		// 两边端点的宽度
	int		m_nWidthThumb;		// 拖动块的宽度

	int		m_nPosThumb;		// 滚动条的位置(上面按钮的下方 －> thumb的上方的距离)

	// 虚拟滚动位置
	int		m_nVirtualMin;		// 最小的值
	int		m_nVirtualMax;		// 最大的值
	int		m_nVirtualLine;
	int		m_nVirtualPage;
	int		m_nVirtualCurPos;		// 滚动一行的值，缺省为1

	int				m_nXCursorToThumbBeg;	// 鼠标点下位置距拖动ThumbBox 距离: point.y - m_rcObj.top - m_nHeightEnd;
	PUSH_DOWN_POS	m_PushDownPos;			// 鼠标按下的位置
	PUSH_DOWN_POS	m_CursorPosLatest;		// 鼠标按下的位置

	CSFImage		m_imgSimple;

	CSFImage		m_imgTrackNormal, m_imgTrackPlayed;
	CSFImage		m_imgThumb;
	tstring			m_strImageTrack, m_strImageThumb;

	IScrollNotify		*m_pScrollNotify;

};

#endif // !defined(AFX_SKINSEEKCTRL_H__21EC47EA_1BCF_4363_95C9_6DD509F7E1F0__INCLUDED_)
