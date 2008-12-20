#ifndef _OUR_LIB_ID_STRING_INC_
#define _OUR_LIB_ID_STRING_INC_


struct ID_TO_STRING
{
	uint32	dwId;
	LPCTSTR	szId;
};

struct ID_TO_STRING_A
{
	uint32	dwId;
	LPCSTR	szId;
};

#define DEFINE_ID(dwId)			{dwId, #dwId}
#define DEFINE_IDT(dwId)		{dwId, _T(#dwId)}

uint32 StringToID(ID_TO_STRING_A Ids[], LPCSTR szID, uint32 dwDefault);
LPCSTR IDToString(ID_TO_STRING_A Ids[], uint32 nID, LPCSTR szDefaultID);

uint32 StringToID(ID_TO_STRING Ids[], LPCTSTR szID, uint32 dwDefault);
LPCTSTR IDToString(ID_TO_STRING Ids[], uint32 nID, LPCTSTR szDefaultID);

// szValue: szIDx | szIDy | szIDz
uint32 GetCombinationValue(ID_TO_STRING Ids[], LPCTSTR szValue);

// Convert nValue to string: XXX | YYY | ZZZ
void GetCombinationStrValue(ID_TO_STRING Ids[], uint32 nValue, tstring &strValue);

#endif // _OUR_LIB_ID_STRING_INC_
