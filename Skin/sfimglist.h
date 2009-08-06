#if !defined(_RAW_IMAGE_LIST_H_)
#define _RAW_IMAGE_LIST_H_

#pragma once

class CSFImgList : public CSFImage
{
public:
	CSFImgList();
	virtual ~CSFImgList();

	bool Load(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx);

	bool Draw(CRawGraph *pGraph, int nIndex, int x, int y, BlendPixMode bpm = BPM_BLEND);

	int GetIconCx();
	int GetIconCy();

protected:
	int				m_nItemCx;

};

#endif // !defined(_RAW_IMAGE_LIST_H_)
