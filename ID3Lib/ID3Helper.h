// ID3Helper.h: interface for the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_)
#define AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_

#pragma once

DECLAR_CPPUNIT_TEST_REG(ID3_HELPER)

// Append text included 0x0 and bom(for ucs2) to buffer.
void inline AppendAnsiStr(CCharBuffer &buff, string &str) { buff.append(str.c_str(), str.size() + 1); }

// Append text included 0x0 and bom(for ucs2) to buffer.
void AppendStrByEncodingAndBom(CCharBuffer &buff, LPCTSTR str, ID3v2EncType encType, CHAR_ENCODING encoding = ED_SYSDEF);

// return the size of bytes used from buffer data
int CopyAnsiStr(string &str, const char *data, int nLen);

// return the size of bytes used from buffer data
int CopyStrByEncodingAndBom(tstringex &str, ID3v2EncType encType, const char *data, int len, CHAR_ENCODING encoding = ED_SYSDEF);
int CopyStrByEncodingAndBom(tstring &str, ID3v2EncType encType, const char *data, int len, CHAR_ENCODING encoding = ED_SYSDEF);

uint32 SynchDataToUInt(byte *byData, int nLen);
void SynchDataFromUInt(uint32 nValue, byte *byData, int nLen);

uint32 ByteDataToUInt(byte *byData, int nLen);
void ByteDataFromUInt(uint32 nValue, byte *byData, int nLen);

int FileDataReset(FILE *m_fp, int nOffset, int nSize, int nMask);
int FileDataCmp(FILE *m_fp, int nOffset, LPCVOID lpData, int nSize, int &nResult);
int FileAppendData(FILE *m_fp, int nAppend, int nMask = 0);
int FileMoveEndData(FILE *m_fp, int nOffset, int nNewOffset);

#endif // !defined(AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_)
