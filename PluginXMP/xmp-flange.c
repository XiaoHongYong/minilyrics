// Flanger DSP plugin example (c) 2004 Ian Luck

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <math.h>

#include "xmpdsp.h"

static HINSTANCE ghInstance;

// config structure
typedef struct {
	BOOL on;
	DWORD speed;
} FlangeStuff;
static FlangeStuff flange;

static int flachans;
static int flabuflen;
static float *flabuf=0;
static int flapos;
static float flas,flasinc;

static void *WINAPI DSP_New();
static void WINAPI DSP_Free(void *inst);
static char *WINAPI DSP_GetDescription(void *inst);
static void WINAPI DSP_Config(void *inst, HWND win);
static DWORD WINAPI DSP_GetConfig(void *inst, void *config);
static BOOL WINAPI DSP_SetConfig(void *inst, void *config);
static void WINAPI DSP_NewTrack(void *inst, char *file);
static void WINAPI DSP_SetFormat(void *inst, DWORD rate, DWORD chans);
static void WINAPI DSP_Reset(void *inst);
static DWORD WINAPI DSP_Process(void *inst, float *srce, DWORD count);

static void WINAPI DSP_About(HWND win)
{
	MessageBox(win,"blah blah blah...","Flanger DSP example",MB_ICONINFORMATION);
}

static XMPDSP dsp={
	0,
	0, // doesn't support multiple instances
	"Flanger",
	DSP_About,
	DSP_New,
	DSP_Free,
	DSP_GetDescription,
	DSP_Config,
	DSP_GetConfig,
	DSP_SetConfig,
	DSP_NewTrack,
	DSP_SetFormat,
	DSP_Reset,
	DSP_Process
};

static void SetFlangeSpeed()
{
	flasinc=0.00003f*pow(2,flange.speed/12.0f);
}

// new DSP instance
static void *WINAPI DSP_New()
{
	flange.on=TRUE;
	flange.speed=50;
	return (void*)1; // no multi-instance, so just return anything not 0
}

// free DSP instance
static void WINAPI DSP_Free(void *inst)
{
}

// get description for plugin list
static char *WINAPI DSP_GetDescription(void *inst)
{
	return dsp.name;
}

#define MESS(id,m,w,l) SendDlgItemMessage(h,id,m,(WPARAM)(w),(LPARAM)(l))

static BOOL CALLBACK DSPDialogProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	switch (m) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case IDCANCEL:
					EndDialog(h,0);
					break;
				case 10:
					DSP_Reset(0);
					flange.on=MESS(10,BM_GETCHECK,0,0);
					break;
			}
			break;
		case WM_VSCROLL:
			if (l) {
				flange.speed=100-SendMessage((HWND)l,TBM_GETPOS,0,0);;
				SetFlangeSpeed();
			}
			break;
		case WM_INITDIALOG:
			{
				MESS(10,BM_SETCHECK,flange.on?1:0,0);
				MESS(11,TBM_SETRANGE,FALSE,MAKELONG(0,100));
				MESS(11,TBM_SETPOS,TRUE,100-flange.speed);
			}
			return 1;
	}
	return 0;
}

// show config options
static void WINAPI DSP_Config(void *inst, HWND win)
{
	DialogBox(ghInstance,(char*)1000,win,&DSPDialogProc);
}
// get DSP config
static DWORD WINAPI DSP_GetConfig(void *inst, void *config)
{
	memcpy(config,&flange,sizeof(flange));
	return sizeof(flange); // return size of config info
}
// set DSP config
static BOOL WINAPI DSP_SetConfig(void *inst, void *config)
{
	memcpy(&flange,config,sizeof(flange));
	return TRUE;
}

// reset DSP when seeking
static void WINAPI DSP_Reset(void *inst)
{
	flapos=0;
	flas=flabuflen/2;
	if (flabuf) memset(flabuf,0,flabuflen*flachans*sizeof(float));
}

// new track has been opened (or closed if file=NULL)
static void WINAPI DSP_NewTrack(void *inst, char *file)
{
}

// set the sample format at start (or end if rate/chans=0) of playback
static void WINAPI DSP_SetFormat(void *inst, DWORD rate, DWORD chans)
{
	if (chans) {
		flachans=chans;
		flabuflen=rate/125;
		flabuf=malloc(flabuflen*flachans*sizeof(float));
		SetFlangeSpeed();
		DSP_Reset(0);
	} else {
		free(flabuf);
		flabuf=0;
	}
}

// process the sample data
static DWORD WINAPI DSP_Process(void *inst, float *buffer, DWORD count)
{
	int a,b;
	if (flange.on) {
		for (a=count;a;a--) {
			int p1=(flapos+(int)flas)%flabuflen;
			int p2=(p1+1)%flabuflen;
			float *fb1=flabuf+p1*flachans;
			float *fb2=flabuf+p2*flachans;
			float *fb3=flabuf+flapos*flachans;
			float f=fmod(flas,1);

			for (b=0;b<flachans;b++) {
				float s=(*buffer+((fb1[b]*(1-f))+(fb2[b]*f)))*0.7;
				fb3[b]=*buffer;
				*buffer++=s;
			}

			flapos++;
			if (flapos==flabuflen) flapos=0;
			flas+=flasinc;
			if (flas<0 || flas>flabuflen-1) {
				flasinc=-flasinc;
				flas+=flasinc;
			}
		}
	}
	return count;
}

// the exported function that XMPlay calls to get the plugin interface
__declspec(dllexport) XMPDSP *WINAPI XMPDSP_GetInterface(DWORD face)
{
	return &dsp;
}

BOOL WINAPI DllMain(HANDLE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			ghInstance=hDLL;
			DisableThreadLibraryCalls(hDLL);
			break;
	}
	return 1;
}
