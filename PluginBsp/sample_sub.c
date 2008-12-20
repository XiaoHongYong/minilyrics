#define  WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bsp.h"
#include "bspplg.h"

PpluginInfo pInfo;

#define DBGMSG(param)		MessageBox(NULL, param, "Message", MB_OK);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

void Config(const HWND winh)
{
  MessageBox(winh,"Configuration","Info",0);
}

int loadSubtitles(const char * subName)
{
  int subID;

  //You would parse subtitle(subName) file here
  //but for example we will just add a few lines

  //Create new subtitles
  subID = pInfo->createSubs("MiniLyrics subtitles");

  //Add new line that will start at 5 sec and last to 12 sec
  pInfo->addLine(subID,5000,12000,"This is subtitle from 5 to 12 sec");

  //Add few more lines
  pInfo->addLine(subID,15000,17000,"<b>This is subtitle from 15 to 17 sec bold</b>");
  pInfo->addLine(subID,20000,24000,"This is subtitle from 20 to 24 sec\\nLine 2");

  //pInfo->activateSubs(subID);

  return 0; //All OK
}

int __stdcall CallbackProc(const DWORD cmdID,void * param1, void * param2)
{
	switch (cmdID) {
		case EV_UNLOAD:
			DBGMSG("About");
			break;
		case EV_BEFORE_FILE_OPEN:
			break;
		case EV_FILE_OPEN:
			break;
		case EV_CONFIG:
			Config((HWND)param1);
			break;
		case EV_ABOUT:
			MessageBox((HWND)param1, "MiniLyrics subtitle plug-in", "About", 0);
			break;
		case EV_LOAD_SUB:
			DBGMSG("EV_LOAD_SUB");
			return loadSubtitles(param1);
		case EV_PLAY:
			break;
		case EV_STOP:
			break;
		default:
			{
				char	szMsg[256];
				wsprintf(szMsg, "Unknown: %d", cmdID);
				DBGMSG(szMsg);
			}
			break;
	}

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

BOOL __stdcall pluginInit(PpluginInfo plgInfo)
{
	DBGMSG("pluginInit");
  strcpy(plgInfo->description,"MiniLyrics subtitle plug-in");
  plgInfo->plgCallback=CallbackProc;

  pInfo=plgInfo;
  pInfo->subRegisterExt("*.*");
  return TRUE;
}

#ifdef __cplusplus
}
#endif
