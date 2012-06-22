/*
 
 EyeTunes.framework - Cocoa iTunes Interface
 http://www.liquidx.net/eyetunes/
 
 Copyright (c) 2005-2007, Alastair Tse <alastair@liquidx.net>
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.
 
 Neither the Alastair Tse nor the names of its contributors may
 be used to endorse or promote products derived from this software without 
 specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
*/

#import <ApplicationServices/ApplicationServices.h>
#import "ETEyeTunes.h"
#import "ETTrack.h"
#import "EyeTunes.h"

void NullLog(NSString *format, ...)
{
	return;
}

const OSType iTunesSignature = ET_APPLE_EVENT_OBJECT_DEFAULT_APPL;

@implementation EyeTunes

+ (EyeTunes *) sharedInstance
{
	static EyeTunes *sharedObject = nil;
	if (sharedObject == nil) {
		sharedObject = [[self alloc] init];
	}
	return sharedObject;
}

#pragma mark -
#pragma mark AppleEvent Utility
#pragma mark -

- (AppleEvent *) newCommandEvent:(AEEventID)eventID
{
	OSErr err;
	AppleEvent *cmdEvent = malloc(sizeof(AppleEvent));
	err = AEBuildAppleEvent(iTunesSignature,
							eventID,
							typeApplSignature,
							&iTunesSignature,
							sizeof(iTunesSignature),
							kAutoGenerateReturnID,
							kAnyTransactionID,
							cmdEvent,
							NULL,
							"'----':'null'()");

	if (err != noErr) {
		ETLog(@"Error creating Apple Event: %d", err);
		free(cmdEvent);
		return nil;
	}
	
	return cmdEvent;
}

- (void)sendCommand:(AEEventID)commandID
{
	OSErr err;
	AppleEvent *event = [self newCommandEvent:commandID];
	err = AESendMessage(event, NULL, kAENoReply | kAENeverInteract, kAEDefaultTimeout);
	if (err != noErr) {
		ETLog(@"Error sending AppleEvent: %d", err);
	}
	AEDisposeDesc(event);
	free(event);
	
}

#pragma mark -
#pragma mark iTunes Commands (No Params)
#pragma mark -


- (void)backTrack
{
	[self sendCommand:ET_BACK_TRACK];
}


- (void)fastForward
{
	[self sendCommand:ET_FAST_FORWARD];
}

- (void)nextTrack
{
	[self sendCommand:ET_NEXT_TRACK];
}

- (void)pause
{
	[self sendCommand:ET_PAUSE];
}

- (void)play
{
	[self sendCommand:ET_PLAY];
}

- (void)playPause
{
	[self sendCommand:ET_PLAYPAUSE];
}

- (void)previousTrack
{
	[self sendCommand:ET_PREVIOUS_TRACK];
}

- (void)resume
{
	[self sendCommand:ET_RESUME];
}

- (void)rewind
{
	[self sendCommand:ET_REWIND];
}

- (void)stop
{
	[self sendCommand:ET_STOP];
}

- (void)playTrackWithPath:(NSString *)path
{
	OSErr err;
	AppleEvent cmdEvent;
	AliasHandle alias =  [EyeTunes newAliasHandleWithPath:path];
	
	if (!alias) {
		ETLog(@"Unable to resolve path to alias");
		return;
	}
	
	err = AEBuildAppleEvent(iTunesSignature,
							ET_PLAY,
							typeApplSignature,
							&iTunesSignature,
							sizeof(iTunesSignature),
							kAutoGenerateReturnID,
							kAnyTransactionID,
							&cmdEvent,
							NULL,
							"'----':alis(@@)",
							alias);
	
	DisposeHandle((Handle)alias);
	
	
	if (err != noErr) {
		ETLog(@"Error creating Apple Event: %d", err);
		return;
	}
	
	err = AESendMessage(&cmdEvent, NULL, kAENoReply | kAENeverInteract, kAEDefaultTimeout);
	if (err != noErr) {
		ETLog(@"Error sending AppleEvent: %d", err);
	}
	AEDisposeDesc(&cmdEvent);
}

