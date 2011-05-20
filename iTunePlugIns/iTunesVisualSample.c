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

#define kSampleVisualPluginName		"Sample Visual"
#define	kSampleVisualPluginCreator	'\?\?\?\?'

#define	kSampleVisualPluginMajorVersion		1
#define	kSampleVisualPluginMinorVersion		0
#define	kSampleVisualPluginReleaseStage		0x80
#define	kSampleVisualPluginNonFinalRelease	0

enum
{
	kSettingsDialogResID	= 1000,
	
	kSettingsDialogOKButton	= 1,
	kSettingsDialogCancelButton,
	
	kSettingsDialogCheckBox1,
	kSettingsDialogCheckBox2,
	kSettingsDialogCheckBox3
};

struct VisualPluginData {
	void *				appCookie;
	ITAppProcPtr		appProc;

#if TARGET_OS_MAC
	ITFileSpec			pluginFileSpec;
#endif
	
	GRAPHICS_DEVICE		destPort;
	Rect				destRect;
	OptionBits			destOptions;
	UInt32				destBitDepth;

	RenderVisualData	renderData;
	UInt32				renderTimeStampID;
	
	SInt8				waveformData[kVisualMaxDataChannels][kVisualNumWaveformEntries];
	
	UInt8				level[kVisualMaxDataChannels];		/* 0-128 */
	
	ITTrackInfoV1		trackInfo;
	ITStreamInfoV1		streamInfo;

	Boolean				playing;
	Boolean				padding[3];

/*
	Plugin-specific data
*/
#if TARGET_OS_MAC
	GWorldPtr			offscreen;
#endif
};
typedef struct VisualPluginData VisualPluginData;

// ClearMemory
//
static void ClearMemory (LogicalAddress dest, SInt32 length)
{
	register unsigned char	*ptr;

	ptr = (unsigned char *) dest;
	
	if( length > 16 )
	{
		register unsigned long	*longPtr;
		
		while( ((unsigned long) ptr & 3) != 0 )
		{
			*ptr++ = 0;
			--length;
		}
		
		longPtr = (unsigned long *) ptr;
		
		while( length >= 4 )
		{
			*longPtr++ 	= 0;
			length		-= 4;
		}
		
		ptr = (unsigned char *) longPtr;
	}
	
	while( --length >= 0 )
	{
		*ptr++ = 0;
	}
}

/*
	ProcessRenderData
*/
static void ProcessRenderData (VisualPluginData *visualPluginData, const RenderVisualData *renderData)
{
	SInt16		index;
	SInt32		channel;
	
	visualPluginData->level[0] = 0;
	visualPluginData->level[1] = 0;
		
	if (renderData == nil)
	{
		ClearMemory(&visualPluginData->renderData, sizeof(visualPluginData->renderData));
		return;
	}

	visualPluginData->renderData = *renderData;
	
	for (channel = 0; channel < kVisualMaxDataChannels; channel++)
	{
		for (index = 0; index < kVisualNumWaveformEntries; index++)
		{
			SInt8		value;
			
			value = renderData->waveformData[channel][index] - 0x80;
			
			visualPluginData->waveformData[channel][index] = value;
				
			if (value < 0)
				value = -value;
				
			if (value > visualPluginData->level[channel])
				visualPluginData->level[channel] = value;
		}
	}
}

#if TARGET_OS_MAC
// GetPortCopyBitsBitMap
//
static BitMap * GetPortCopyBitsBitMap (CGrafPtr port)
{
	BitMap *		destBMap;
#if OPAQUE_TOOLBOX_STRUCTS
	PixMapHandle	pixMap;
	
	pixMap		= GetPortPixMap(port);
	destBMap	= (BitMap *)(*pixMap);
#else
	destBMap	= (BitMap *)&((GrafPtr)port)->portBits;
#endif

	return destBMap;
}
#endif

