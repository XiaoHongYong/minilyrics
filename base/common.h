// MLError.h: interface for the CMLError class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ML_COMMON_INC_
#define _ML_COMMON_INC_

enum FILE_ENCODING
{
	FE_ANSI,
	FE_UCS2,
	FE_UTF8
};

#define CountOf(arr)		(sizeof(arr) / sizeof(arr[0]))

#define IsEmptyString(str)	((str)[0] == '\0')

#define EmptyStr(str)	((str)[0] = '\0')

#define IsFlagValid(value, flag)	(((value) & (flag)) == (flg))
#define IsFlagSet(value, flag)	(((value) & (flag)) == (flag))

#define Tobool(bValue)		((bValue) != 0)

template<class int_t1, class int_t2>
int_t1 inline max_fun(int_t1 a, int_t2 b)
{
	return (((a) > (b)) ? (a) : (b));
}

template<class int_t1, class int_t2>
int_t1 inline min_fun(int_t1 a, int_t2 b)
{
	return (((a) < (b)) ? (a) : (b));
}

#ifndef min
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define DIR_SLASH		'\\'
#define SZ_DIR_SLASH	"\\"
#else
#define DIR_SLASH		'/'
#define SZ_DIR_SLASH	"/"
#endif

#define CHAR_SPACE		_T(' ')
#define CHAR_TAB		_T('\t')
#define CHAR_NEWLINE	_T('\n')
#define CHAR_RETURN		_T('\r')
#define CHAR_NULL		_T('\0')
#define CHAR_COMMA		_T(',')
#define CHAR_SEMICOLON	_T(';')

#ifndef _WIN32
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((a) & 0xff)) | ((WORD)((BYTE)((b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((a) & 0xffff)) | ((DWORD)((WORD)((b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((l) & 0xffff))
#define HIWORD(l)           ((WORD)((l) >> 16))
#define LOBYTE(w)           ((BYTE)((w) & 0xff))
#define HIBYTE(w)           ((BYTE)((w) >> 8))
#endif

#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#endif

#ifndef _WIN32

inline void InterlockedIncrement(long *nReference)
{
	(*nReference)++;
}

inline int InterlockedDecrement(long *nReference)
{
	return --(*nReference);
}

#endif

#define OBJ_REFERENCE_DECL	\
public:\
	virtual void AddRef();\
	virtual void Release();\
protected:\
	long			m_nReference;

#define OBJ_REFERENCE_INIT	\
	m_nReference = 0;

#define OBJ_REFERENCE_IMP(class)	\
void class::AddRef()\
{\
	InterlockedIncrement(&m_nReference);\
}\
void class::Release()\
{\
	if (InterlockedDecrement(&m_nReference) <= 0)\
		delete this;\
}\


template <class T>
class CMPAutoPtr
{
public:
	typedef T _PtrClass;
	CMPAutoPtr()
	{
		p=NULL;
	}
	CMPAutoPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CMPAutoPtr(const CMPAutoPtr<T>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	~CMPAutoPtr()
	{
		if (p)
			p->Release();
	}
	void Release()
	{
		if (p)
		{
			p->Release();
			p = NULL;
		}
	}
	operator T*() const
	{
		return (T*)p;
	}
	T& operator*() const
	{
		ASSERT(p!=NULL);
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&()
	{
		ASSERT(p==NULL);
		return &p;
	}
	T* operator->() const
	{
		ASSERT(p!=NULL);
		return p;
	}
	T* operator=(T* lp)
	{
		if (lp)
			lp->AddRef();
		if (p)
			(p)->Release();
		p = lp;
		return (T*)lp;
	}
	T* operator=(const CMPAutoPtr<T>& lp)
	{
		if (lp.p)
			lp.p->AddRef();
		if (p)
			(p)->Release();
		p = lp.p;
		return (T*)lp.p;
	}
	bool operator!() const
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const
	{
		return p < pT;
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}

	T* p;
};


#endif // _ML_COMMON_INC_
