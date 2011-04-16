// WaitingDlg.cpp: implementation of the CWaitingDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinToggleCtrlContainer.h"

const int DEFAULT_ANIMATION_DURATION = 500;

class CObjAnimationNewer
{
public:
	virtual ~CObjAnimationNewer() {}
	virtual CAnimationMotion *NewObject(int nId) = 0;

};

template<class _ObjClass>
class _CObjAnimationNewer : public CObjAnimationNewer
{
public:
	virtual CAnimationMotion *NewObject(int nId)
	{
		return new _ObjClass(nId);
	}

};

class CAnimateMoveMotion : public CAnimationMotion
{
public:
	CAnimateMoveMotion(int nId, AnimateDirection animateDirection)
		: CAnimationMotion(nId), m_animateDirection(animateDirection) { }

protected:
	int					m_posStart;
	AnimateDirection	m_animateDirection;

};

template<class _ObjClass, AnimateDirection direction>
class _CObjAnimationMoveNewer : public CObjAnimationNewer
{
public:
	virtual CAnimationMotion *NewObject(int nId)
	{
		return new _ObjClass(nId, direction);
	}

};

class CAnimateMoveInMotion : public CAnimateMoveMotion
{
public:
	CAnimateMoveInMotion(int nId, AnimateDirection animateDirection)
		: CAnimateMoveMotion(nId, animateDirection) { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE || as == AS_DONE)
		{
			if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT)
			{
				CalculatePosLatest(pObj->m_rcObj.left, nMotionPercent);

				pObj->m_rcObj.right = m_posLatest + pObj->m_rcObj.Width();
				pObj->m_rcObj.left = m_posLatest;
			}
			else
			{
				CalculatePosLatest(pObj->m_rcObj.top, nMotionPercent);

				pObj->m_rcObj.bottom = m_posLatest + pObj->m_rcObj.Height();
				pObj->m_rcObj.top = m_posLatest;
			}

			pObj->OnSize();
		}
		else if (as == AS_INIT)
		{
			if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT)
			{
				if (m_animateDirection == AD_LEFT)
					InitStartPos(0 - pObj->m_rcObj.right, pObj->m_rcObj.left);
				else
					InitStartPos(pObj->GetSkinWnd()->GetBoundBox().Width(), pObj->m_rcObj.left);

				pObj->m_rcObj.right = m_posStart + pObj->m_rcObj.Width();
				pObj->m_rcObj.left = m_posStart;
			}
			else
			{
				if (m_animateDirection == AD_TOP)
					InitStartPos(0 - pObj->m_rcObj.bottom, pObj->m_rcObj.top);
				else
					InitStartPos(pObj->GetSkinWnd()->GetBoundBox().Height(), pObj->m_rcObj.top);

				pObj->m_rcObj.bottom = m_posStart + pObj->m_rcObj.Height();
				pObj->m_rcObj.top = m_posStart;
			}
			pObj->OnSize();
			pObj->SetVisible(true);
		}
	}

	void InitStartPos(int posStart, int posTarget)
	{
		m_posLatest = m_posStart = posStart;
		m_posTarget = posTarget;
	}

	void CalculatePosLatest(int pos, int nMotionPercent)
	{
		if (m_posLatest != pos)
			m_posTarget = pos;

		// m_posLatest = m_posStart + (m_posTarget - m_posStart) * nMotionPercent / 100;
		m_posLatest = m_posStart + (m_posTarget - m_posStart) * (nMotionPercent * nMotionPercent) / (100 * 100);
	}

	int					m_posTarget, m_posLatest;

};

class CAnimateMoveOutMotion : public CAnimateMoveMotion
{
public:
	CAnimateMoveOutMotion(int nId, AnimateDirection animateDirection)
		: CAnimateMoveMotion(nId, animateDirection) { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE)
		{
			int posTarget, posNew;
			if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT)
			{
				if (m_animateDirection == AD_LEFT)
					posTarget = -pObj->m_rcObj.Width();
				else
					posTarget = pObj->GetSkinWnd()->GetBoundBox().Width();
				posNew = CalculatePosLatest(posTarget, nMotionPercent);

				pObj->m_rcObj.right = posNew + pObj->m_rcObj.Width();
				pObj->m_rcObj.left = posNew;
			}
			else
			{
				if (m_animateDirection == AD_TOP)
					posTarget = -pObj->m_rcObj.Height();
				else
					posTarget = pObj->GetSkinWnd()->GetBoundBox().Height();
				posNew = CalculatePosLatest(posTarget, nMotionPercent);

				pObj->m_rcObj.bottom = posNew + pObj->m_rcObj.Height();
				pObj->m_rcObj.top = posNew;
			}
			pObj->OnSize();
		}
		else if (as == AS_INIT)
		{
			if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT)
				m_posStart = pObj->m_rcObj.left;
			else
				m_posStart = pObj->m_rcObj.top;
		}
		else
		{
			pObj->SetVisible(false);
			if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT)
			{
				pObj->m_rcObj.right = m_posStart + pObj->m_rcObj.Width();
				pObj->m_rcObj.left = m_posStart;
			}
			else
			{
				pObj->m_rcObj.top = m_posStart;
				pObj->m_rcObj.bottom = m_posStart + pObj->m_rcObj.Height();
			}
		}
	}

	int CalculatePosLatest(int posTarget, int nMotionPercent)
	{
		// return m_posStart + (posTarget - m_posStart) * nMotionPercent / 100;
		return m_posStart + (posTarget - m_posStart) * (nMotionPercent * nMotionPercent) / (100 * 100);
	}

};

