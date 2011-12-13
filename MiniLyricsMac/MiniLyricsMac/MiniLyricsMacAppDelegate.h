//
//  MiniLyricsMacAppDelegate.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MiniLyricsMacAppDelegate : NSObject <NSApplicationDelegate> {
	NSWindow *window;
	NSPersistentStoreCoordinator *__persistentStoreCoordinator;
	NSManagedObjectModel *__managedObjectModel;
	NSManagedObjectContext *__managedObjectContext;
}

@property (assign) IBOutlet NSWindow *window;

@property (nonatomic, retain, readonly) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (nonatomic, retain, readonly) NSManagedObjectModel *managedObjectModel;
@property (nonatomic, retain, readonly) NSManagedObjectContext *managedObjectContext;

- (IBAction)saveAction:(id)sender;

- (IBAction)newWindow:(id)sender;

@end
