/* MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */
#include "stdafx.h"
#include "digest.h"
#include "md5.h"

#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final

/* Digests a string and prints the result.
*/
int MDString (const char *str, unsigned char digest[])
{
	MD_CTX context;
	unsigned int len = strlen (str);
	
	MDInit (&context);
	MDUpdate (&context, (unsigned char *)str, len);
	MDFinal (digest, &context);
	return 1;
}

/* Digests a string and prints the result.
*/
int MDString(const char *str, int len, unsigned char digest[])
{
	MD_CTX context;
	if (len == -1)
		len = strlen(str);

	MDInit (&context);
	MDUpdate (&context, (unsigned char *)str, len);
	MDFinal (digest, &context);
	return 1;
}

/* Digests a file and prints the result.
*/
int MDFile(LPCTSTR filename, unsigned char digest[])
{
	FILE *file;
	MD_CTX context;
	int len;
	unsigned char buffer[1024];
	
	if ((file = _tfopen(filename, _T("rb"))) == NULL)
		return 0;
	else
	{
		MDInit (&context);
		while (len = fread (buffer, 1, 1024, file))
			MDUpdate (&context, buffer, len);
		MDFinal (digest, &context);
		fclose (file);
	}
	return 1;
}

BOOL MD5Gen16(LPCSTR szSource, char szMD5[])
{
	unsigned char	digest[16];
	unsigned int i, n;
	szMD5[0] = '\0';

	if (!MDString(szSource, digest))
		return FALSE;

	
	for (i = 4, n = 0; i < 12; i++, n += 2)
	{
		int	a;
		a = digest[i] / 16;
		if (a <= 9)
			szMD5[n] = a + '0';
		else
			szMD5[n] = 'A' + a - 10;
		a = digest[i] % 16;
		if (a <= 9)
			szMD5[n + 1] = a + '0';
		else
			szMD5[n + 1] = 'A' + a - 10;
	}
	szMD5[n] = '\0';

	return TRUE;
}

//BOOL MD5Gen(LPCSTR szSource, int nLen, char szMD5[])
//{
//	unsigned char	digest[16];
//	szMD5[0] = '\0';
//
//	if (!MDString(szSource, nLen, digest))
//		return FALSE;
//	MDPrint(digest, szMD5);
//	return TRUE;
//}
//
//BOOL MD5FileGen(LPCSTR szSrcFile, char szMD5[])
//{
//	unsigned char	digest[16];
//	szMD5[0] = '\0';
//
//	if (!MDFile(szSrcFile, digest))
//		return FALSE;
//	MDPrint(digest, szMD5);
//	return TRUE;
//}
