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

#import "EyeTunesEventCodes.h"
#import "ETEyeTunes.h"
#import "ETPlaylist.h"
#import "ETTrack.h"

static const BOOL doLog = NO;

@implementation ETPlaylist

void Log(const char* szFormat, ...);

- (id) initWithDescriptor:(AEDesc *)desc
{
	if (![super initWithDescriptor:desc applCode:ET_APPLE_EVENT_OBJECT_DEFAULT_APPL])
		return nil;
	
	parentPlaylistId = -1;
	persistentId = -1;
	return self;
}

- (void) dealloc
{
	[childPlaylistIds release];
	
	[super dealloc];
}

- (NSString *)name
{
	return [self getPropertyAsStringForDesc:ET_ITEM_PROP_NAME];
}


- (DescType) specialKind
{
	return [self getPropertyAsEnumForDesc:ET_PLAYLIST_PROP_SPECIAL_KIND];
}

- (BOOL) isSmart
{
    return [self getPropertyAsIntegerForDesc:ET_PLAYLIST_PROP_SMART];
}

- (int) trackCount
{
	return [self getCountOfElementsOfClass:ET_CLASS_TRACK];
}

- (ETPlaylist*) parentPlaylist
{
	AppleEvent *replyEvent = [self getPropertyOfType:ET_PLAYLIST_PROP_PARENT];
	
	if (!replyEvent) 
	{
		// TODO: raise exception?
		return nil;
	}
	
	Handle stringHandle;
	OSErr __unused err = AEPrintDescToHandle(replyEvent, &stringHandle);
	if (doLog) NSLog(@"-[ETPlaylist parentplaylist] %@ -- replyEvent: %s (AEPrintDescToHandle result %d)", [self name], *stringHandle, err);
	
	/* Read Results */
	AEDesc playlistDescriptor;
	err = AEGetParamDesc((const AppleEvent *)replyEvent, keyDirectObject, typeWildCard, &playlistDescriptor);
	if (err != noErr) 
	{
		DescType		playlistDesc;
		int			replyValue = -1;
		Size		resultSize;
		err = AEGetParamPtr(replyEvent, keyErrorNumber, typeWildCard, &playlistDesc, 
							&replyValue, sizeof(replyValue), &resultSize);
		if (replyValue != errAENoSuchObject)
		{
//			Log("ERROR in -[ETPlaylist parentplaylist] \"%s\" -- replyEvent: %s (AEGetParamPtr result %d)", [[self name] UTF8String], *stringHandle, err);
		}
		return nil;
	}
	
	ETPlaylist * parentPlaylist = [[[ETPlaylist alloc] initWithDescriptor:&playlistDescriptor] autorelease];
	AEDisposeDesc(replyEvent);
	free(replyEvent);
	
	return parentPlaylist;
}

// we need to be able to overwrite this value to move the playlist to a different location in the tree
- (void) setParentPlaylistId:(unsigned long long)inParentPlaylistId;
{
	parentPlaylistId = inParentPlaylistId;
}

- (NSArray*) childPlaylistIds;
{
	if (!areChildrenSorted)
	{
		[childPlaylistIds sortUsingSelector:@selector(comparePlaylistName:)];
		areChildrenSorted = YES;
	}
	
	return [NSArray arrayWithArray:childPlaylistIds];
}


- (void) addChildPlaylistId:(NSNumber*)childPlaylistId;
{
	if (!childPlaylistIds)
		childPlaylistIds = [[NSMutableArray alloc] init];
	
	[childPlaylistIds addObject:childPlaylistId];
	areChildrenSorted = NO;
}

- (ETTrack *)trackWithDatabaseId:(int)databaseId
{
	
	ETTrack *foundTrack = nil;
	AppleEvent *replyEvent = [self getElementOfClass:ET_CLASS_TRACK 
											   byKey:ET_TRACK_PROP_DATABASE_ID 
										withIntValue:databaseId];
	
	/* Read Results */
	AEDesc replyObject;
	OSErr err;
	err = AEGetParamDesc(replyEvent, keyDirectObject, typeWildCard, &replyObject);
	if (err != noErr) {
//		Log("Error extracting from reply event: %d", err);
		goto cleanup_reply_event;
	}
	
	foundTrack = [[[ETTrack alloc] initWithDescriptor:&replyObject] autorelease];
	
cleanup_reply_event:
	AEDisposeDesc(replyEvent);
	free(replyEvent);
	
	return foundTrack;
}

- (BOOL)containsTrack:(ETTrack *)track{
    return ([self trackWithDatabaseId:track.databaseId] != nil);
}

- (ETTrack *)trackAtIndex:(int)index
{
	OSErr err;
	AEDesc trackDescriptor;

	AppleEvent *replyEvent = [self getElementOfClass:ET_CLASS_TRACK atIndex:index];
	if (!replyEvent)
		return nil;
	
	err = AEGetParamDesc(replyEvent, keyDirectObject, typeWildCard, &trackDescriptor);
	AEDisposeDesc(replyEvent);
	free(replyEvent);
	if (err != noErr)
		return nil;

	return [[[ETTrack alloc] initWithDescriptor:&trackDescriptor] retain];
}

- (BOOL) isSpecialKind;
{
	NSLog (@"WARNING: not yet implemented");
	return NO;
}

- (BOOL) isCached;
{
	NSLog (@"WARNING: not yet implemented");
	return NO;
}

@end