/*
	RenderVisualPort
*/
static void RenderVisualPort (VisualPluginData *visualPluginData, GRAPHICS_DEVICE destPort, const Rect *destRect, Boolean onlyUpdate)
{

	(void) visualPluginData;
	(void) onlyUpdate;
	
	if (destPort == nil)
		return;

	#if TARGET_OS_MAC	
	{	
		BitMap *	srcBitMap;
		BitMap *	dstBitMap;
		Rect		srcRect;
		CGrafPtr	oldPort;
		GDHandle	oldDevice;

		if (visualPluginData->offscreen == nil)
			return;
			
		srcRect		= *destRect;
		MacOffsetRect(&srcRect, -srcRect.left, -srcRect.top);

		GetGWorld(&oldPort, &oldDevice);
		
		if (onlyUpdate == false)
		{
			RGBColor	foreColor;
		
			/*
				Update our offscreen pixmap
			*/

			SetGWorld(visualPluginData->offscreen, nil);

			foreColor.red = foreColor.green = ((UInt16)visualPluginData->level[1] << 9);
			foreColor.blue = ((UInt16)visualPluginData->level[0] << 9);

			RGBForeColor(&foreColor);
			PaintRect(&srcRect);
		}
		
		srcBitMap	= GetPortCopyBitsBitMap(visualPluginData->offscreen);
		dstBitMap	= GetPortCopyBitsBitMap(destPort);
		
		SetGWorld(destPort, nil);
		
		ForeColor(blackColor);
		BackColor(whiteColor);

		CopyBits(srcBitMap, dstBitMap, &srcRect, destRect, srcCopy, nil);

		SetGWorld(oldPort, oldDevice);
	}
	#else
	{
		RECT	srcRect;
		HBRUSH	hBrush;
		HDC		hdc;
		
		srcRect.left = destRect->left;
		srcRect.top = destRect->top;
		srcRect.right = destRect->right;
		srcRect.bottom = destRect->bottom;
		
		hdc = GetDC(destPort);		
		hBrush = CreateSolidBrush(RGB((UInt16)visualPluginData->level[1]<<1, (UInt16)visualPluginData->level[1]<<1, (UInt16)visualPluginData->level[0]<<1));
		FillRect(hdc, &srcRect, hBrush);
		DeleteObject(hBrush);
		ReleaseDC(destPort, hdc);
	}
	#endif
}

/*
	AllocateVisualData is where you should allocate any information that depends
	on the port or rect changing (like offscreen GWorlds).
*/
static OSStatus AllocateVisualData (VisualPluginData *visualPluginData, const Rect *destRect)
{
	OSStatus		status;
#if TARGET_OS_MAC
	CGrafPtr		oldPort;
	GDHandle		oldDevice;
	Rect			allocateRect;

	GetGWorld(&oldPort, &oldDevice);
	
	allocateRect = *destRect;
	MacOffsetRect(&allocateRect, -allocateRect.left, -allocateRect.top);
				
	status = JRNewGWorld(&visualPluginData->offscreen, 32, &allocateRect, nil, nil, useTempMem);
	if (status == noErr)
	{
		PixMapHandle	pix = GetGWorldPixMap(visualPluginData->offscreen);

		LockPixels(pix);

		// Offscreen starts out black
		SetGWorld(visualPluginData->offscreen, nil);
		
		ForeColor(blackColor);
		PaintRect(&allocateRect);
	}

	SetGWorld(oldPort, oldDevice);
#else
	(void) visualPluginData;
	(void) destRect;

	status = noErr;
#endif
	return status;
}


/*
	DeallocateVisualData is where you should deallocate the things you have allocated
*/
static void DeallocateVisualData (VisualPluginData *visualPluginData)
{
	#if TARGET_OS_MAC
	if (visualPluginData->offscreen != nil)
	{
		JRDisposeGWorld(visualPluginData->offscreen);
		visualPluginData->offscreen = nil;
	}
	#else
		(void)visualPluginData;
	#endif
}

