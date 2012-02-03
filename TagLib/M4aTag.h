#pragma once

//////////////////////////////////////////////////////////////////////////
// References:
// http://atomicparsley.sourceforge.net/mpeg-4files.html
//////////////////////////////////////////////////////////////////////////

class M4aBox
{
public:
	M4aBox();
	virtual ~M4aBox();

	typedef vector<M4aBox*> VecBoxes;
	enum {
		TYPE_SIZE = 4,
		HEADER_SIZE = 8,
	};

	void AddChild(M4aBox *box);

	bool IsType(LPCSTR szType);

	M4aBox *FindBox(LPCSTR szBox);
	M4aBox *NextBox(M4aBox *brother);

	void ClearChildren();

	void ToBuf(string &buf);

	void CalculateNewSize();

	size_t GetDataSize() { return m_nSize - HEADER_SIZE; }

public:
	size_t		m_nOffset;	// The offset of the box
	size_t		m_nSize;	// The size of the whole box.
	char		m_szType[TYPE_SIZE + 1];
	string		m_data;

	size_t		m_nSizeNew;

	VecBoxes	m_vChildren;

};

class CM4aTag
{
public:
	CM4aTag();
	virtual ~CM4aTag();

	virtual int Open(LPCTSTR szFile, bool bModify, bool bCreate = false);
	virtual int Open(FILE *fp, bool bCreate = false);
	void Close();

	int Save();

	int SetLyrics(LPCTSTR szLyrics);

	M4aBox &GetRoot() { return m_root; }

	M4aBox *GetBox(LPCSTR szPath);

protected:
	int Parse(M4aBox *parent, int nExtraHeaderOffset = M4aBox::HEADER_SIZE);

	int Read(M4aBox *parent);
	int Write(string &buf, size_t offset);

	FILE			*m_fp;
	M4aBox			m_root;		// The virtual root box

};
