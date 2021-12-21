// PrivateProfileString.cpp
//
//////////////////////////////////////////////////////////////////////

DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	FILE		*fp;
	TCHAR		szBuff[1024];
	int			nAppName;
	int			nKeyName;
	BOOL		bFoundAppName = FALSE;
	int			n;

	fp = _tfopen(lpFileName, _T("r"));
	if (!fp)
		goto R_FAILED;

	nAppName = _tcslen(lpAppName);
	nKeyName = _tcslen(lpKeyName);

	//
	// 查找 AppName
	while (_fgetts(szBuff, CountOf(szBuff), fp))
	{
		LPCTSTR		szBeg = szBuff;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '[')
			continue;
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (_tcsnicmp(lpAppName, szBeg, nAppName) != 0)
			continue;
		szBeg += nAppName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != ']')
			continue;
		szBeg++;

		bFoundAppName = TRUE;

		break;
	}

	if (!bFoundAppName)
		goto R_FAILED;

	//
	// 查找 KeyName
	while (_fgetts(szBuff, CountOf(szBuff), fp))
	{
		LPCTSTR		szBeg = szBuff;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg == '[')
			goto R_FAILED;

		if (_tcsnicmp(lpKeyName, szBeg, nKeyName) != 0)
			continue;
		szBeg += nKeyName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '=')
			continue;
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		_tcscpy_safe(lpReturnedString, nSize, szBeg);

		n = _tcslen(lpReturnedString);

		while (n > 0 && (lpReturnedString[n - 1] == '\r' || lpReturnedString[n - 1] == '\n'))
			n--;

		lpReturnedString[n] = '\0';

		if (fp)
			fclose(fp);

		return n;
	}

R_FAILED:
	if (fp)
		fclose(fp);

	_tcscpy_safe(lpReturnedString, nSize, lpDefault);

	return _tcslen(lpReturnedString);
}


UINT GetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nDefault, LPCTSTR lpFileName)
{
	TCHAR	szValue[64];
	TCHAR	szDefault[64];
	_stprintf(szDefault, _T("%d"), nDefault);

	GetPrivateProfileString(lpAppName, lpKeyName, szDefault, szValue, 64, lpFileName);

	return _ttoi(szValue);
}


BOOL ReplacePrivateProfileStr(CCharBuffer	&buff, const char *lpAppName, const char *lpKeyName, const char *lpString)
{
	const char	*szBeg;
	int			nAppName;
	int			nKeyName;
	BOOL		bFoundAppName = FALSE;
	char	szBuff[512];

	nAppName = strlen(lpAppName);
	nKeyName = strlen(lpKeyName);

	szBeg = buff.c_str();

	for (szBeg = buff.c_str(); szBeg != NULL; szBeg = strchr(szBeg, '\n'))
	{
		while (*szBeg == '\r' || *szBeg == '\n')
			szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '[')
			continue;
		szBeg++;

		if (_strnicmp(lpAppName, szBeg, nAppName) != 0)
			continue;
		szBeg += nAppName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != ']')
			continue;
		szBeg++;

		bFoundAppName = TRUE;

		break;
	}

	if (!bFoundAppName)
	{
		if (buff.size())
		{
			if (buff[int(buff.size() -1)] != '\n')
			{
				buff.append("\r\n", 2);
			}
		}
		// Append at end.
		sprintf(szBuff, "[%s]\r\n", lpAppName);
		buff += szBuff;
		sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
		buff += szBuff;

		return TRUE;
	}

	//
	// 查找 KeyName
	for (; szBeg != NULL; szBeg = strchr(szBeg, '\n'))
	{
		szBeg = strchr(szBeg, '\n');
		while (*szBeg == '\r' || *szBeg == '\n')
			szBeg++;

		const char*		szInsert = szBeg;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg == '[')
		{
			sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
			buff.insert(int(szInsert - buff.c_str()), szBuff, strlen(szBuff));
			return TRUE;
		}

		if (_strnicmp(lpKeyName, szBeg, nKeyName) != 0)
			continue;
		szBeg += nKeyName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '=')
			continue;
		szBeg++;
		szInsert = szBeg;

		while (*szBeg && *szBeg != '\r' && *szBeg != '\n')
			szBeg++;

		buff.replace(int(szInsert - buff.c_str()), int(szBeg - szInsert), lpString, strlen(lpString));

		return TRUE;
	}

	// the key name doesn't exist in this section.
	if (buff[int(buff.size() -1)] != '\n')
	{
		buff.append("\r\n", 2);
	}
	sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
	buff += szBuff;

	return TRUE;
}

BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	// Unicode isn't being supported.
	CCharBuffer		buff;

	if (IsFileExist(lpFileName) && !OpenFileToBuffer(lpFileName, buff))
		return FALSE;

	stringex	strAppName, strKeyname, strString;

	ConvertStr(lpAppName, -1, strAppName);
	ConvertStr(lpKeyName, -1, strKeyname);
	ConvertStr(lpString, -1, strString);

	ReplacePrivateProfileStr(buff, strAppName.c_str(), strKeyname.c_str(), strString.c_str());

	return SaveDataAsFile(lpFileName, buff.c_str(), buff.size());
}