static Boolean RectanglesEqual(const Rect *r1, const Rect *r2)
{
	if (
		(r1->left == r2->left) &&
		(r1->top == r2->top) &&
		(r1->right == r2->right) &&
		(r1->bottom == r2->bottom)
		)
		return true;
	return false;
	
}

// ChangeVisualPort
//
static OSStatus ChangeVisualPort (VisualPluginData *visualPluginData, GRAPHICS_DEVICE destPort, const Rect *destRect)
{
	OSStatus		status;
	Boolean			doAllocate;
	Boolean			doDeallocate;
	
	status = noErr;
	
	doAllocate		= false;
	doDeallocate	= false;
		
	if (destPort != nil)
	{
		if (visualPluginData->destPort != nil)
		{
			if (RectanglesEqual(destRect, &visualPluginData->destRect) == false)
			{
				doDeallocate	= true;
				doAllocate		= true;
			}
		}
		else
		{
			doAllocate = true;
		}
	}
	else
	{
		doDeallocate = true;
	}
	
	if (doDeallocate)
		DeallocateVisualData(visualPluginData);
	
	if (doAllocate)
		status = AllocateVisualData(visualPluginData, destRect);

	if (status != noErr)
		destPort = nil;

	visualPluginData->destPort = destPort;
	if (destRect != nil)
		visualPluginData->destRect = *destRect;

	return status;
}


/*
	ResetRenderData
*/
static void ResetRenderData (VisualPluginData *visualPluginData)
{
	ClearMemory(&visualPluginData->renderData, sizeof(visualPluginData->renderData));
	
	ClearMemory(&visualPluginData->waveformData[0][0], sizeof(visualPluginData->waveformData));
	
	visualPluginData->level[0] = 0;
	visualPluginData->level[1] = 0;
}

#if TARGET_OS_MAC
/*
	SettingsDialogFilterProc
*/
static DEFINE_API(Boolean) SettingsDialogFilterProc (DialogPtr theDialog, EventRecord * theEvent, short * itemHit)
{
	Boolean				handled;
	WindowPtr			theWindow;
	VisualPluginData *	visualPluginData;
	GrafPtr				savePort;
	
	theWindow			= GetDialogWindow(theDialog);
	visualPluginData	= (VisualPluginData *) GetWRefCon(theWindow);
	handled 			= true;
	*itemHit 			= 0;
	
	GetPort( &savePort );
	MacSetPort( (GrafPtr) GetWindowPort(theWindow) );
	
	switch (theEvent->what)
	{
		case nullEvent:
		{
			/* Let iTunes have idle events so the visuals can continue while playing */ 
			PlayerHandleMacOSEvent( visualPluginData->appCookie, visualPluginData->appProc, theEvent, nil );
			handled = false;
			break;
		}
	
		case updateEvt:
		{
			if( (WindowPtr) theEvent->message == theWindow )
			{
				handled = false;
			}
			else
			{
				PlayerHandleMacOSEvent( visualPluginData->appCookie, visualPluginData->appProc, theEvent, &handled );
			}
			
			break;
		}
		
		default:
			PlayerHandleMacOSEvent( visualPluginData->appCookie, visualPluginData->appProc, theEvent, &handled );
			break;
	}

	if (handled == false)
		handled = StdFilterProc( theDialog, theEvent, itemHit );

	MacSetPort( savePort );
	
	return handled ;
}

