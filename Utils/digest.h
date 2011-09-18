#pragma once

#define MD5_LEN			16
#define MD5_STR_LEN		(32 + 1)

void MD5ToBinary(LPCVOID input, int len, unsigned char digest[]);

void MD5ToString(LPCVOID input, int len, TCHAR szDigest[]);

void MD5ToString16(LPCSTR szSource, TCHAR szMD5[]);

inline void MD5ToBinary(LPCTSTR str, unsigned char digest[])
{
	MD5ToBinary(str, _tcslen(str) * sizeof(TCHAR), digest);
}

bool MD5FileToBinary(LPCTSTR filename, unsigned char digest[]);

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
