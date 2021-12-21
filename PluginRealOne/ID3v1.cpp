// ID3v1.cpp: implementation of the CID3v1 class.
//
//////////////////////////////////////////////////////////////////////

#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapckts.h"

#include "../MLPluginLib/MLPlugin.h"
#include "ID3v1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//
// 去掉字符串两端的空格
void TrimStr(LPTSTR szString)
{
	int		nLen;
	LPTSTR	szBeg;
	szBeg = szString;

	while (*szBeg == ' ')
		szBeg++;

	nLen = strlen(szBeg);
	if (nLen > 0)
	{
		while (szBeg[nLen - 1] == ' ')
			nLen--;
		szBeg[nLen] = '\0';
	}

	if (szBeg != szString)
		memmove(szString, szBeg, nLen + 1);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CID3v1::CID3v1()
{

}

CID3v1::~CID3v1()
{

}

int CID3v1::GetInfo(LPCTSTR szFile, char *szTitle, char *szArtist)
{
/*
	ID3V1	id3v1;
*/
	HANDLE hFile;

//	memset(&id3v1, 0, sizeof(ID3V1));
//	
	hFile = CreateFile(szFile, GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	if (SetFilePointer(hFile, -ID3_V1_LEN, 0, FILE_END) == 0XFFFFFFFF)
		goto FAILED;

	char	szID3v1[ID3_V1_LEN];
	DWORD	dwRead;

	if (!ReadFile(hFile, szID3v1, ID3_V1_LEN, &dwRead, NULL))
		goto FAILED;

	if (memcmp(szID3v1, "TAG", ID3_V1_LEN_ID) != 0)
		return 0;

	char	*szPtr;

	szPtr = szID3v1 + ID3_V1_LEN_ID;
	memcpy(szTitle, szPtr, ID3_V1_LEN_TITLE);
	szTitle[ID3_V1_LEN_TITLE] = '\0';
	TrimStr(szTitle);

	szPtr += ID3_V1_LEN_TITLE;
	memcpy(szArtist, szPtr, ID3_V1_LEN_ARTIST);
	szArtist[ID3_V1_LEN_ARTIST] = '\0';
	TrimStr(szArtist);

	CloseHandle(hFile);
	return 1;
FAILED:
	if (hFile)
		CloseHandle(hFile);
	return 0;
}