/* 
	DoSettingsDialog
*/
static OSStatus DoSettingsDialog (VisualPluginData *visualPluginData)
{
	OSStatus	status;
	DialogPtr	theDialog;
	GrafPtr		savePort;
	
	status = noErr;
	
	GetPort( &savePort );
	
	theDialog = GetNewDialog( kSettingsDialogResID, nil, (WindowRef) -1L );
	if( theDialog != nil )
	{
		WindowRef		theWindow;
		SInt16			itemHit;
		ModalFilterUPP	filterUPP;
		
		theWindow = GetDialogWindow( theDialog );

		MacSetPort( (GrafPtr) GetWindowPort(theWindow) );
		
		SetWRefCon( theWindow, (UInt32) visualPluginData );
		
		SetDialogDefaultItem( theDialog, kSettingsDialogOKButton );
		SetDialogCancelItem( theDialog, kSettingsDialogCancelButton );
	
		MacShowWindow( theWindow );
		SelectWindow( theWindow );
		
		itemHit = 0;
		
		filterUPP = NewModalFilterUPP( SettingsDialogFilterProc );
		
		while( itemHit != kSettingsDialogOKButton && itemHit != kSettingsDialogCancelButton )
		{
			ModalDialog( filterUPP, &itemHit );
		
			switch( itemHit )
			{
				case kSettingsDialogCheckBox1:
				case kSettingsDialogCheckBox2:
				case kSettingsDialogCheckBox3:
				{
					ControlHandle	theControl;
					
					if( GetDialogItemAsControl( theDialog, itemHit, &theControl ) == noErr )
					{
						SetControlValue( theControl, GetControlValue(theControl) == 0 ? 1 : 0 );
					}
				
					break;
				}
				
				case kSettingsDialogCancelButton:
					status = userCanceledErr;
					break;
					
				default:
					break;
			}
		}
		
		DisposeModalFilterUPP( filterUPP );
		DisposeDialog( theDialog );
	}
	else
	{
		status = memFullErr;
	}
	
	MacSetPort( savePort );
	
	return status;
}
#endif

