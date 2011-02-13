// xstr.h: interface for the Cxstr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XSTR_H__F115BDBC_A6C9_49FD_A00B_6971C90F650C__INCLUDED_)
#define AFX_XSTR_H__F115BDBC_A6C9_49FD_A00B_6971C90F650C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<class INT_t>
INT_t abs_x(INT_t a)
{
	if (a >= 0)
		return a;
	else
		return -a;
}

template<class T1, class T2>
T1 *_xcscpy(T1 *strDestination, const T2 *strSource)
{
    T1	*cp = strDestination;

	for (; *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

    return (strDestination);
}

template<class T1, class T2>
T1 *_xcsncpy(T1 *strDestination, const T2 *strSource, int nLen)
{
	T1	*cp = strDestination;

	const T2 *sourceEnd = strSource + nLen;
	for (; strSource < sourceEnd; cp++, strSource++)
	{
		*cp = (T1)*strSource;
	}

	*cp = '\0';

	return (strDestination);
}

template<class T>
size_t _xcslen (
					   const T * str
					   )
{
	const T *eos = str;

	while( *eos++ ) ;

	return( eos - str - 1 );
}

template<class T1, class T2>
int _xcsncmp(
        const T1 * first,
        const T2 * last,
        size_t count
        )
{
        if (!count)
                return(0);

        while (--count && *first && *first == *last)
        {
                first++;
                last++;
        }

        return( *(unsigned char *)first - *(unsigned char *)last );
}

template<class T1, class T2>
int _xcscmp(
			 const T1 * first,
			 const T2 * last
			 )
{
	while (*first && *first == *last)
	{
		first++;
		last++;
	}

	return( *(unsigned char *)first - *(unsigned char *)last );
}

template<class T1, class T2>
T1 * _xcsistr (
        const T1 * str1,
        const T2 * str2
        )
{
        T1 *cp = (T1 *) str1;
        T1 *s1;
		T2 *s2;
		int f,l;

        if ( !*str2 )
            return((T1 *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (T2 *) str2;

                while ( *s1 && *s2)
				{
					if ( ((f = (unsigned)(*(s1))) >= 'A') && (f <= 'Z') )
						f -= ('A' - 'a');

					if ( ((l = (unsigned)(*(s2))) >= 'A') && (l <= 'Z') )
						l -= ('A' - 'a');
					if (f == l)
                        s1++, s2++;
					else
						break;
				}

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

template<class T>
T *  _xcschr (
        const T * string,
        int ch
        )
{
        while (*string && *string != (T)ch)
                string++;

        if (*string == (T)ch)
                return((T *)string);
        return(NULL);
}

template<class T>
T * _xcsrchr (
						const T * string,
						int ch
						)
{
	T *start = (T *)string;

	while (*string++)                       /* find end of string */
		;
	/* search towards front */
	while (--string != start && *string != (T)ch)
		;

	if (*string == (T)ch)                /* char found ? */
		return( (T *)string );

	return(NULL);
}

template<class T1, class T2>
int _xcsicmp (
        const T1 * dst,
        const T2 * src
        )
{
	int f, l;

	do {
		if ( ((f = (unsigned)(*(dst++))) >= 'A') &&
			(f <= 'Z') )
			f -= 'A' - 'a';
		if ( ((l = (unsigned)(*(src++))) >= 'A') &&
			(l <= 'Z') )
			l -= 'A' - 'a';
	} while ( f && (f == l) );

	return(f - l);
}

template<class T1, class T2>
int _xcsnicmp (
	const T1 * first,
	const T2 * last,
	size_t count
	)
{
	if(count)
	{
		int f=0;
		int l=0;

		do
		{

			if ( ((f = (unsigned)(*(first++))) >= 'A') &&
				(f <= 'Z') )
				f -= 'A' - 'a';

			if ( ((l = (unsigned)(*(last++))) >= 'A') &&
				(l <= 'Z') )
				l -= 'A' - 'a';

		}
		while ( --count && f && (f == l) );

		return ( f - l );
	}
	else
	{
		return 0;
	}
}

template<class TCHARDEF>
int _xtoi(const TCHARDEF *str)
{
	int		nValue = 0;
	BOOL	bPositive = TRUE;

	if (str[0] == '-')
	{
		bPositive = FALSE;
		str++;
	}
	else if (str[0] == '+')
	{
		str++;
	}

	while (*str >= '0' && *str <= '9')
	{
		nValue *= 10;
		nValue += *str - '0';
		str++;
	}

	if (bPositive)
		return nValue;
	else
		return -nValue;
}

template<class TCHARDEF>
TCHARDEF* _itox_s(int value, TCHARDEF *result, size_t buffSize, int base)
{
	// check that the base if valid
	if (base < 2 || base > 16) { *result = 0; return result; }

	TCHARDEF *out = result;
	TCHARDEF *end = result + buffSize - 1;
	int quotient = value;

	do
	{
		*out = "0123456789abcdef"[abs_x(quotient % base)];
		++out;
		quotient /= base;
	}
	while (quotient && out < end);

	// Only apply negative sign for base 10
	if (value < 0 && base == 10 && out < end)
		*out++ = '-';

	std::reverse(result, out);
	*out = 0;
	return result;
}

template<class TCHARDEF>
void _xcsrep(TCHARDEF *str, TCHARDEF Src, TCHARDEF Targ)
{
	while (*str)
	{
		if (*str == Src)
			*str = Targ;
		str++;
	}
}

#endif // !defined(AFX_XSTR_H__F115BDBC_A6C9_49FD_A00B_6971C90F650C__INCLUDED_)
