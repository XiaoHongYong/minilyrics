#include "../base/base.h"
#include "../Utils/Utils.h"

#include "../MLLib/MLEncrypt.h"


int _tmain(int argc, _TCHAR* argv[])
{
	stringex	strData;

	if (argc != 4)
	{
		_tprintf(_T("Usage: %s MiniLyrics.dll search.crintsoft.com www.minilyrics.com\n"), argv[0]);
		return 1;
	}

	TCHAR		szFile[MAX_PATH];
	tstring		strMsg;

	GetModulePath(szFile, NULL);
	_tcscat_safe(szFile, MAX_PATH, argv[1]);

	stringex	strOrg1, strOrg2;

	ConvertStr(argv[2], -1, strOrg1);
	ConvertStr(argv[3], -1, strOrg2);

	// encrypt
	MLEncriptV2(szFile, strOrg1.c_str(), strOrg2.c_str(), strData, strMsg);

	string			str1, str2;
	MLDecriptV2(szFile, str1, str2);

	// try to decrypt data.
	if (strcmp(str1.c_str(), strOrg1.c_str()) == 0 &&
		strcmp(str2.c_str(), strOrg2.c_str()) == 0)
	{
		_tprintf(_T("file: %s was encrypted sucessfully.\n"), argv[0]);
		return 0;
	}

	_tprintf(strMsg.c_str());
	_tprintf(_T("\n"));

	return 1;
}
