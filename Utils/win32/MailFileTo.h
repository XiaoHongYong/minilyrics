#ifndef __SENDFILETO_H__
#define __SENDFILETO_H__

#include <mapi.h>

class CMailFileTo
{
public:
	bool SendMail(HWND hWndParent, LPCTSTR szSendToName, LPCTSTR szSendToAddress, LPCTSTR strAttachmentFileName, LPCTSTR strSubject)
	{
		if (IsEmptyString(strAttachmentFileName))
			return false;

// 		if (!hWndParent || !::IsWindow(hWndParent))
// 			return false;

		HINSTANCE hMAPI = ::LoadLibrary(_T("MAPI32.DLL"));
		if (!hMAPI)
			return false;

		ULONG (PASCAL *SendMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
		(FARPROC&)SendMail = GetProcAddress(hMAPI, "MAPISendMail");
		if (!SendMail)
			return false;

		char szFileName[_MAX_PATH] = { 0 };
		char szPath[_MAX_PATH] = { 0 };
		char szSubject[_MAX_PATH] = { 0 };

		ConvertStr2(strAttachmentFileName, -1, szFileName, CountOf(szFileName));
		ConvertStr2(strAttachmentFileName, -1, szPath, CountOf(szPath));
		ConvertStr2(strSubject, -1, szSubject, CountOf(szSubject));
		//_tcscpy_safe(szFileName, CountOf(szFileName), strAttachmentFileName);
		//_tcscpy_safe(szPath, CountOf(szPath), strAttachmentFileName);
		//_tcscpy_safe(szSubject, CountOf(szSubject), strSubject);

		MapiRecipDesc	sendTo;
		char			szSendToNameAnsi[MAX_PATH] = { 0 };
		char			szSendToAddressAnsi[MAX_PATH] = { 0 };
		ConvertStr2(szSendToName, -1, szSendToNameAnsi, CountOf(szSendToNameAnsi));
		ConvertStr2(szSendToAddress, -1, szSendToAddressAnsi, CountOf(szSendToAddressAnsi));

		memset(&sendTo, 0, sizeof(sendTo));
		sendTo.ulRecipClass = MAPI_TO;
		sendTo.lpszName = szSendToNameAnsi;
		sendTo.lpszAddress = szSendToAddressAnsi;

		MapiFileDesc fileDesc;
		::ZeroMemory(&fileDesc, sizeof(fileDesc));
		fileDesc.nPosition = (ULONG)-1;
		fileDesc.lpszPathName = szPath;
		fileDesc.lpszFileName = szFileName;

		MapiMessage message;
		::ZeroMemory(&message, sizeof(message));
		message.lpszSubject = szSubject;
		message.nFileCount = 1;
		message.lpFiles = &fileDesc;

		message.lpRecips = &sendTo;
		message.nRecipCount = 1;

		int nError = SendMail(0, (ULONG_PTR)hWndParent, &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

		if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
			return false;

		return true;
	}
};

#endif
