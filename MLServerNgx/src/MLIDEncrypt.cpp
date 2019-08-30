#include "StdAfx.h"
#include "MLIDEncrypt.h"

#define ENCRYPT_INT_LEN		(sizeof(uint32) + 1)
#define ENCRYPT_CRC_EXTRA	2

LPCSTR	__MLID_Table = { "qscavhuiklopnbzwe" };
byte	__MLStr_IDTable['z' - 'a' + 1] = { 0xFF };

void MLIdEncode(byte byData[], int nLen, tstringex &str)
{
	for (int i = 0; i < nLen; i++)
	{
		uint32	c = byData[i];
		str += __MLID_Table[(c >> 4) & 0xF];
		str += __MLID_Table[c & 0xF];
	}
}

void MLIdDecode(LPCTSTR szIn, int nLen, tstringex &strOut)
{
	if (__MLStr_IDTable[0] == 0xFF)
	{
		// NOT initialized, init it.
		for (int i = 0; i < 'z' - 'a' + 1; i++)
			__MLStr_IDTable[0] = 0xFF;

		for (int i = 0; i < 16; i++)
			__MLStr_IDTable[__MLID_Table[i] - 'a'] = i;
	}

	strOut.clear();

	for (int i = 0; i < nLen; i += 2)
	{
		byte	c;
		c = __MLStr_IDTable[szIn[i] - 'a'] << 4;
		c |= __MLStr_IDTable[szIn[i + 1] - 'a'];
		strOut += (char)c;
	}
}

// byte[0] CRC, followed 4 bytes are network byte.
void MLIDEncrypt(uint32 nId, tstring &str)
{
	byte	szBuf[64];

	LittleEndianFromUInt32(nId, szBuf + 1);

	// Calculate CRC
	szBuf[0] = ENCRYPT_CRC_EXTRA;
	for (int i = 1; i < ENCRYPT_INT_LEN; i++)
		szBuf[0] += szBuf[i];

	tstringex		strBase64;
	int n = ENCRYPT_INT_LEN;
	while (n > 1 && szBuf[n - 1] == 0)
		n--;
	MLIdEncode(szBuf, n, strBase64);

	ConvertStr(strBase64.c_str(), strBase64.size(), str);
}

uint32 MLIDDecrypt(LPCTSTR szId)
{
	tstringex		strBuf;
	tstringex		strBase64;

	strBase64 = szId;
	MLIdDecode(strBase64.c_str(), strBase64.size(), strBuf);
	for (int i = strBuf.size(); i < ENCRYPT_INT_LEN; i++)
		strBuf += (TCHAR)0;

	// Verify CRC
	byte			byCrc = ENCRYPT_CRC_EXTRA;
	for (int i = 1; i < ENCRYPT_INT_LEN; i++)
		byCrc += (byte)strBuf[(uint32)i];

	// CRC wrong
	if (byCrc != (byte)strBuf[(uint32)0])
		return 0;

	return LittleEndianToUInt32((byte*)strBuf.c_str() + 1);
}


/*//
// Base 64 will include '+', '/', '=' characters, they can't be used in URL,
// So, they must be escape to alpha letters.
//
// Map table:
// 'a' > 'aa
// '+' > 'ab'
// '/' > 'ac'
// '=' > 'ad'
//


void Base64EscapeToAlpha(stringex &strBase64)
{
	char		vSrc[] = { 'a', '+', '/', '=' };
	LPCSTR		vDst[] = { "aa", "ab", "ac", "ad" };
	for (int i = 0; i < strBase64.size(); i++)
	{
		for (int k = 0; k < CountOf(vSrc); k++)
		{
			if (strBase64[i] == vSrc[k])
			{
				strBase64.replace(i, 1, vDst[k], 2);
				i++;
				break;
			}
		}
	}
}

void Base64EscapeFromAlpha(stringex &strBase64)
{
	char		vSrc[] = { 'a', 'b', 'c', 'd' };
	char		vDst[] = { 'a', '+', '/', '=' };

	for (int i = 0; i < strBase64.size(); i++)
	{
		if (strBase64[i] == 'a')
		{
			for (int k = 0; k < CountOf(vSrc); k++)
			{
				if (strBase64[i + 1] == vSrc[k])
				{
					strBase64.replace(i, 2, vDst + k, 1);
					i--;
					break;
				}
			}
		}
	}
}
*/
