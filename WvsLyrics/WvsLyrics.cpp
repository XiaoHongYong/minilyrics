/**
	Example Winamp AVS plug-in
	Copyright (c) 2000, Nullsoft Inc.
	
	Hello, welcome to the first Advanced Visualization 
	Studio tutorial!
		The hope is that together, we can learn to utilize 
	AVS's powerful features: Namely direct access to the 
	frame buffer, EZ beat detection, and the ability to 
	stack plug-ins written by other developers for an 
	infinite array of possible effects.
	
	I hereby present: 
	Tutorial 0: BOX-
		Simplicity at its finest. Displays a rectangle on 
	screen on every beat. Oh, and you can change its color 
	too... Check avstut00.avs for a demonstration of a 
	spinning rectangle's power!

	good luck and have fun!
**/

#include <windows.h>
#include "resource.h"
#include "MemFont.h"
#include "avs_ape.h"


#define MOD_NAME "MiniLyrics / Lyrics v1.0"
#define UNIQUEIDSTRING "MiniLyrics : Avs lyrics"



class C_THISCLASS : public C_RBASE 
{
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();

		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual char *get_desc();

		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		int enabled;	// toggles plug-in on and off

		CMemFont	m_MemFont;
};

// global configuration dialog pointer 
static C_THISCLASS *g_ConfigThis; 
// global DLL instance pointer (not needed in this example, but could be useful)
static HINSTANCE g_hDllInstance; 


static COLORREF _custcolors[16];

// this is where we deal with the configuration screen
static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		
		if (g_ConfigThis->enabled) 
		{
			CheckDlgButton(hwndDlg, IDC_ENABLE, BST_CHECKED);
		}
		return 1;
		
	case WM_DRAWITEM:
		
		DRAWITEMSTRUCT *di;
		
		di=(DRAWITEMSTRUCT *)lParam;
		if (di->CtlID == IDC_INNER_COLOR) 
		{
			int w;
			int color;
			
			w = di->rcItem.right - di->rcItem.left;
			color = g_ConfigThis->m_MemFont.m_nColorInner;
			color = ((color>>16)&0xff)|(color&0xff00)|((color<<16)&0xff0000);
			
			// paint nifty color button
			HBRUSH hBrush,hOldBrush;
			LOGBRUSH lb={BS_SOLID,color,0};
			hBrush = CreateBrushIndirect(&lb);
			hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
			Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);
			SelectObject(di->hDC,hOldBrush);
			DeleteObject(hBrush);
		}
		else if (di->CtlID == IDC_OUTER_COLOR)
		{
			int w;
			int color;
			
			w = di->rcItem.right - di->rcItem.left;
			color = g_ConfigThis->m_MemFont.m_nColorOuter;
			color = ((color>>16)&0xff)|(color&0xff00)|((color<<16)&0xff0000);
			
			// paint nifty color button
			HBRUSH hBrush,hOldBrush;
			LOGBRUSH lb={BS_SOLID,color,0};
			hBrush = CreateBrushIndirect(&lb);
			hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
			Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);
			SelectObject(di->hDC,hOldBrush);
			DeleteObject(hBrush);
		}
		return 0;
	case WM_COMMAND:
		// see if enable checkbox is checked
		if (LOWORD(wParam) == IDC_ENABLE)
		{
			g_ConfigThis->enabled= (IsDlgButtonChecked(hwndDlg,IDC_ENABLE)?1:0);
		}
		
		// is colorbox is selected?
		if (LOWORD(wParam) == IDC_INNER_COLOR)
		{
			int *a;
			CHOOSECOLOR cs;
			
			a=&g_ConfigThis->m_MemFont.m_nColorInner;
			
			cs.lStructSize = sizeof(cs);
			cs.hwndOwner = hwndDlg;
			cs.hInstance = 0;
			cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
			cs.lpCustColors = _custcolors;
			cs.Flags = CC_RGBINIT|CC_FULLOPEN;
			
			// go to windows color selection screen
			if (ChooseColor(&cs))
			{
				*a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
			}
			InvalidateRect(GetDlgItem(hwndDlg, IDC_INNER_COLOR), NULL, TRUE);            
			
		}
		if (LOWORD(wParam) == IDC_OUTER_COLOR)
		{
			int *a;
			CHOOSECOLOR cs;
			
			a=&g_ConfigThis->m_MemFont.m_nColorOuter;
			
			cs.lStructSize = sizeof(cs);
			cs.hwndOwner = hwndDlg;
			cs.hInstance = 0;
			cs.rgbResult=((*a>>16)&0xff)|(*a&0xff00)|((*a<<16)&0xff0000);
			cs.lpCustColors = _custcolors;
			cs.Flags = CC_RGBINIT|CC_FULLOPEN;
			
			// go to windows color selection screen
			if (ChooseColor(&cs))
			{
				*a = ((cs.rgbResult>>16)&0xff)|(cs.rgbResult&0xff00)|((cs.rgbResult<<16)&0xff0000);
			}
			InvalidateRect(GetDlgItem(hwndDlg,IDC_OUTER_COLOR),NULL,TRUE);            
		}
		if (LOWORD(wParam) == IDC_SEL_FONT)
		{
			LOGFONT		lgFont;

			lgFont = g_ConfigThis->m_MemFont.m_LogFont;

			CHOOSEFONT	choosefont;
			choosefont.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;// | CF_EFFECTS;
			choosefont.lpLogFont = &lgFont;
			choosefont.hwndOwner = hwndDlg;
			choosefont.lStructSize = sizeof(choosefont);
			if (ChooseFont(&choosefont))
			{
				if (lgFont.lfHeight < 0)
					lgFont.lfHeight = -lgFont.lfHeight;

				g_ConfigThis->m_MemFont.SetFont(lgFont);
			}
		}
		return 0;
	}
	return 0;
}




