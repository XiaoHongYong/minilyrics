// MLGenRegV4.h: interface for the MLGenRegGenV4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLGENREGGENV4_H__3C9B9921_99D2_48B1_8540_1433F49D75F7__INCLUDED_)
#define AFX_MLGENREGGENV4_H__3C9B9921_99D2_48B1_8540_1433F49D75F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//
//BOOL MLGenRegCodeV4(const char *szName, const char *szDate, char *szRegCode, int nLicenceYear);
//
//BOOL MLVerifyRegCodeV4(const char *szName, const char *szRegCode, OUT char *szDate, OUT int &nLicenceYear);

BOOL SimpleVerifyRegCodeV4();

#endif // !defined(AFX_MLGENREGGENV4_H__3C9B9921_99D2_48B1_8540_1433F49D75F7__INCLUDED_)
