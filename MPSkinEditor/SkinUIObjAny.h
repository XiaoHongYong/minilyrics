// SkinUIObjAny.h: interface for the CSkinUIObjAny class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_)
#define AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinUIObjAny : public CUIObject  
{
public:
	CSkinUIObjAny();
	virtual ~CSkinUIObjAny();
public:

	virtual void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void EnumProperties(CUIObjProperties &listProperties);

	void SetClassName(LPCTSTR szClassName) { m_strClassName = szClassName; }

public:
	LPCTSTR GetClassName() { return m_strClassName.c_str(); }
	BOOL IsKindOf(LPCTSTR szClassName)
	{
		if (CUIObject::IsKindOf(szClassName))
			return TRUE;

		if (_tcsicmp(szClassName, m_strClassName.c_str()) == 0)
			return TRUE;

		return _tcsicmp(szClassName, ms_szClassName) == 0;
	}
	static LPCTSTR ClassName() { return ms_szClassName; }
protected:
	static LPCTSTR		ms_szClassName;

protected:
	tstring		m_strClassName;
	vector<tstring>		m_vProperties;

};

#endif // !defined(AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_)
