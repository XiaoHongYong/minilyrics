// SkinVScrollBar.h: interface for the CSkinVScrollBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINVSCROLLBAR_H__681E7823_B758_40C0_ABF3_983D2364CB5E__INCLUDED_)
#define AFX_SKINVSCROLLBAR_H__681E7823_B758_40C0_ABF3_983D2364CB5E__INCLUDED_

#include "UIObject.h"

class CSkinSrollBarBase : public CUIObject, public IScrollBar
{
public:
	CSkinSrollBarBase();
	~CSkinSrollBarBase();

public:
	enum PUSH_DOWN_POS
	{
		PUSH_DOWN_NONE,			// 没有按在任何位置
		PUSH_DOWN_TOPBT,		// 滚动条上面的按钮
		PUSH_DOWN_TOPTRACK,		// 滚动条的空白位置，进行翻页操作
		PUSH_DOWN_BOTTOMBT,		// 滚动条下面的按钮
		PUSH_DOWN_BOTTOMTRACK,	// 滚动条的空白位置，进行翻页操作
		PUSH_DOWN_THUMB,		// 滚动条的拖动box(thumb)
	};

	enum
	{
		MARGIN_THUMB	= 3
	};

	void SetScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true);
	int SetScrollPos(int nPos, bool bRedraw = true);

	int GetScrollPos() const { return m_nVirtualCurPos; }
	int GetPage() const { return m_nVirtualPage; }
	int GetMin() const { return m_nVirtualMin; }
	int GetMax() const;
	int GetID() const { return m_nId; }

	void SetScrollNotify(IScrollNotify *pNofity);

	virtual bool IsEnabled() const;
	virtual void DisableScrollBar();
	virtual bool HandleScrollCode(UINT nSBCode, int nPos) { return true; }

public:
	void OnSize(int cx, int cy);

	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	void OnTimer(int nId);

protected:
	void UnregisterTimer();
	void RegisterTimer();

	void TopBtOnLButtonUp(UINT nFlags, CPoint point);
	void BottomBtOnLButtonUp(UINT nFlags, CPoint point);
	void ThumbOnLButtonUp(UINT nFlags, CPoint point);
	void TrackOnLButtonUp(UINT nFlags, CPoint point);

	void TopBtOnLButtonDown(UINT nFlags, CPoint point);
	void BottomBtOnLButtonDown(UINT nFlags, CPoint point);
	void TrackOnLButtonDown(UINT nFlags, CPoint point);

	void TopBtOnMouseMove(UINT nFlags, CPoint point);
	void BottomBtOnMouseMove(UINT nFlags, CPoint point);
	void TrackOnMouseMove(UINT nFlags, CPoint point);

	virtual void OnScroll(UINT nSBCode) = 0;

	virtual void AdjustThumbSize() = 0;

	virtual PUSH_DOWN_POS GetPushDownPos(CPoint pt) = 0;

	virtual void ThumbOnLButtonDown(UINT nFlags, CPoint point) = 0;
	virtual void ThumbOnMouseMove(UINT nFlags, CPoint point) = 0;

	virtual int VirtualPosToObjectPos(int nVirtualPos) = 0;
	virtual int ObjectPosToVirtualPos(int nThumbPos) = 0;

protected:
	IScrollNotify	*m_pScrollNotify;

	int				m_nPosThumb;		// 滚动条的位置(上面按钮的下方 －> thumb的上方的距离)

	int				m_nLinesPerWheel;

	// 虚拟滚动位置
	int				m_nVirtualMin;		// 最小的值
	int				m_nVirtualMax;		// 最大的值
	int				m_nVirtualPage;		// 滚动一页的值
	int				m_nVirtualLine;		// 滚动一行的值，缺省为1
	int				m_nVirtualCurPos;		// 滚动一行的值，缺省为1

	int				m_nClickDelayTimerId;	// 登记的点击后延迟的TIMER的ID
	int				m_nRepeatTimerId;		// 登记的重复滚动TIMER的ID

	bool			m_bStretchedThumb;	// Can the thumb be stretched?
	int				m_nSizeThumbStretched;

	int				m_nCursorToThumbBeg;	// 鼠标点下位置距拖动ThumbBox 距离: point.y - m_rcObj.top - m_nHeightPushBt;

	PUSH_DOWN_POS	m_PushDownPos;			// 鼠标按下的位置
	PUSH_DOWN_POS	m_CursorPosLatest;		// 鼠标按下的位置

	tstring			m_strBmpFile;		// 图片

	// 上面的按钮
	CSFImage		m_imgTopBtNormal;		// 普通状态下的图片
	CSFImage		m_imgTopBtFocus;			// 焦点状态下的图片
	CSFImage		m_imgTopBtPushDown;		// 按下状态的图片

	// 下面的按钮
	CSFImage		m_imgBottomBtNormal;		// 普通状态下的图片
	CSFImage		m_imgBottomBtFocus;			// 焦点状态下的图片
	CSFImage		m_imgBottomBtPushDown;		// 按下状态的图片

	// 拖动的thumb
	CSFImage		m_imgThumbNormal;		// 普通状态下的图片
	CSFImage		m_imgThumbFocus;			// 焦点状态下的图片
	CSFImage		m_imgThumbPushDown;		// 按下状态的图片

	// 空白的轨道Track
	CSFImage		m_imgTrack;			// 滚动条拖动的轨道：）

};

class CSkinVScrollBar : public CSkinSrollBarBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	void Draw(CRawGraph *pGraph);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	CSkinVScrollBar();
	virtual ~CSkinVScrollBar();

protected:
	virtual void OnScroll(UINT nSBCode);

	virtual void AdjustThumbSize();

	virtual PUSH_DOWN_POS GetPushDownPos(CPoint pt);

	virtual void ThumbOnLButtonDown(UINT nFlags, CPoint point);
	virtual void ThumbOnMouseMove(UINT nFlags, CPoint point);

	virtual int VirtualPosToObjectPos(int nVirtualPos);
	virtual int ObjectPosToVirtualPos(int nThumbPos);

protected:
	// 基本设置
	int				m_nSBWidth;			// 滚动条的宽度
	int				m_nHeightPushBt;	// 上、下按钮的高度
	int				m_nHeightThumb;		// 拖动块的高度
	int				m_nHeightTrack;		// 拖动区的高度

};

class CSkinHScrollBar : public CSkinSrollBarBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	void Draw(CRawGraph *pGraph);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	CSkinHScrollBar();
	virtual ~CSkinHScrollBar();

protected:
	virtual void OnScroll(UINT nSBCode);

	virtual void AdjustThumbSize();

	virtual PUSH_DOWN_POS GetPushDownPos(CPoint pt);

	virtual void ThumbOnLButtonDown(UINT nFlags, CPoint point);
	virtual void ThumbOnMouseMove(UINT nFlags, CPoint point);

	virtual int VirtualPosToObjectPos(int nVirtualPos);
	virtual int ObjectPosToVirtualPos(int nThumbPos);

protected:
	// 基本设置
	int				m_nSBHeight;		// The height of scroll bar
	int				m_nWidthPushBt;		// button width of scroll bar
	int				m_nWidthThumb;		// Thumb width
	int				m_nWidthTrack;		// Track width

};

#endif // !defined(AFX_SKINVSCROLLBAR_H__681E7823_B758_40C0_ABF3_983D2364CB5E__INCLUDED_)
