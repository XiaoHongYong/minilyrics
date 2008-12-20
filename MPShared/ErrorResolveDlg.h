// ErrorResolveDlg.h: interface for the CErrorResolveDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERRORRESOLVEDLG_H__866666ED_D817_4161_97F2_2B8F6EE2F989__INCLUDED_)
#define AFX_ERRORRESOLVEDLG_H__866666ED_D817_4161_97F2_2B8F6EE2F989__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void ShowErrorDlg(CBaseWnd *pWnd, int nError);

class CErrorResolveDlg : public CBaseDialog  
{
public:
	CErrorResolveDlg();
	virtual ~CErrorResolveDlg();

	BOOL OnInitDialog();

	void SetInfo(LPCTSTR szError, LPCTSTR szResolve)
	{
		m_strError = szError;
		m_strResolve = szResolve;
	}

	void OnCommand(UINT uID, UINT nNotifyCode);

protected:
	tstring			m_strError;
	tstring			m_strResolve;
	CHyperLinkCtrl	m_hlEmail;

};

#endif // !defined(AFX_ERRORRESOLVEDLG_H__866666ED_D817_4161_97F2_2B8F6EE2F989__INCLUDED_)
