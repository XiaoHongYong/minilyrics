/*
	File:		iTunesVisualSample.c

    Contains:   iTunes Visual Plug-ins sample code
 
    Version:    Technology: iTunes
                Release:    4.1

	Copyright: 	?Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/

*/

	// Windows headers
#include <windows.h>
#include <shellapi.h>
#include "iTunesVisualAPI.h"


#if TARGET_OS_WIN32
	#define GRAPHICS_DEVICE	HWND
#endif
#if TARGET_OS_MAC
	#define GRAPHICS_DEVICE	CGrafPtr
#endif

#if TARGET_OS_WIN32
#define	MAIN iTunesPluginMain
#define IMPEXP	__declspec(dllexport)
#else
#define IMPEXP
#define	MAIN main
#endif

#define kSampleVisualPluginName		"iPodLyricsDownloader Plug-ins"
#define	kSampleVisualPluginCreator	'\c\r\i\n'

#define	kSampleVisualPluginMajorVersion		1
#define	kSampleVisualPluginMinorVersion		0
#define	kSampleVisualPluginReleaseStage		0x80
#define	kSampleVisualPluginNonFinalRelease	0

// ClearMemory
//
#define UM_ITUNE_LAUNCHED	(WM_USER	+ 0x2314)
#define UM_ITUNE_DESTORYED	(WM_USER	+ 0x2315)

void onITuneLaunch(void)
{
	SHELLEXECUTEINFO info;
	info.cbSize			= sizeof(SHELLEXECUTEINFO);
	info.fMask			= 0;
	info.hwnd			= NULL;
	info.lpVerb			= NULL;
	info.lpFile			= "D:\\@project\\zikiplayer\\trunk\\bin-iPodLyricsDownloader\\iPodLyricsDownloader.exe";
	info.lpParameters	= NULL;
	info.lpDirectory	= NULL;
	info.nShow			= SW_SHOW;
	info.hInstApp		= NULL;

	ShellExecuteEx(&info);


	HWND h = FindWindow("", "");
	if(h) SendMessage(h, UM_ITUNE_LAUNCHED, 0, 0);
}
void onITuneDestroy(void)
{
	HWND h = ::FindWindow("", "");
	if(h) ::SendMessage(h, UM_ITUNE_DESTORYED, 0, 0);
}

/*
	MAIN
*/
IMPEXP OSStatus MAIN (OSType message, PluginMessageInfo *messageInfo, void *refCon)
{
	OSStatus		status = noErr; // or unimpErr;
	
	switch (message)
	{
		case kPluginInitMessage:
			onITuneLaunch();
			break;
			
		case kPluginCleanupMessage:
			onITuneDestroy();
			break;
			
		default:
			status = unimpErr;
			break;
	}
	
	return status;
}
