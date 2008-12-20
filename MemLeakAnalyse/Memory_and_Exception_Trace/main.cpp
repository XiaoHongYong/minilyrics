#include <windows.h>
#include "Stackwalker.h"

void f(int i)
{
  f(i+1);
}

void COM_Mem_Leak()
{
  CoTaskMemAlloc(120);
}

void main()
{
  // Uncomment the following if you only need the UnhandledException-Filter (to log unhandled exceptions)
  // then you can remove the "(De)InitAllocCheck" lines
  //OnlyInstallUnhandeldExceptionFilter();

  InitAllocCheck(ACOutput_XML);

  // This shows how the mem-leak function works
  char *pTest1 = new char[100];

  COM_Mem_Leak();

  // Uncomment the following to show the feature of catching stack-overflows:
  //f(0);

  // This shows the exception handling and log-file writing for an exception:
  // If you want to try it, please comment it out...
  //char *p = NULL;
  //*p = 'A'; // BANG!

  DeInitAllocCheck();
}
