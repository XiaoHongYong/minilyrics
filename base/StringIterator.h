#ifndef _STRING_ITERATOR_H_
#define _STRING_ITERATOR_H_

#pragma once

class Ucs2CharType
{
protected:
	WCHAR		szChar[2];

public:
	Ucs2CharType(LPCTSTR str)
	{
		szChar[0] = str[0];
		szChar[1] = 0;
	}
	Ucs2CharType(WCHAR ch)
	{
		szChar[0] = ch;
		szChar[1] = 0;
	}
	Ucs2CharType() { szChar[0] = 0; }

	void operator + (const char *str)
	{
		szChar[0] = str[0];
		szChar[1] = 0;
	}

	bool operator == (Ucs2CharType ch)
	{
		return szChar[0] == ch.szChar[0];
	}

	bool operator != (Ucs2CharType ch)
	{
		return szChar[0] != ch.szChar[0];
	}

	uint32 GetIntValue()
	{
		return szChar[0];
	}

	operator const WCHAR *()
	{
		return szChar;
	}

	int size() { return 1; }

};

class Utf8CharType
{
protected:
	char		szChar[4];

public:
	Utf8CharType(LPCSTR str)
	{
		set(str);
	}
	Utf8CharType(char ch)
	{
		szChar[0] = ch;
		szChar[1] = 0;
	}
	Utf8CharType() { szChar[0] = 0; }

	void operator + (const char *str)
	{
		set(str);
	}

	bool operator == (Utf8CharType ch)
	{
		for (int i = 0; szChar[i] != 0; i++)
		{
			if (szChar[i] != ch.szChar[i])
				return false;
		}
		return true;
	}

	bool operator != (Utf8CharType ch)
	{
		return !(*this == ch);
	}

	void set(const char *str)
	{
		unsigned char	firstChar = (unsigned char)(str[0]);
		int				n;

		if (firstChar < 0xc0)			n = 1;
		else if (firstChar < 0xe0)		n = 2;
		else if (firstChar < 0xf0)		n = 3;
		else							n = 1;

		for (int i = 0; i < n; i++)
			szChar[i] = str[i];
		szChar[n] = 0;
	}

	int32 GetIntValue()
	{
		uint32		nValue = 0;

		for (int i = 0; szChar[i] != 0; i++)
		{
			nValue <<= 8;
			nValue |= uint8(szChar[i]);
		}

		return nValue;
	}

	operator const char *()
	{
		return szChar;
	}

	size_t size() { return strlen(szChar); }

};

template<class _CharType, class _char_t>
class StringIterator_t
{
public:
	typedef _CharType		CharType;

	StringIterator_t(const _char_t *szText)
	{
		m_szText = szText;
		m_nPos = 0;
		m_ch = szText;
	}

	void operator = (const _char_t *szText)
	{
		m_szText = szText;
		m_nPos = 0;
		m_ch = szText;
	}

	// Is end of string
	bool IsEOS() { return m_szText[m_nPos] == 0; }

	CharType &CurChar()
	{
		return m_ch;
	}

	void operator ++()
	{
		m_nPos += m_ch.size();
		m_ch = m_szText + m_nPos;
	}

	int GetPos() { return m_nPos; }
	void SetPos(int nPos) { m_nPos = nPos; m_ch = m_szText + m_nPos; }

protected:
	const _char_t * m_szText;
	int				m_nPos;
	CharType		m_ch;

};

typedef StringIterator_t<Ucs2CharType, WCHAR>		UCS2StringIterator;
typedef StringIterator_t<Utf8CharType, char>		UTF8StringIterator;

#ifdef UNICODE
typedef UCS2StringIterator		StringIterator;
#else
typedef UTF8StringIterator		StringIterator;
#endif

/*
void Test()
{
	LPCSTR		szText = "aaaaaaaaaabb";

	UTF8StringIterator		strIterator(szText);

	for (; !strIterator.IsEOS(); ++strIterator)
	{
		UTF8StringIterator::CharType	ch = strIterator.CurChar();

		UTF8StringIterator::CharType	ch2;
		if (ch != ch2)
			ch2 = 'a';
	}
}
*/

#endif // _STRING_ITERATOR_H_
