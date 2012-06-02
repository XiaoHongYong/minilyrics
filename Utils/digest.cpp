#include "stdafx.h"
#include "digest.h"
#include "md5.h"

#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final

void MD5ToBinary(LPCVOID input, int len, unsigned char digest[])
{
	ASSERT(len >= 0);

	MD_CTX context;

	MDInit (&context);
	MDUpdate (&context, (unsigned char *)input, len);
	MDFinal (digest, &context);
}

void MD5ToString(LPCVOID input, int len, TCHAR szDigest[])
{
	EmptyStr(szDigest);

	unsigned char	digest[MD5_LEN];
	MD5ToBinary((unsigned char *)input, len, digest);
	MDPrint(digest, szDigest);
}

void MD5ToString16(LPCSTR szSource, TCHAR szMD5[])
{
	unsigned char	digest[16];
	unsigned int i, n;
	EmptyStr(szMD5);

	MD5ToBinary(szSource, strlen(szSource), digest);

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
}

// Digests a file
bool MD5FileToBinary(LPCTSTR filename, unsigned char digest[])
{
	FILE *file;
	MD_CTX context;
	int len;
	unsigned char buffer[1024];
	
	if ((file = _tfopen(filename, _T("rb"))) == NULL)
		return false;
	else
	{
		MDInit (&context);
		while ((len = fread(buffer, 1, 1024, file)) > 0)
			MDUpdate(&context, buffer, len);
		MDFinal(digest, &context);
		fclose(file);
	}

	return true;
}
