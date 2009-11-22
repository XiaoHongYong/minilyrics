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

int _tmain(int argc, _TCHAR* argv[])
{
	++argv, --argc;
	if (argc <= 0)
	{
		printf("Please input the file to be compiled.");
		return 1;
	}

	CSJVM		vm;

	if (vm.Compile(argv[0]) != ERR_OK)
		return 1;

	vm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_print_Int, TID_VOID, TID_INT, TID_INVALID);

	vm.RegisterNativeAPI("DebugLog", "print", SJVM_DebugLog_printStr, TID_VOID, TID_STRING, TID_INVALID);

	vm.Run();

	return 0;
}


int	Func22(int a, int b, int c)
{
	return a + b + c;
}
