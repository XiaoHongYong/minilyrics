#ifndef _STRING_ITERATOR_H_
#define _STRING_ITERATOR_H_

#pragma once

class UCS2StringIterator
{
public:
	typedef wchar_t		CharType;

public:
	UCS2StringIterator(LPCTSTR szText)
	{
		m_szText = szText;
		m_nPos = 0;
	}

	void Set(LPCTSTR szText) { m_szText = szText; m_nPos = 0; }

	// Is end of string
	bool IsEOS() { return m_szText[m_nPos] == 0; }

	CharType CurChar() { return m_szText[m_nPos]; }

	void operator ++()
	{
		ASSERT(m_szText[m_nPos] != 0);
		m_nPos++;
	}

	int GetPos() { return m_nPos; }
	void SetPos(int nPos) { m_nPos = nPos; }

protected:
	LPCTSTR			m_szText;
	int				m_nPos;

};


class UTF8StringIterator
{
public:
	class CharType
	{
	public:
		char		szUtf8Char[4];

	public:
		CharType(int nChar)
		{
			szUtf8Char[0] = nChar;
			szUtf8Char[1] = 0;
		}
		CharType() { }

		bool operator == (CharType ch)
		{
			for (int i = 0; szUtf8Char[i] != 0; i++)
			{
				if (szUtf8Char[i] != ch.szUtf8Char[i])
					return false;
			}
			return true;
		}

		operator int ()
		{
			int		nValue = 0;

			for (int i = 0; szUtf8Char[i] != 0; i++)
			{
				nValue <<= 8;
				nValue |= szUtf8Char[i];
			}

			return nValue;
		}

		operator const char *()
		{
			return szUtf8Char;
		}

	};

public:
	UTF8StringIterator(LPCSTR szText)
	{
		m_szText = szText;
		m_nPos = 0;
	}

	void Set(LPCSTR szText) { m_szText = szText; m_nPos = 0; }

	// Is end of string
	bool IsEOS() { return m_szText[m_nPos] == 0; }

	CharType CurChar() {
		CharType		utf8Char;
		unsigned char	firstChar = (unsigned char)(m_szText[m_nPos]);
		int				n;

		if (firstChar < 0xc0)
			n = 1;
		else if (firstChar < 0xe0)
			n = 2;
		else if (firstChar < 0xf0)
			n = 3;
		else
			n = 1;

		for (int i = 0; i < n; i++)
			utf8Char.szUtf8Char[i] = m_szText[m_nPos + i];
		utf8Char.szUtf8Char[n] = 0;

		return utf8Char;
	}

	void operator ++()
	{
		ASSERT(m_szText[m_nPos] != 0);
		unsigned char	firstChar = (unsigned char)(m_szText[m_nPos]);

		if (firstChar < 0xc0)
			m_nPos += 1;
		else if (firstChar < 0xe0)
			m_nPos += 2;
		else if (firstChar < 0xf0)
			m_nPos += 3;
		else
			m_nPos++;
	}

	int GetPos() { m_nPos; }
	void SetPos(int nPos) { m_nPos = nPos; }

protected:
	LPCSTR			m_szText;
	int				m_nPos;

};

#ifdef UNICODE
typedef UCS2StringIterator		StringIterator;
#else
typedef UTF8StringIterator		StringIterator;
#endif
// 
// void Test()
// {
// 	LPCSTR		szText = "aaaaaaaaaabb";
// 
// 	StringIterator		strIterator(szText);
// 
// 	for (; !strIterator.IsEOS(); ++strIterator)
// 	{
// 		StringIterator::CharType	ch = strIterator.CurChar();
// 	}
// }

#endif // _STRING_ITERATOR_H_
