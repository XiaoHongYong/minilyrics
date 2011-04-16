#pragma once

class CSkinAnimationUIObj;

extern const int DEFAULT_ANIMATION_DURATION;

enum AnimateState
{
	AS_INIT,
	AS_MOVE,
	AS_DONE
};

class CAnimationMotion
{
public:
	CAnimationMotion(int nId) : m_nId(nId) { }
	virtual ~CAnimationMotion() { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj) = 0;

	int			m_nId;

};
typedef list<CAnimationMotion *>	ListObjAnimation;


enum AnimateType {
	AT_UNKNOWN,
	AT_FADE,
	AT_MOVE,
};

enum AnimateDirection
{
	AD_LEFT,
	AD_TOP,
	AD_RIGHT,
	AD_BOTTOM
};

AnimateType AnimateTypeFromString(LPCTSTR szAnimateType);

CAnimationMotion *NewAnimate(AnimateType type, bool bHide, CUIObject *pUIObjTarget, AnimateDirection direction);

//
// Definition the animation process: start, running and stop.
//
class CSkinAnimation
{
public:
	CSkinAnimation(CSkinAnimationUIObj *pObjTarget);	// CSkinAnimationUIObj::OnAnimate() to process the animation
	CSkinAnimation(CUIObject *pObjTarget, int nAnimateId, int nDurationTime);	// CUIObject::OnAnimate() to process the animation
	CSkinAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime); // Construct a temporarily Animation
	virtual ~CSkinAnimation();

	int GetUIObjectID() const;
	CUIObject *GetUIObject() const { return m_pObjTarget; }

	bool OnAnimate();

	int GetID() const { return m_nAnimateId; }
	AnimateState GetState() const { return m_animateState; }

	// return 0 ~ 100
	int GetMotionPercent() const { return m_nMotionPercent; }
	int GetMotionTimeOffset() const { return m_nMotionTimeOffset; }

protected:
	void InitStart();

protected:
	AnimateState		m_animateState;
	DWORD				m_dwTimeBegin;
	int					m_nMotionPercent;
	int					m_nMotionTimeOffset;
	float				m_nOneMotionTime;
	int					m_nAnimateId;

	CUIObject			*m_pObjTarget;
	CAnimationMotion	*m_pMotion;

};