// set up default configuration 
C_THISCLASS::C_THISCLASS() 
{
	//set initial color
	enabled=1;
}

// virtual destructor
C_THISCLASS::~C_THISCLASS() 
{
}


// RENDER FUNCTION:
// render should return 0 if it only used framebuffer, or 1 if the new output data is in fbout. this is
// used when you want to do something that you'd otherwise need to make a copy of the framebuffer.
// w and h are the-*/ width and height of the screen, in pixels.
// isBeat is 1 if a beat has been detected.
// visdata is in the format of [spectrum:0,wave:1][channel][band].

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
//	int halfw;
//	int halfh;

	// is this effect on?
	if (!enabled) 
	{
		return 0;
	}

	m_MemFont.render(framebuffer, w, h);

/*	// did we just hit a beat?
	if(isBeat)
	{
		// draw our magic box
		halfw=w/2;
		halfh=h/2;

		framebuffer+=(((halfh/2)*w)+ (halfw/2));
		
		for(int j=0;j<halfh;j++)
		{
			for(int i=0;i<halfw;i++)
			{
				framebuffer[i]=color;
			}
			framebuffer+=w;
		}		
	}*/
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent) // return NULL if no config dialog possible
{
	g_ConfigThis = this;
	return CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hwndParent, g_DlgProc);
}


char *C_THISCLASS::get_desc(void)
{ 
	return MOD_NAME; 
}


// load_/save_config are called when saving and loading presets (.avs files)

#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))

#define GET_STRING(str)		\
{\
	char *cp;\
	cp = str;\
	while (pos < len)\
	{\
		if (data[pos] == 0)\
		{\
			pos++;\
			break;\
		}\
		*cp = (char)data[pos];\
		pos++;\
		cp++;\
	}\
	*cp = 0;\
}

int load_string(unsigned char *data, int len, char *szOut)
{
	char *cp;
	int		pos = 0;

	cp = szOut;
	while (pos < len)
	{
		if (data[pos] == 0)
		{
			pos++;
			break;
		}
		*cp = (char)data[pos];
		pos++;
		cp++;
	}
	*cp = 0;

	return pos;
}

void C_THISCLASS::load_config(unsigned char *data, int len) // read configuration of max length "len" from data.
{
	int pos=0;

	// always ensure there is data to be loaded
	if (len-pos >= 4) 
	{
		// load activation toggle
		enabled=GET_INT();
		pos+=4; 
	}

	if (len - pos >= 4) 
    { 
		m_MemFont.m_nColorInner = GET_INT(); 
		pos += 4; 
	}

	if (len - pos >= 4) 
    { 
		m_MemFont.m_nColorOuter = GET_INT(); 
		pos += 4; 
	}

	if (len - pos >= 4) 
    { 
		m_MemFont.m_LogFont.lfHeight = GET_INT(); 
		pos += 4; 
	}

	if (len - pos >= 4) 
    { 
		m_MemFont.m_LogFont.lfWeight = GET_INT(); 
		pos += 4; 
	}

	if (len - pos >= 4) 
    { 
		m_MemFont.m_LogFont.lfItalic = GET_INT(); 
		pos += 4; 
	}

	pos += load_string(data + pos, len - pos, m_MemFont.m_LogFont.lfFaceName);

	m_MemFont.SetFont(m_MemFont.m_LogFont);
}


// write configuration to data, return length. config data should not exceed 64k.
#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define PUT_STRING(str)		\
{\
	int		nLen;\
	nLen = strlen(str);\
	memcpy(&(data[pos]), str, nLen);\
	pos += nLen;\
}

int put_string(unsigned char *data, char *szStr)
{
	int		nLen;

	nLen = strlen(szStr) + 1;
	memcpy(data, szStr, nLen);

	return nLen;
}

int  C_THISCLASS::save_config(unsigned char *data) 
{
	int pos=0;
	
	PUT_INT(enabled); 
	pos+=4;

	PUT_INT(m_MemFont.m_nColorInner); 
	pos+=4;

	PUT_INT(m_MemFont.m_nColorOuter); 
	pos+=4;

	PUT_INT(m_MemFont.m_LogFont.lfHeight);
	pos+=4;

	PUT_INT(m_MemFont.m_LogFont.lfWeight);
	pos+=4;

	PUT_INT(m_MemFont.m_LogFont.lfItalic);
	pos+=4;

	pos += put_string(data + pos, m_MemFont.m_LogFont.lfFaceName);

	return pos;
}







// export stuff
C_RBASE *R_RetrFunc(char *desc) // creates a new effect object if desc is NULL, otherwise fills in desc with description
{
	if (desc) 
	{ 
		strcpy(desc,MOD_NAME); 
		return NULL; 
	}
	return (C_RBASE *) new C_THISCLASS();
}

extern "C"
{
	__declspec (dllexport) int _AVS_APE_RetrFunc(HINSTANCE hDllInstance, char **info, int *create) // return 0 on failure
	{
		g_hDllInstance=hDllInstance;
		*info=UNIQUEIDSTRING;
		*create=(int)(void*)R_RetrFunc;
		return 1;
	}
};


/**
	Final Thoughts:
		Alright! Hopefully you guys can take the next step 
	and display more than just a colored rectangle ;) The 
	exciting thing is, each time you write an AVS plug-in, 
	you exponentially increase AVS's potential, unlocking
	the possibility of an effect you never expected. Good 
	luck, I hope this has helped!
	
	See you next time!
**/