// SimpleJVM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "SJVM.h"

 int __stdcall	Func(int a, int b, int c)
{
	printf ("a: %d, b: %d, c: %d\n", a, b, c);
	return a + b + c;
}

void ASMCallTest()
{
	int		nRet = 1;
	int		vParameter[10] = { 1, 2, 3, 4, };

	nRet = Func(vParameter[0], vParameter[1], vParameter[2]);


	printf("%d\n", nRet);

	int		nCount = 2;

	//	for (int i = (int)2; i >= 0; i--)
	{
		//		printf("%d", vParameter[i]);
		__asm
		{
			mov         eax,	dword ptr [nCount]
ParamStart:
			cmp         eax,	0 
				jl          Param_end
				mov         ecx,	dword ptr vParameter[eax*4] 
			push        ecx  
				sub         eax,	1 
				jmp			ParamStart 
Param_end:
		}
	}

	LPVOID pNativeFunc = Func;

	__asm  {
		mov			eax,      dword ptr [pNativeFunc]
		call		eax
			mov         dword ptr [nRet],eax 
	};

	printf("%d\n", nRet);

}

void __stdcall SJVM_DebugLog_print_Int(int nValue)
{
	printf("%d\n", nValue);
}

void __stdcall SJVM_DebugLog_printStr(LPCWSTR szValue)
{
	wprintf(L"%s\n", szValue);
}

int RunFile(LPCTSTR szFile)
{
	DWORD		dwBeg, dwEnd;

	dwBeg = GetTickCount();

	if (g_sjvm.Compile(szFile) != ERR_OK)
		return 1;

	dwEnd = GetTickCount();
	printf("Compile cost: %d ms\n", dwEnd - dwBeg);

	g_sjvm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_print_Int, TID_VOID, TID_INT, TID_INVALID);

	g_sjvm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_printStr, TID_VOID, TID_STRING, TID_INVALID);

	g_sjvm.Run();

	dwEnd = GetTickCount();
	printf("Execute cost: %d ms\n", dwEnd - dwBeg);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
/*	++argv, --argc;
	if (argc <= 0)
	{
		printf("Please input the file to be compiled.");
		return 1;
	}*/

	// Set flag to freeze the current SJVM status as system module.
	g_sjvm.FinishedInitSysModule();

	if (argc > 1)
	{
		RunFile(argv[1]);
	}

	TCHAR		szFolder[MAX_PATH];

	GetModulePath(szFolder, NULL);
	tstring		strFolder = szFolder;
	strFolder += _T("..\\SimpleJVM\\Test\\");

	V_Str		vFiles;

	if (!EnumFilesInDir(strFolder.c_str(), _T("*.*"), vFiles, true))
	{
		printf("No file is in the test folder");
		return 1;
	}

	DWORD		dwBeg, dwEnd;

	for (size_t i = 0; i < vFiles.size(); i++)
	{
		_tprintf(_T("Run Test case: %s\n"), vFiles[i].c_str());

		dwBeg = GetTickCount();

		g_sjvm.Reset();

		if (g_sjvm.Compile(vFiles[i].c_str()) != ERR_OK)
			return 1;

		dwEnd = GetTickCount();
		printf("Compile cost: %d ms\n", dwEnd - dwBeg);

		g_sjvm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_print_Int, TID_VOID, TID_INT, TID_INVALID);

		g_sjvm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_printStr, TID_VOID, TID_STRING, TID_INVALID);

		g_sjvm.Run();

		dwEnd = GetTickCount();
		printf("Execute cost: %d ms\n", dwEnd - dwBeg);
	}

	return 0;
}


int	Func22(int a, int b, int c)
{
	return a + b + c;
}