- (void)playTrack:(ETTrack *)track
{
	OSErr err;
	AppleEvent cmdEvent;
	
	err = AEBuildAppleEvent(iTunesSignature,
							ET_PLAY,
							typeApplSignature,
							&iTunesSignature,
							sizeof(iTunesSignature),
							kAutoGenerateReturnID,
							kAnyTransactionID,
							&cmdEvent,
							NULL,
							"'----':@",
							[track descriptor]);
	
	if (err != noErr) {
		ETLog(@"Error creating Apple Event: %d", err);
		return;
	}
	
	err = AESendMessage(&cmdEvent, NULL, kAENoReply | kAENeverInteract, kAEDefaultTimeout);
	if (err != noErr) {
		ETLog(@"Error sending AppleEvent: %d", err);
	}
	AEDisposeDesc(&cmdEvent);
}

#pragma mark -
#pragma mark iTunes Properties
#pragma mark -

- (int)playerPosition
{
   return (int)[self getPropertyAsIntegerForDesc:ET_APP_PLAYER_POSITION];
}

- (DescType)playerState
{
    return [self getPropertyAsEnumForDesc:ET_APP_PLAYER_STATE];
}

- (ETTrack *)currentTrack
{
	OSErr err;
	ETTrack *currentTrack = nil;
	
	/* Vars for getting reference to the current track */
	AEDesc	replyObject;
	AppleEvent getEvent, replyEvent;
	
	/* create the apple event to GET something*/
	err = AEBuildAppleEvent(kAECoreSuite,
							'getd',
							typeApplSignature,
							&iTunesSignature,
							sizeof(iTunesSignature),
							kAutoGenerateReturnID,
							kAnyTransactionID,
							&getEvent,
							NULL,
							"'----':obj { form:prop, want:type(prop), seld:type(pTrk), from:'null'() }");
	
	if (err != noErr) {
		ETLog(@"Error creating AppleEvent: %d", err);
		return nil;
	}
	
	/* Send the Apple Event */
	err = AESendMessage(&getEvent, &replyEvent, kAEWaitReply + kAENeverInteract, kAEDefaultTimeout);
	if (err != noErr) {
		ETLog(@"Error sending AppleEvent: %d", err);
		goto cleanup_get_event;
	}

	
	/* Read Results */
	err = AEGetParamDesc(&replyEvent, keyDirectObject, typeWildCard, &replyObject);
	if (err != noErr) {
		ETLog(@"Error extracting from reply event: %d", err);
		goto cleanup_reply_event;
	}
	
	currentTrack = [[[ETTrack alloc] initWithDescriptor:&replyObject] autorelease];

cleanup_reply_event:
	AEDisposeDesc(&replyEvent);
cleanup_get_event:
	AEDisposeDesc(&getEvent);
	return currentTrack;		
}

- (BOOL) fixedIndexing
{
	return (BOOL)[self getPropertyAsIntegerForDesc:ET_APP_FIXED_INDEXING];
}

- (void) setFixedIndexing:(BOOL)useFixedIndexing
{
	[self setPropertyWithInteger:useFixedIndexing forDesc:ET_APP_FIXED_INDEXING];
}

#pragma mark -
#pragma mark Version Checking

- (NSString *)versionString
{
	static NSString *_cachedVersion = nil;
	if (_cachedVersion == nil) {
		_cachedVersion = [[self getPropertyAsVersionForDesc:ET_APP_VERSION] retain];
	}
	return _cachedVersion;
}

- (unsigned int)versionNumber
{
	static unsigned int _cachedVersionInt = 0;
	if (_cachedVersionInt == 0) {
		NSArray *components = [[self versionString] componentsSeparatedByString:@"."];
		int i;
		for (i = 0; i < [components count] && i < 3; i++) {
			_cachedVersionInt |= ([[components objectAtIndex:i] intValue] & 0xff) << (8 - 4*i);
		}
	}
	return _cachedVersionInt;
}

@end
