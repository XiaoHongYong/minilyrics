// UIEditObject.h: interface for the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
#define AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _OUR_MFC_LIB

// typedef vector<string>	CProperties;

class CUIObject;
interface ISESkinNotify;

class CUIEditObject : public CUIObject  
{
public:
	CUIEditObject();
	virtual ~CUIEditObject();

	virtual void OnCreate();

	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnMouseMove(UINT nFlags, CPoint point);

	virtual void Draw(CRawGraph *pGraph);

	void SetContainedUIObject(CUIObject *pObj) { m_pUIObject = pObj; }

	virtual BOOL ReCalculatePos(FORMULA_VAR vars[]);

	virtual void EnumProperties(CUIObjProperties &listProperties);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual int FromXML(CSkinContainer *pContainer, SXNode *pXmlNode);

	virtual void ToXML(CXMLWriter &xmlStream);

	void SetNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

public:
	//
	// CSkinContainer API
	//
	virtual bool IsContainer() const { return m_pUIObject->IsContainer(); }
	virtual CSkinContainer *GetContainerIf() { return m_pUIObject->GetContainerIf(); }

public:
	LPCTSTR GetClassName() { return m_pUIObject->GetClassName(); }
	BOOL IsKindOf(LPCTSTR szClassName)
	{
		if (CUIObject::IsKindOf(szClassName))
			return TRUE;

		if (m_pUIObject->IsKindOf(szClassName))
			return TRUE;

		return _tcsicmp(szClassName, ms_szClassName) == 0;
	}
	static LPCTSTR ClassName() { return ms_szClassName; }
protected:
	static LPCTSTR		ms_szClassName;

protected:
	virtual void OnKillFocus();
	virtual void OnSetFocus();

protected:
	CUIObject		*m_pUIObject;
	BOOL			m_bLBtDown;
	POINT			m_ptDragOld;			// drag begin pos

	HBRUSH			m_hbrBk;

	ISESkinNotify	*m_pNotify;

};

#endif // !defined(AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
