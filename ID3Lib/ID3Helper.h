// ID3Helper.h: interface for the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_)
#define AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Append text included 0x0 and bom(for ucs2) to buffer.
#define AppendAnsiStr(buff, strAnsi)	buff.append(strAnsi.c_str(), (strAnsi.size() + 1) * sizeof(char))
// void AppendUcs2Str(CCharBuffer &buff, wstringex &strUcs2);
void AppendUcs2Str(CCharBuffer &buff, ID3v2EncType encType, LPCWSTR str, int nLen);

// return the size of bytes used from buffer data
int CopyAnsiStr(string &str, const char *data, int nLen);
int CopyAnsiStr(stringex &str, const char *data, int nLen);
int CopyUcs2StrByBom(wstringex &str, ID3v2EncType encType, const byte *wtext, int len);
int CopyUcs2StrByBom(wstring_t &str, ID3v2EncType encType, const BYTE *wtext, int len);

uint32 SynchDataToUInt(byte *byData, int nLen);
void SynchDataFromUInt(uint32 nValue, byte *byData, int nLen);

uint32 ByteDataToUInt(byte *byData, int nLen);
void ByteDataFromUInt(uint32 nValue, byte *byData, int nLen);

int FileDataReset(FILE *m_fp, int nOffset, int nSize, int nMask);
int FileDataCmp(FILE *m_fp, int nOffset, LPCVOID lpData, int nSize, int &nResult);
int FileAppendData(FILE *m_fp, int nAppend, int nMask = 0);
int FileMoveEndData(FILE *m_fp, int nOffset, int nNewOffset);

bool IsUnsynchLyricsFrameID(UINT nFrameID);

bool IsSynchLyricsFrameID(UINT nFrameID);

#endif // !defined(AFX_HELPERFUN_H__0EF9ABC8_88D8_4CFE_8F2C_3FA90B7AC30C__INCLUDED_)
