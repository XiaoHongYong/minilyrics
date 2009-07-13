// MPHelper.h: interface for the CMPHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPHELPER_H__C910ABDB_E887_461F_8A57_5F8CCCC6045C__INCLUDED_)
#define AFX_MPHELPER_H__C910ABDB_E887_461F_8A57_5F8CCCC6045C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

void calc_freq(unsigned char *dest, unsigned char *src);

LPCTSTR MultiStrNext(LPCTSTR szText);

class CXStr : public IXStr
{
OBJ_REFERENCE_DECL
public:
	CXStr();
	virtual ~CXStr();

	virtual LPXSTR data();
	virtual LPCXSTR c_str();
	virtual size_t size();
	virtual uint32 capacity();
	virtual void resize(uint32 nSize);
	virtual MLRESULT reserve(uint32 nCapacity);
	virtual void copy(IXStr *pSrcStr);
	virtual void copy(LPCXSTR str);
	virtual void erase(int nOffset, int n);
	virtual void clear();
	virtual void insert(int nOffset, LPCXSTR str, int n);
	virtual void append(LPCXSTR str, int n);

protected:
	tstringex		m_str;

};

class CVXStr : public IVXStr
{
OBJ_REFERENCE_DECL
public:
	CVXStr();
	virtual ~CVXStr();

	virtual size_t size();
	virtual void push_back(LPCXSTR szStr);
	virtual void clear();
	virtual LPCXSTR at(int index);
	virtual void set(int index, LPCXSTR szStr);
	virtual void insert(int index, LPCTSTR szStr);

protected:
	vector<tstring>		m_vstr;

};

class CVInt : public IVInt
{
	OBJ_REFERENCE_DECL
public:
	CVInt();
	virtual ~CVInt();

	virtual size_t size();
	virtual void push_back(int nData);
	virtual void clear();
	virtual int at(int index);
	virtual void set(int index, int nData);
	virtual void insert(int index, int nData);

protected:
	vector<int>			m_vInt;

};

class CVector : public IVector
{
	OBJ_REFERENCE_DECL
public:
	CVector() { OBJ_REFERENCE_INIT }
	virtual ~CVector() { }

	virtual size_t size() { return m_vData.size(); }
	virtual void push_back(LPVOID p) { m_vData.push_back(p); }
	virtual void clear() { m_vData.clear(); }
	virtual LPVOID at(int index)
		{ if (index >= 0 && index < (int)m_vData.size()) return m_vData[index]; else return NULL; }
	virtual void set(int index, LPVOID p)
		{ if (index >= 0 && index < (int)m_vData.size()) m_vData[index] = p; }
	virtual void insert(int index, LPVOID p)
		{ if (index >= 0 && index < (int)m_vData.size()) m_vData.insert(m_vData.begin() + index, p); }

protected:
	vector<LPVOID>			m_vData;

};

#endif // !defined(AFX_MPHELPER_H__C910ABDB_E887_461F_8A57_5F8CCCC6045C__INCLUDED_)
