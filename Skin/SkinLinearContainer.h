#pragma once

DECLAR_CPPUNIT_TEST_REG(CSkinLinearContainer)

class CSkinLinearContainer : public CSkinContainer  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CSkinLinearContainer();
	virtual ~CSkinLinearContainer();

	void RecalculateUIObjSizePos(CUIObject *pObj);

	void OnMeasureSizeByContent();
	void OnSize();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	void OnSizeForVert();
	void OnSizeForHorz();

	struct Item
	{
		uint16		minSize, size;
		uint16		weight;

		void Set(uint16 minSize, uint16 size, uint16 weight) {
			this->minSize = minSize;
			this->size = size;
			this->weight = weight;
		}
	};
	typedef vector<Item>	VecItems;

	static void ZoomWeightable(VecItems &vItems, int nSize);

	static void Align(VecItems &vItems, int nSize);

	void UIObjsToItems(VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, VecItems &vItems, int &totalMinSize, int &totalSize, int &totalFixedSize);

	void ResizeFromItemsToUIObjs(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);
	void ResizeFromItemsToUIObjsForVert(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);
	void ResizeFromItemsToUIObjsForHorz(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);

	virtual void OnSetChildVisible(CUIObject *pChild, bool bVisible, bool bRedraw);

protected:
	friend class CTestCaseCSkinLinearContainer;

	bool			m_bVertical;
	int				m_nSeparatorThickness;

};