/*
	VisualPluginHandler
*/
static OSStatus VisualPluginHandler (OSType message, VisualPluginMessageInfo *messageInfo, void *refCon)
{
	OSStatus			status;
	VisualPluginData *	visualPluginData;
	
	visualPluginData = (VisualPluginData *)refCon;
	
	status = noErr;

	switch (message)
	{
		/*
			Sent when the visual plugin is registered.  The plugin should do minimal
			memory allocations here.  The resource fork of the plugin is still available.
		*/		
		case kVisualPluginInitMessage:
		{
			visualPluginData = (VisualPluginData *)malloc(sizeof(VisualPluginData));
			if (visualPluginData == nil)
			{
				status = memFullErr;
				break;
			}

			visualPluginData->appCookie	= messageInfo->u.initMessage.appCookie;
			visualPluginData->appProc	= messageInfo->u.initMessage.appProc;

			/* Remember the file spec of our plugin file. We need this so we can open our resource fork during */
			/* the configuration message */

#if TARGET_OS_MAC			
			status = PlayerGetPluginFileSpec( visualPluginData->appCookie, visualPluginData->appProc, &visualPluginData->pluginFileSpec );
#endif

			messageInfo->u.initMessage.refCon	= (void *)visualPluginData;
			break;
		}
			
		/*
			Sent when the visual plugin is unloaded
		*/		
		case kVisualPluginCleanupMessage:
			if (visualPluginData != nil)
				free(visualPluginData);
			break;
			
		/*
			Sent when the visual plugin is enabled.  iTunes currently enables all
			loaded visual plugins.  The plugin should not do anything here.
		*/
		case kVisualPluginEnableMessage:
		case kVisualPluginDisableMessage:
			break;

		/*
			Sent if the plugin requests idle messages.  Do this by setting the kVisualWantsIdleMessages
			option in the PlayerRegisterVisualPluginMessage.options field.
		*/
		case kVisualPluginIdleMessage:
			if (visualPluginData->playing == false)
				RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, false);
			break;

#if TARGET_OS_MAC					
		/*
			Sent if the plugin requests the ability for the user to configure it.  Do this by setting
			the kVisualWantsConfigure option in the PlayerRegisterVisualPluginMessage.options field.
		*/
		case kVisualPluginConfigureMessage:
		{
			short	oldResFile;
			short	ourResFile;
			
			oldResFile = CurResFile();
			
			ourResFile = FSpOpenResFile( &visualPluginData->pluginFileSpec, fsRdPerm );
			if( ourResFile > 0 )
			{
				status = DoSettingsDialog( visualPluginData );
				
				CloseResFile( ourResFile );
			}
			else
			{
				status = fnfErr;
			}

			UseResFile( oldResFile );
			break;
		}
#endif
		
		/*
			Sent when iTunes is going to show the visual plugin in a port.  At
			this point, the plugin should allocate any large buffers it needs.
		*/
		case kVisualPluginShowWindowMessage:
			visualPluginData->destOptions = messageInfo->u.showWindowMessage.options;

			status = ChangeVisualPort(	visualPluginData,
										#if TARGET_OS_WIN32
											messageInfo->u.showWindowMessage.window,
										#endif
										#if TARGET_OS_MAC
											messageInfo->u.showWindowMessage.port,
										#endif
										&messageInfo->u.showWindowMessage.drawRect);
			if (status == noErr)
				RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, true);
			break;
			
		/*
			Sent when iTunes is no longer displayed.
		*/
		case kVisualPluginHideWindowMessage:
			(void) ChangeVisualPort(visualPluginData, nil, nil);

			ClearMemory(&visualPluginData->trackInfo, sizeof(visualPluginData->trackInfo));
			ClearMemory(&visualPluginData->streamInfo, sizeof(visualPluginData->streamInfo));
			break;
		
		/*
			Sent when iTunes needs to change the port or rectangle of the currently
			displayed visual.
		*/
		case kVisualPluginSetWindowMessage:
			visualPluginData->destOptions = messageInfo->u.setWindowMessage.options;

			status = ChangeVisualPort(	visualPluginData,
										#if TARGET_OS_WIN32
											messageInfo->u.setWindowMessage.window,
										#endif
										#if TARGET_OS_MAC
											messageInfo->u.setWindowMessage.port,
										#endif
										&messageInfo->u.setWindowMessage.drawRect);

			if (status == noErr)
				RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, true);
			break;
		
		/*
			Sent for the visual plugin to render a frame.
		*/
		case kVisualPluginRenderMessage:
			visualPluginData->renderTimeStampID	= messageInfo->u.renderMessage.timeStampID;

			ProcessRenderData(visualPluginData, messageInfo->u.renderMessage.renderData);
				
			RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, false);
			break;
			
		/*
			Sent in response to an update event.  The visual plugin should update
			into its remembered port.  This will only be sent if the plugin has been
			previously given a ShowWindow message.
		*/	
		case kVisualPluginUpdateMessage:
			RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, true);
			break;
		
		/*
			Sent when the player starts.
		*/
		case kVisualPluginPlayMessage:
			if (messageInfo->u.playMessage.trackInfo != nil)
				visualPluginData->trackInfo = *messageInfo->u.playMessage.trackInfo;
			else
				ClearMemory(&visualPluginData->trackInfo, sizeof(visualPluginData->trackInfo));

			if (messageInfo->u.playMessage.streamInfo != nil)
				visualPluginData->streamInfo = *messageInfo->u.playMessage.streamInfo;
			else
				ClearMemory(&visualPluginData->streamInfo, sizeof(visualPluginData->streamInfo));
		
			visualPluginData->playing = true;
			break;

		/*
			Sent when the player changes the current track information.  This
			is used when the information about a track changes, or when the CD
			moves onto the next track.  The visual plugin should update any displayed
			information about the currently playing song.
		*/
		case kVisualPluginChangeTrackMessage:
			if (messageInfo->u.changeTrackMessage.trackInfo != nil)
				visualPluginData->trackInfo = *messageInfo->u.changeTrackMessage.trackInfo;
			else
				ClearMemory(&visualPluginData->trackInfo, sizeof(visualPluginData->trackInfo));

			if (messageInfo->u.changeTrackMessage.streamInfo != nil)
				visualPluginData->streamInfo = *messageInfo->u.changeTrackMessage.streamInfo;
			else
				ClearMemory(&visualPluginData->streamInfo, sizeof(visualPluginData->streamInfo));
			break;

		/*
			Sent when the player stops.
		*/
		case kVisualPluginStopMessage:
			visualPluginData->playing = false;
			
			ResetRenderData(visualPluginData);

			RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, true);
			break;
		
		/*
			Sent when the player changes the track position.
		*/
		case kVisualPluginSetPositionMessage:
			break;

		/*
			Sent when the player pauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginPauseMessage:
			visualPluginData->playing = false;

			ResetRenderData(visualPluginData);

			RenderVisualPort(visualPluginData, visualPluginData->destPort, &visualPluginData->destRect, true);
			break;
			
		/*
			Sent when the player unpauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginUnpauseMessage:
			visualPluginData->playing = true;
			break;
		
		/*
			Sent to the plugin in response to a MacOS event.  The plugin should return noErr
			for any event it handles completely, or an error (unimpErr) if iTunes should handle it.
		*/
		case kVisualPluginEventMessage:
			status = unimpErr;
			break;

		default:
			status = unimpErr;
			break;
	}

	return status;	
}