//////////////////////////////////////////////////////////////////////////

class CAnimationFadeOutMotion : public CAnimationMotion
{
public:
	CAnimationFadeOutMotion(int nId) : CAnimationMotion(nId) { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE)
		{
			pObj->m_nOpacity = 255 - 255 * nMotionPercent / 100;
		}
		else if (as == AS_INIT)
		{
			if (pObj->IsVisible())
				pObj->m_nOpacity = 255;
		}
		else
			pObj->SetVisible(false);
	}

};

class CAnimationFadeInMotion : public CAnimationMotion
{
public:
	CAnimationFadeInMotion(int nId) : CAnimationMotion(nId) { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE)
		{
			if (m_bAnimate)
				pObj->m_nOpacity = 255 * nMotionPercent / 100;
		}
		else if (as == AS_INIT)
		{
			if (!pObj->IsVisible())
			{
				pObj->SetVisible(true);
				pObj->m_nOpacity = 0;
				m_bAnimate = true;
			}
			else
				m_bAnimate = false;
		}
		else
			pObj->m_nOpacity = 255;
	}

	bool		m_bAnimate;

};

class CAnimationToggleMotion : public CAnimationMotion
{
public:
	CAnimationToggleMotion(int nId) : CAnimationMotion(nId) { }

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_INIT)
		{
			if (!pObj->IsVisible())
			{
				CUIObject *pObjToggle = pObj->GetSkinWnd()->GetUIObjectByClassName(CSkinToggleCtrlContainer::ClassName());
				if (pObjToggle)
				{
					pObjToggle->SetProperty(_T("CmdToggle"),
						pObj->GetSkinWnd()->GetSkinFactory()->GetStringOfID(pObj->GetID()).c_str());
				}
			}
		}
	}

};

class CAnimationResizeCtrlMotion : public CAnimationMotion
{
public:
	CAnimationResizeCtrlMotion(int nId, LPCTSTR szLeft, LPCTSTR szTop, LPCTSTR szWidth, LPCTSTR szHeight)
		: CAnimationMotion(nId)
	{
		formLeft.SetFormula(szLeft);
		formTop.SetFormula(szTop);
		formWidth.SetFormula(szWidth);
		formHeight.SetFormula(szHeight);
	}

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE)
		{
			CRect	rc;
			int		nValue;
			if (!formLeft.Empty() && pObj->GetParent()->RecalculateVarValue(formLeft, nValue))
				rc.left = rcObjOrg.left + (nValue - rcObjOrg.left) * (nMotionPercent * nMotionPercent) / (100 * 100);
			else
				rc.left = rcObjOrg.left;

			if (!formTop.Empty() && pObj->GetParent()->RecalculateVarValue(formTop, nValue))
				rc.top = rcObjOrg.top + (nValue - rcObjOrg.top) * (nMotionPercent * nMotionPercent) / (100 * 100);
			else
				rc.top = rcObjOrg.top;

			if (!formWidth.Empty() && pObj->GetParent()->RecalculateVarValue(formWidth, nValue))
				rc.right = rc.left + rcObjOrg.Width() + (nValue - rcObjOrg.Width()) * (nMotionPercent * nMotionPercent) / (100 * 100);
			else
				rc.right = rcObjOrg.right;

			if (!formHeight.Empty() && pObj->GetParent()->RecalculateVarValue(formHeight, nValue))
				rc.bottom = rc.top + rcObjOrg.Height() + (nValue - rcObjOrg.Height()) * (nMotionPercent * nMotionPercent) / (100 * 100);
			else
				rc.bottom = rcObjOrg.bottom;

			pObj->m_rcObj = rc;
			pObj->OnSize();
		}
		else if (as == AS_INIT)
		{
			rcObjOrg = pObj->m_rcObj;
			if (!formLeft.Empty())
				pObj->m_formLeft.SetFormula(formLeft.GetFormula());
			if (!formTop.Empty())
				pObj->m_formTop.SetFormula(formTop.GetFormula());
			if (!formWidth.Empty())
				pObj->m_formWidth.SetFormula(formWidth.GetFormula());
			if (!formHeight.Empty())
				pObj->m_formHeight.SetFormula(formHeight.GetFormula());
			pObj->GetParent()->RecalculateUIObjSizePos(pObj);
		}
	}

	CRect		rcObjOrg;
	CFormula	formLeft, formTop, formWidth, formHeight;

};

