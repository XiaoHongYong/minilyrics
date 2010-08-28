// MILocalFile.h: interface for the CMILocalFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_)
#define AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMILocalFile : public IMediaInput  
{
OBJ_REFERENCE_DECL
public:
	CMILocalFile();
	virtual ~CMILocalFile();

	virtual MLRESULT Open(LPCXSTR szSourceMedia);
	virtual uint32 Read(LPVOID lpBuffer, uint32 dwSize);
	virtual MLRESULT Seek(uint32 dwOffset, int nOrigin = SEEK_SET);
	virtual MLRESULT GetSize(uint32 &dwSize);
	virtual uint32 GetPos();

	virtual bool IsEOF();
	virtual bool IsError();

	virtual void Close();

	virtual LPCXSTR GetSource();

protected:
	FILE		*m_fp;
	tstring		m_strFile;

};

#endif // !defined(AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_)
