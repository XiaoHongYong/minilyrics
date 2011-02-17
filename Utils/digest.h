/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
 */
#ifndef _DIGEST_H
#define _DIGEST_H

#define MD5_LEN			16
#define MD5_STR_LEN		(32 + 1)

int MDString (const char *str, unsigned char digest[]);

int MDString(const char *str, int len, unsigned char digest[]);

int MDFile(LPCTSTR filename, unsigned char digest[]);

template<class TCHARDEF>
void MDPrint(const unsigned char *digest, TCHARDEF *szDigest)
{
	unsigned int i, n;
	
	for (i = 0, n = 0; i < MD5_LEN; i++, n += 2)
	{
		int	a;
		a = digest[i] / 16;
		if (a <= 9)
			szDigest[n] = a + '0';
		else
			szDigest[n] = 'A' + a - 10;
		a = digest[i] % 16;
		if (a <= 9)
			szDigest[n + 1] = a + '0';
		else
			szDigest[n + 1] = 'A' + a - 10;
	}
	szDigest[n] = '\0';
}

template<class TCHARDEF>
BOOL MD5FileStr(LPCSTR szSrcFile, TCHARDEF *szMD5)
{
	unsigned char	digest[MD5_LEN];
	szMD5[0] = '\0';

	if (!MDFile(szSrcFile, digest))
		return FALSE;
	MDPrint(digest, szMD5);
	return TRUE;
}

#ifdef WIN32
template<class TCHARDEF>
BOOL MD5FileStr(LPCWSTR szSrcFile, TCHARDEF *szMD5)
{
	unsigned char	digest[MD5_LEN];
	szMD5[0] = '\0';

	if (!MDFile(szSrcFile, digest))
		return FALSE;
	MDPrint(digest, szMD5);
	return TRUE;
}
#endif

template<class TCHARDEF>
BOOL MD5Str(LPCSTR szSource, int nLen, TCHARDEF szMD5[])
{
	unsigned char	digest[MD5_LEN];
	szMD5[0] = '\0';

	if (!MDString(szSource, nLen, digest))
		return FALSE;
	MDPrint(digest, szMD5);
	return TRUE;
}

BOOL MD5Gen16(LPCSTR szSource, char szMD5[]);

#endif //_DIGEST_H
