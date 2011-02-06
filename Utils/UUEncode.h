// UUEncode.h: interface for the UUEncode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_)
#define AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void encodeuu(const char *strIn, int nLen, tstring &str, bool bBase64 = true, bool bIncNewLine = false);

BOOL decodebase64(LPCTSTR szIn, int nLen, string &strOut);

BOOL decodestduu(LPCTSTR szIn, int nLen, string &strOut);


#endif // !defined(AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_)