//////////////////////////////////////////////////////////////////////////
class CAnimationResizeWndMotion : public CAnimationMotion
{
public:
	CAnimationResizeWndMotion(CSkinWnd *pSkinWnd, int nResizeToWidth, int nResizeToHeight)
		: CAnimationMotion(UID_INVALID), m_pSkin(pSkinWnd), m_nResizeToWidth(nResizeToWidth), m_nResizeToHeight(nResizeToHeight)
	{
	}

	virtual void Animate(AnimateState as, int nMotionPercent, CUIObject *pObj)
	{
		if (as == AS_MOVE || as == AS_DONE)
		{
			if (m_nResizeToWidth != 0 && m_nResizeToHeight != 0
				&& m_nWndWidthOrg != m_nResizeToWidth && m_nWndHeightOrg != m_nResizeToHeight)
			{
				CRect	&rc = m_pSkin->GetBoundBox();

				m_pSkin->MoveWindow(rc.left, rc.top, 
					m_nWndWidthOrg + (m_nResizeToWidth - m_nWndWidthOrg) * nMotionPercent / 100,
					m_nWndHeightOrg + (m_nResizeToHeight - m_nWndHeightOrg) * nMotionPercent / 100);
			}

		}
		else if (as == AS_INIT)
		{
			m_nWndWidthOrg = m_pSkin->GetBoundBox().Width();
			m_nWndHeightOrg = m_pSkin->GetBoundBox().Height();
		}

	}

protected:
	int					m_nResizeToWidth, m_nResizeToHeight;
	int					m_nWndWidthOrg, m_nWndHeightOrg;
	CSkinWnd			*m_pSkin;

};

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinAnimationUIObj, _T("Animation"))

CSkinAnimationUIObj::CSkinAnimationUIObj()
{
	m_bEnable = false;
	m_bVisible = false;
	m_nDuration = DEFAULT_ANIMATION_DURATION;
}

