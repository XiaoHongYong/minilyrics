#ifndef _SIMPSTR_T_H_
#define _SIMPSTR_T_H_

template<class T>
class stringex_t
{
public:
	//typedef T& reference;

#define UNIT_SIZE		1
	enum
	{
		BUFF_INC		= 1024,
	};
	stringex_t()
	{
		nSizeMax = 256;
		m_szBuff = (T *)malloc(nSizeMax * sizeof(T));
		m_szBuff[0] = '\0';
		nLen = 0;
	}
	stringex_t(const stringex_t<T> &obj)
	{
		nSizeMax = 0;
		m_szBuff = NULL;
		nLen = 0;
		*this = obj;
	}
	~stringex_t()
	{
		if (m_szBuff)
			free(m_szBuff);
	}

	void erase(int nOffset, int n)
	{
		assert(n >= 0);
		if (nOffset > (int)nLen)
			return;

		if (nOffset + n > (int)nLen)
			n = nLen - nOffset;

		T		*s1, *s2, *sEnd;

		s1 = m_szBuff + nOffset;
		s2 = s1 + n;
		sEnd = m_szBuff + nLen;

		while (s2 != sEnd)
		{
			*s1 = *s2;
			s1++;
			s2++;
		}
		*s1 = '\0';

		nLen -= n;
	}

	void clear()
	{
		nLen = 0;
		m_szBuff[0] = '\0';
	}

	void replace(int nBeg, int n1, const T*str, int n = -1)
	{
		assert(n >= 0);
		if (nBeg + n1 > (int)nLen)
			return;
		if (n < 0)
			n = _xcslen(str);

		T		*s1, *s2;
		const T	*sEnd;

		if (n1 >= n)
		{
			s1 = m_szBuff + nBeg;
			sEnd = str + n;
			while (str != sEnd)
			{
				*s1 = *str;
				s1++;
				str++;
			}

			if (n1 > n)
			{
				s2 = m_szBuff + nBeg + n1;
				sEnd = m_szBuff + nLen;
				while (s2 != sEnd)
				{
					*s1 = *s2;
					s1++;
					s2++;
				}
				*s1 = '\0';
			}
		}
		else
		{
			reserve(size() + n - n1);

			//  nBeg___n1(sEnd)____s2
			//                          s1
			//
			s1 = m_szBuff + nLen + n - n1 - 1;
			s2 = m_szBuff + nLen - 1;
			sEnd = m_szBuff + nBeg + n1 - 1;

			s1[1] = '\0';

			// move tail
			while (s2 != sEnd)
			{
				*s1 = *s2;
				s1--;
				s2--;
			}

			// insert
			s1 = m_szBuff + nBeg;
			sEnd = str + n;
			while (str != sEnd)
			{
				*s1 = *str;
				s1++;
				str++;
			}
		}
		nLen += n - n1;
	}

	void insert(int nOffset, const T*str, int n)
	{
		assert(n >= 0);
		if (nOffset > (int)nLen || n == 0)
			return;

		reserve(size() + n);

		T		*s1, *s2, *sEnd;

		s1 = m_szBuff + nOffset - 1;
		s2 = m_szBuff + nLen - 1;
		sEnd = m_szBuff + nLen + n - 1;

		sEnd[1] = '\0';
		nLen += n;

		// move tail
		while (s2 != s1)
		{
			*sEnd = *s2;
			sEnd--;
			s2--;
		}

		// insert
		s1 = m_szBuff + nOffset;
		sEnd = s1 + n;
		while (s1 != sEnd)
		{
			*s1 = *str;
			s1++;
			str++;
		}
	}

	const T *NCopy(const T *szBeg, const T *szEnd)
	{
		reserve(int(szEnd - szBeg));

		T	*cp = m_szBuff;
		// T	*cpEnd = m_szBuff + nSizeMax - 1;

		nLen = 0;

		for (; szBeg != szEnd; cp++, szBeg++)
		{
			*cp = *szBeg;
		}
		nLen = (int)(cp - m_szBuff);
		
		*cp = '\0';

		return m_szBuff;
	}

/*	// 此函数使用在日志分析时，用来替换+ --> 空格
	const T *NCopyLogRep(const T *szBeg, const T *szEnd)
	{
		reserve(int(szEnd - szBeg));
		T	*cp = m_szBuff;
		// T	*cpEnd = m_szBuff + nSizeMax - 1;

		nLen = 0;

		for (; szBeg != szEnd; cp++, szBeg++)
		{
			//
			// 此函数使用在日志分析时，用来替换+ --> 空格
			//
			if (*szBeg == '+')
				*cp = ' ';
			else
				*cp = *szBeg;
		}
		nLen = (int)(cp - m_szBuff);
		
		*cp = '\0';

		return m_szBuff;
	}
*/
	stringex_t<T> &append(const T * str, int n = -1)
	{
		assert(n >= -1);
		if (n < 0)
		{
			if (n < -1) // -1 for copy all, others are error
				return *this;
			n = _xcslen(str);
		}
		reserve(size() + n);

		T	*cp = m_szBuff + nLen;
		const T	*szEnd = str + n;

		for (; str != szEnd; cp++, str++)
			*cp = *str;
		nLen = (int)(cp - m_szBuff);

		*cp = '\0';

		return *this;
	}

	stringex_t<T> &append(const T c)
	{
		reserve(size() + 1);

		m_szBuff[nLen] = c;
		nLen++;

		return *this;
	}

