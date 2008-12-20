// UUEncode.h: interface for the UUEncode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_)
#define AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void encodeuu(const char *strIn, int nLen, stringex &str, bool bBase64 = true, bool bIncNewLine = true);

BOOL decodebase64(const char *szIn, int nLen, stringex &strOut);

BOOL decodestduu(const char *szIn, int nLen, stringex &strOut);


#endif // !defined(AFX_UUENCODE_H__453AEFDC_3F0E_4933_9652_7D6722E27EBE__INCLUDED_)