BOOL CSkinAnimationUIObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	CObjAnimationNewer	*pNewer = NULL;

	if (IsPropertyName(szProperty, _T("MoveFromLeft")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_LEFT>();
	else if (IsPropertyName(szProperty, _T("MoveFromRight")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_RIGHT>();
	else if (IsPropertyName(szProperty, _T("MoveFromTop")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_TOP>();
	else if (IsPropertyName(szProperty, _T("MoveFromBottom")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_BOTTOM>();
	else if (IsPropertyName(szProperty, _T("MoveToLeft")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_LEFT>();
	else if (IsPropertyName(szProperty, _T("MoveToRight")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_RIGHT>();
	else if (IsPropertyName(szProperty, _T("MoveToTop")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_TOP>();
	else if (IsPropertyName(szProperty, _T("MoveToBottom")))
		pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_BOTTOM>();
	else if (IsPropertyName(szProperty, _T("FadeOut")))
		pNewer = new _CObjAnimationNewer<CAnimationFadeOutMotion>();
	else if (IsPropertyName(szProperty, _T("FadeIn")))
		pNewer = new _CObjAnimationNewer<CAnimationFadeInMotion>();
	else if (IsPropertyName(szProperty, _T("Toggle")))
		pNewer = new _CObjAnimationNewer<CAnimationToggleMotion>();
	else if (IsPropertyName(szProperty, _T("ResizeCtrl")))
	{
		// id, left,top,width,height
		V_Str		vParams;
		StrSplit(szValue, _T(','), vParams);
		TrimStr(vParams, ' ');
		for (int i = 0; i + 5 <= (int)vParams.size(); i += 5)
		{
			int nObjId = m_pSkin->GetSkinFactory()->GetIDByName(vParams[0].c_str());

			m_listObjAnimation.push_back(
				new CAnimationResizeCtrlMotion(nObjId, vParams[1].c_str(), vParams[2].c_str(), 
				vParams[3].c_str(), vParams[4].c_str()));
		}
	}
	else if (IsPropertyName(szProperty, _T("ResizeWnd")))
	{
		int nResizeToWidth, nResizeToHeight;
		if (Scan2IntX(szValue, nResizeToWidth, nResizeToHeight))
			m_listObjAnimation.push_back(new CAnimationResizeWndMotion(m_pSkin, nResizeToWidth, nResizeToHeight));
	}
	else if (IsPropertyName(szProperty, _T("Duration")))
	{
		m_nDuration = _ttoi(szValue);
	}
	else
		return FALSE;

	if (pNewer)
	{
		V_Str		vObjects;
		StrSplit(szValue, _T(','), vObjects);
		TrimStr(vObjects, ' ');
		for (int i = 0; i < (int)vObjects.size(); i++)
		{
			if (vObjects[i].size())
				m_listObjAnimation.push_back(
					pNewer->NewObject(
						m_pSkin->GetSkinFactory()->GetIDByName(vObjects[i].c_str())
					)
				);
		}

		delete pNewer;
	}

	return TRUE;
}

void CSkinAnimationUIObj::OnAnimate(CSkinAnimation *pAnimation)
{
	for (ListObjAnimation::iterator it = m_listObjAnimation.begin(); 
		it != m_listObjAnimation.end(); ++it)
	{
		CAnimationMotion	*pMotion = *it;
		CUIObject *pObj = NULL;
		if (pMotion->m_nId != UID_INVALID)
			pObj = m_pSkin->GetUIObjectById(pMotion->m_nId);

		pMotion->Animate(pAnimation->GetState(), pAnimation->GetMotionPercent(), pObj);
	}
}

//////////////////////////////////////////////////////////////////////////


AnimateType AnimateTypeFromString(LPCTSTR szAnimateType)
{
	if (IsPropertyName(_T("move"), szAnimateType))
		return AT_MOVE;
	else if (IsPropertyName(_T("fade"), szAnimateType))
		return AT_FADE;
	else
		return AT_UNKNOWN;
}

CAnimationMotion *NewAnimate(AnimateType type, bool bHide, CUIObject *pUIObjTarget, AnimateDirection direction)
{
	if (type == AT_MOVE)
	{
		if (bHide)
			return new CAnimateMoveOutMotion(0, direction);
		else
			return new CAnimateMoveInMotion(0, direction);
	}
	else
	{
		ASSERT(type == AT_FADE);
		if (bHide)
			return new CAnimationFadeOutMotion(0);
		else
			return new CAnimationFadeInMotion(0);
	}
}

//////////////////////////////////////////////////////////////////////////

CSkinAnimation::CSkinAnimation(CSkinAnimationUIObj *pObjTarget)	// CSkinAnimationUIObj::OnAnimate() to process the animation
{
	ASSERT(pObjTarget);
	m_pObjTarget = pObjTarget;
	m_pMotion = NULL;
	m_nAnimateId = 0;
	m_nOneMotionTime = pObjTarget->GetDuration() / (float)100;

	InitStart();
}

CSkinAnimation::CSkinAnimation(CUIObject *pObjTarget, int nAnimateId, int nDurationTime)	// CUIObject::OnAnimate() to process the animation
{
	ASSERT(pObjTarget);
	m_pObjTarget = pObjTarget;
	m_pMotion = NULL;
	m_nAnimateId = nAnimateId;
	m_nOneMotionTime = nDurationTime / (float)100;

	InitStart();
}

CSkinAnimation::CSkinAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime) // Construct a temporarily Animation
{
	ASSERT(pObjTarget && pAnimation);
	m_pObjTarget = pObjTarget;
	m_pMotion = pAnimation;
	m_nAnimateId = 0;
	m_nOneMotionTime = nDurationTime / (float)100;

	InitStart();
}

CSkinAnimation::~CSkinAnimation()
{
	if (m_pMotion)
		delete m_pMotion;
}

int CSkinAnimation::GetUIObjectID() const
{
	return m_pObjTarget->GetID();
}

void CSkinAnimation::InitStart()
{
	m_dwTimeBegin = GetTickCount();
	m_nMotionPercent = 0;
	m_nMotionTimeOffset = 0;
	m_animateState = AS_INIT;
}

bool CSkinAnimation::OnAnimate()
{
	ASSERT(m_pObjTarget);

	m_nMotionTimeOffset = (GetTickCount() - m_dwTimeBegin);
	m_nMotionPercent = int(m_nMotionTimeOffset / m_nOneMotionTime);
	if ((m_nMotionPercent > 100 || m_nMotionPercent  < 0) && m_animateState != AS_INIT)
	{
		m_nMotionPercent = 100;
		m_nMotionTimeOffset = int(m_nOneMotionTime * 100);
		m_animateState = AS_DONE;
	}

	if (m_pMotion)
		m_pMotion->Animate(m_animateState, m_nMotionPercent, m_pObjTarget);
	else
		m_pObjTarget->OnAnimate(this);

	if (m_animateState == AS_INIT)
		m_animateState = AS_MOVE;

	return m_animateState != AS_DONE;
}