/*
	RegisterVisualPlugin
*/
static OSStatus RegisterVisualPlugin (PluginMessageInfo *messageInfo)
{
	OSStatus			status;
	PlayerMessageInfo	playerMessageInfo;
		
	ClearMemory(&playerMessageInfo.u.registerVisualPluginMessage,sizeof(playerMessageInfo.u.registerVisualPluginMessage));
	
	// copy in name length byte first
	playerMessageInfo.u.registerVisualPluginMessage.name[0] = lstrlen(kSampleVisualPluginName);
	// now copy in actual name
	memcpy(&playerMessageInfo.u.registerVisualPluginMessage.name[1], kSampleVisualPluginName, lstrlen(kSampleVisualPluginName));

	SetNumVersion(&playerMessageInfo.u.registerVisualPluginMessage.pluginVersion, kSampleVisualPluginMajorVersion, kSampleVisualPluginMinorVersion, kSampleVisualPluginReleaseStage, kSampleVisualPluginNonFinalRelease);

	playerMessageInfo.u.registerVisualPluginMessage.options					= kVisualWantsIdleMessages | kVisualWantsConfigure;
	playerMessageInfo.u.registerVisualPluginMessage.handler					= VisualPluginHandler;
	playerMessageInfo.u.registerVisualPluginMessage.registerRefCon			= 0;
	playerMessageInfo.u.registerVisualPluginMessage.creator					= kSampleVisualPluginCreator;
	
	playerMessageInfo.u.registerVisualPluginMessage.timeBetweenDataInMS		= 0xFFFFFFFF; // 16 milliseconds = 1 Tick, 0xFFFFFFFF = Often as possible.
	playerMessageInfo.u.registerVisualPluginMessage.numWaveformChannels		= 2;
	playerMessageInfo.u.registerVisualPluginMessage.numSpectrumChannels		= 2;
	
	playerMessageInfo.u.registerVisualPluginMessage.minWidth				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.minHeight				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.maxWidth				= 32767;
	playerMessageInfo.u.registerVisualPluginMessage.maxHeight				= 32767;
	playerMessageInfo.u.registerVisualPluginMessage.minFullScreenBitDepth	= 0;
	playerMessageInfo.u.registerVisualPluginMessage.maxFullScreenBitDepth	= 0;
	playerMessageInfo.u.registerVisualPluginMessage.windowAlignmentInBytes	= 0;
	
	status = PlayerRegisterVisualPlugin(messageInfo->u.initMessage.appCookie, messageInfo->u.initMessage.appProc,&playerMessageInfo);
		
	return status;
	
}


/*
	MAIN
*/
IMPEXP OSStatus MAIN (OSType message, PluginMessageInfo *messageInfo, void *refCon)
{
	OSStatus		status;
	
	(void) refCon;
	
	switch (message)
	{
		case kPluginInitMessage:
			status = RegisterVisualPlugin(messageInfo);
			break;
			
		case kPluginCleanupMessage:
			status = noErr;
			break;
			
		default:
			status = unimpErr;
			break;
	}
	
	return status;
}
