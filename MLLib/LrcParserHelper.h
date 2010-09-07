
#pragma once

#define _IsCharBlank(ch) (ch == ' ' || ch == '	')

CHAR_ENCODING GetLyricsTextEncoding(LPBYTE &szLrc, int &nLen);


template<class _TCHAR>
bool ParseLrcTimeTag(const _TCHAR *szLine, int &nNextPos, int &nTime)
{
	const _TCHAR *szTag = szLine;

	nTime = 0;

	if (*szTag != '[')
		goto S_ERROR;

	szTag++;
	// time tag ?
	// seek to ':'
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 60 * 1000;	// nTime += atoi(szTag) * 60s * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;

	// ':'
	if (*szTag != ':' && *szTag != '.')
		goto S_ERROR;
	szTag ++;

	// number
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 1000;	// nTime += atoi(szTag) * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;

	if (*szTag == '.' || *szTag == ':')
	{
		szTag ++;
		// new version of lrc file
		while (_IsCharBlank(*szTag))
			szTag ++;

		if (isdigit(szTag[1]))
		{
			// .000 or // .00
			if (isdigit(szTag[2]))
				nTime += _xtoi(szTag);
			else
				nTime += _xtoi(szTag) * 10;
		}
		else
			nTime += _xtoi(szTag) * 100;	// .0
		if (*szTag == '-')
			szTag++;

		while (IsDigit(*szTag))
			szTag ++;
		while (_IsCharBlank(*szTag))
			szTag ++;
	}
	if (*szTag != ']')
		goto S_ERROR;
	szTag++;

	nNextPos = (int)(szTag - szLine);

	return true;

S_ERROR:
	return false;
}

template<class _TCHAR>
bool ParseLrcPropTag(const _TCHAR *szLine, int nLineLen, const string &strPropName,
					 int &nValPosBeg, int &nValPosEnd)
{
	// strPropName Can be empty, return false.
	ASSERT(strPropName.size());

	ASSERT(*szLine == '[');
	const _TCHAR *p = szLine;
	p++;

	// Ignore spaces
	while (_IsCharBlank(*p))
		p++;

	// Compare name
	if (_xcsnicmp(p, strPropName.c_str(), strPropName.size()) != 0)
		return false;
	p += strPropName.size();

	nValPosBeg = int(p - szLine);

	// find end position
	const _TCHAR *pEnd = szLine + nLineLen;
	while (pEnd >= p && *pEnd != ']')
		pEnd--;
	if (pEnd < p)
		nValPosEnd = nLineLen;
	else
		nValPosEnd = pEnd - szLine;

	return true;
}

template<class _TCHAR>
bool ParseTxtPropTag(const _TCHAR *szLine, int nLineLen, const string &strPropName,
					 int &nValPosBeg, int &nValPosEnd)
{
	// strPropName Can be empty, return false.
	if (strPropName.empty())
		return false;

	const _TCHAR *p = szLine;

	// Compare name
	if (_xcsnicmp(p, strPropName.c_str(), strPropName.size()) != 0)
		return false;
	p += strPropName.size();

	nValPosBeg = int(p - szLine);
	nValPosEnd = nLineLen;
	ASSERT(nValPosEnd >= nValPosBeg);

	while (nValPosEnd > nValPosBeg)
	{
		_TCHAR c = szLine[nValPosEnd - 1];
		if (!_IsCharBlank(c) && c != '\r' && c != '\n')
			break;
		nValPosEnd--;
	}

	return true;
}

template<class _TCHAR>
const _TCHAR * ReadLine(const _TCHAR *szText, stringex_t<_TCHAR> &strLine)
{
	ASSERT(szText);
	// Search until \n or \0
	const _TCHAR *p = szText;
	while (*p && *p != '\n' && *p != '\r')
		p++;

	if (*p == '\r' && p[1] == '\n')
		p++;

	// Put in line, and trim \r
	strLine.clear();
	strLine.append(szText, (int)(p - szText));
	while (strLine.size() && strLine[strLine.size() - 1] == '\r')
		strLine.resize(strLine.size() - 1);

	if (*p != '\n' && *p != '\r')
		return NULL;

	return ++p;
}

template<class _TCHAR>
const _TCHAR *NextLine(const _TCHAR *szLine)
{
	while (*szLine && *szLine != '\n' && *szLine != '\r')
		szLine++;

	if (*szLine == '\r' && szLine[1] == '\n')
		szLine++;

	if (*szLine == '\n' || *szLine == '\r')
	{
		szLine++;
		return szLine;
	}

	return NULL;
}

template<class _TCHAR>
const _TCHAR *IgnoreSpaces(const _TCHAR *szLine)
{
	while (_IsCharBlank(*szLine) || *szLine == '\r')
		szLine++;

	return szLine;
}

template<class _TCHAR>
bool SearchLrcPropTag(const _TCHAR *szText, const string &strPropName,
					  int &nValPosBeg, int &nValPosEnd)
{
	const _TCHAR *szLine = szText;

	while (szLine)
	{
		const _TCHAR * szNextLine = NextLine(szLine);

		szLine = IgnoreSpaces(szLine);
		if (*szLine == '[' && !IsDigit(szLine[1]))
		{
			int nLineLen;
			if (szNextLine)
				nLineLen = int(szNextLine - szLine);
			else
				nLineLen = _xcslen(szLine);
			if (ParseLrcPropTag(szLine, nLineLen, strPropName, nValPosBeg, nValPosEnd))
			{
				// Got it.
				int	nOffset = int(szLine - szText);
				nValPosBeg += nOffset;
				nValPosEnd += nOffset;
				return true;
			}
		}
		szLine = szNextLine;
	}

	return false;
}