	void trim()
	{
		T		*szBeg;

		// trim begin
		szBeg = m_szBuff;
		while (*szBeg == ' ')
			szBeg++;

		// move
		if (szBeg != m_szBuff)
		{
			T		*szPos;
			szPos = m_szBuff;
			while (*szBeg !='\0')
			{
				*szPos = *szBeg;
				szPos++;
				szBeg++;
			}
			nLen = size_t(szPos - m_szBuff);
		}

		// trim end
		while (nLen > 0)
		{
			if (m_szBuff[nLen - 1] == ' ')
				nLen--;
			else
				break;
		}
		m_szBuff[nLen] = '\0';
	}

	template<class index_t>
	T & operator[](index_t n)
	{
		if (n >= nLen)
		{
			static	T	temp(0);
			return temp;
		}

		return (m_szBuff[n]);
	}

	stringex_t<T> &operator = (const stringex_t<T> &obj)
	{
		if (nSizeMax < obj.nSizeMax)
		{
			if (m_szBuff)
				free(m_szBuff);
			nSizeMax = obj.nSizeMax;
			m_szBuff = (T *)malloc(nSizeMax * sizeof(T));
		}
		memcpy(m_szBuff, obj.m_szBuff, nSizeMax);
		nLen = obj.nLen;

		return *this;
	}

	const T *operator = (const T * str)
	{
		T	*cp = m_szBuff;
		T	*cpEnd = m_szBuff + nSizeMax - 1;

		nLen = 0;

		for (; *str; cp++, str++)
		{
			if (cp >= cpEnd)
			{
				nLen = (int)(cp - m_szBuff);
				nSizeMax += BUFF_INC;
				m_szBuff = (T *)realloc(m_szBuff, nSizeMax * sizeof(T));
				if (!m_szBuff)
					return NULL;

				cp = m_szBuff + nLen;
				cpEnd = m_szBuff + nSizeMax - 1;
			}
			*cp = *str;
		}
		nLen = (int)(cp - m_szBuff);

		*cp = '\0';

		return m_szBuff;
	}

	stringex_t &operator += (const T ch)
	{
		if (nLen + 1 >= nSizeMax)
		{
			m_szBuff = (T *)realloc(m_szBuff, (nSizeMax + BUFF_INC) * sizeof(T));
			if (!m_szBuff)
				return *this;

			nSizeMax += BUFF_INC;
		}
		m_szBuff[nLen] = ch;
		nLen++;
		m_szBuff[nLen] = '\0';

		return *this;
	}

	stringex_t &operator += (const T *str)
	{
		T	*cp = m_szBuff + nLen;
		T	*cpEnd = m_szBuff + nSizeMax - 1;

		for (; *str; cp++, str++)
		{
			if (cp >= cpEnd)
			{
				nLen = (int)(cp - m_szBuff);
				nSizeMax += BUFF_INC;
				m_szBuff = (T *)realloc(m_szBuff, nSizeMax * sizeof(T));
				if (!m_szBuff)
					return *this;

				cp = m_szBuff + nLen;
				cpEnd = m_szBuff + nSizeMax - 1;
			}
			*cp = *str;
		}
 		nLen = (int)(cp - m_szBuff);

		*cp = '\0';

		return *this;
	}

	stringex_t &operator += (const stringex_t &obj)
	{
		reserve(size() + obj.size() + 1);

		T	*cp = m_szBuff + nLen;
		const T	*str = obj.c_str();
		const T *strEnd = obj.c_str() + obj.size();

		for (; str < strEnd; cp++, str++)
			*cp = *str;

		nLen = (int)(cp - m_szBuff);

		*cp = '\0';

		return *this;
	}

	template<class T2>
	int Cmp(const T2* dst) const
	{
        int ret = 0 ;
		const T * src;

		src = m_szBuff;

        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
			++src, ++dst;

        if ( ret < 0 )
			ret = -1 ;
        else if ( ret > 0 )
			ret = 1 ;

        return( ret );
	}

	bool operator == (stringex_t<T> &obj2)
	{
		return Cmp(obj2.m_szBuff) == 0;
	}

	bool operator != (stringex_t<T> &obj2)
	{
		return Cmp(obj2.m_szBuff) != 0;
	}

	operator const T *() const
	{
		return m_szBuff;
	}
	const T *c_str() const
	{
		return m_szBuff;
	}

	T *data()
	{
		return m_szBuff;
	}

	size_t size() const
	{
		return nLen;
	}

	bool empty() const
	{
		return (nLen == 0);
	}

	size_t capacity() const
	{
		return nSizeMax - 1;
	}

	void reserve(size_t n)
	{
		if (n < nSizeMax)
			return;

		nSizeMax = n + 1;
		m_szBuff = (T *)realloc(m_szBuff, nSizeMax * sizeof(T));
		if (!m_szBuff)
		{
			assert(m_szBuff);
			nSizeMax = 0;
			nLen = 0;
		}
	}

	void resize(int n)
	{
		setSize(n);
	}

	void setSize(int n)
	{
		if (n < (int)nSizeMax)
		{
			nLen = n;
			m_szBuff[n] = '\0';
		}
		else
		{
			reserve(n + 1);
			nLen = n;
			m_szBuff[n] = '\0';
		}
	}

protected:
	T		*m_szBuff;
	size_t	nSizeMax;
	size_t	nLen;

};

typedef stringex_t<TCHAR>	tstringex;
typedef stringex_t<char>	stringex;
typedef stringex_t<WCHAR>	wstringex;
typedef stringex			CCharBuffer;


#endif // _SIMPSTR_T_H_
