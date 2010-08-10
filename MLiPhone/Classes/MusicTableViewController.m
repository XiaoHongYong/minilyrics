
#import "MusicTableViewController.h"
#import "MainViewController.h"

@implementation MusicTableViewController

static NSString *kCellIdentifier = @"MusicCell";

@synthesize delegate;					// The main view controller is the delegate for this class.
@synthesize mediaItemCollectionTable;	// The table shown in this class's view.
@synthesize clearPlaylistButton;
@synthesize musicPlayer;
@synthesize currentPlaying;

#define ROW_HEIGHT 60

// Configures the table view.
- (void) viewDidLoad {

    [super viewDidLoad];

	NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
						   selector: @selector (handle_NowPlayingItemChanged:)
							   name: MPMusicPlayerControllerNowPlayingItemDidChangeNotification
							 object: musicPlayer];

	[self.clearPlaylistButton setTitle: NSLocalizedString (@"Clearlocal", @"clear play list")];
	
    self.view.backgroundColor = [UIColor groupTableViewBackgroundColor];
	self.mediaItemCollectionTable.rowHeight = ROW_HEIGHT;
}


// When the user taps Done, invokes the delegate's method that dismisses the table view.
- (IBAction) doneShowingMusicList: (id) sender {

	[self.delegate musicTableViewControllerDidFinish: self];	
}

- (IBAction) clearCollection: (id) sender
{
	[self.delegate updatePlayerQueueWithMediaCollection: nil];
	[self.mediaItemCollectionTable reloadData];
}

// Configures and displays the media item picker.
- (IBAction) showMediaPicker: (id) sender {

	MPMediaPickerController *picker =
		[[MPMediaPickerController alloc] initWithMediaTypes: MPMediaTypeAnyAudio];
	
	picker.delegate						= self;
	picker.allowsPickingMultipleItems	= YES;
	picker.prompt						= NSLocalizedString (@"Add songs to play", "Prompt in media item picker");
	
	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleDefault animated:YES];

	[self presentModalViewController: picker animated: YES];
	[picker release];
}


// Responds to the user tapping Done after choosing music.
- (void) mediaPicker: (MPMediaPickerController *) mediaPicker didPickMediaItems: (MPMediaItemCollection *) mediaItemCollection {

	[self dismissModalViewControllerAnimated: YES];
	[self.delegate updatePlayerQueueWithMediaCollection: mediaItemCollection];
	[self.mediaItemCollectionTable reloadData];

	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleBlackOpaque animated:YES];
}


// Responds to the user tapping done having chosen no music.
- (void) mediaPickerDidCancel: (MPMediaPickerController *) mediaPicker {

	[self dismissModalViewControllerAnimated: YES];

	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleBlackOpaque animated:YES];
}



#pragma mark Table view methods________________________

#define ARTIST_TAG 1
#define TITLE_TAG 2
#define NO_TAG	3
#define DURATION_TAG 4
#define PLAYING_TAG	5

#define NO_COLUMN_OFFSET 2.0
#define NO_COLUMN_WIDTH 40.0

#define PLAYING_IMAGE_SIDE 16.0

#define PLAYING_COLUMN_OFFSET 10
#define PLAYING_COLUMN_WIDTH PLAYING_IMAGE_SIDE

#define DURATION_COLUMN_OFFSET (320.0 - 52.0)
#define DURATION_COLUMN_WIDTH 50.0

#define TITLE_COLUMN_OFFSET (NO_COLUMN_OFFSET + NO_COLUMN_WIDTH + 2.0)
#define TITLE_COLUMN_WIDTH  (DURATION_COLUMN_OFFSET - TITLE_COLUMN_OFFSET - 5.0)

#define ARTIST_COLUMN_OFFSET TITLE_COLUMN_OFFSET
#define ARTIST_COLUMN_WIDTH  (320.0 - ARTIST_COLUMN_OFFSET)

#define MAIN_FONT_SIZE 16.0
#define LABEL_HEIGHT 20.0

#define SMALL_FONT_SIZE 13.0
#define SMALL_LABLE_HEIGHT 16.0


- (UITableViewCell *)tableViewCellWithReuseIdentifier:(NSString *)identifier {
	
	/*
	 Create an instance of UITableViewCell and add tagged subviews
	 */
	
	UITableViewCell *cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier] autorelease];
	
	UILabel *label;
	CGRect rect;

	// Lable for row index.
	rect = CGRectMake(NO_COLUMN_OFFSET, (ROW_HEIGHT/2.0 - LABEL_HEIGHT), NO_COLUMN_WIDTH, LABEL_HEIGHT);
	label = [[UILabel alloc] initWithFrame:rect];
	label.tag = NO_TAG;
	label.font = [UIFont boldSystemFontOfSize:MAIN_FONT_SIZE];
	label.adjustsFontSizeToFitWidth = YES;
	label.textAlignment = UITextAlignmentCenter;
	[cell.contentView addSubview:label];
	label.highlightedTextColor = [UIColor whiteColor];
	[label release];

	// Image view for now playing
	rect = CGRectMake(PLAYING_COLUMN_OFFSET, ROW_HEIGHT / 2.0 + 7, PLAYING_IMAGE_SIDE, PLAYING_IMAGE_SIDE);
	UIImageView *imageView = [[UIImageView alloc] initWithFrame:rect];
	imageView.tag = PLAYING_TAG;
	[cell.contentView addSubview:imageView];
	[imageView release];
	
	// Lable for Title
	rect = CGRectMake(TITLE_COLUMN_OFFSET, (ROW_HEIGHT/2.0 - LABEL_HEIGHT), TITLE_COLUMN_WIDTH, LABEL_HEIGHT);
	label = [[UILabel alloc] initWithFrame:rect];
	label.tag = TITLE_TAG;
	label.font = [UIFont boldSystemFontOfSize:MAIN_FONT_SIZE];
	[cell.contentView addSubview:label];
	label.highlightedTextColor = [UIColor whiteColor];
	[label release];

	// Lable for artist and album
	rect = CGRectMake(ARTIST_COLUMN_OFFSET, ROW_HEIGHT / 2.0 + (ROW_HEIGHT / 2.0 - SMALL_LABLE_HEIGHT) / 2.0, ARTIST_COLUMN_WIDTH, SMALL_LABLE_HEIGHT);
	label = [[UILabel alloc] initWithFrame:rect];
	label.tag = ARTIST_TAG;
	label.font = [UIFont systemFontOfSize:SMALL_FONT_SIZE];
	[cell.contentView addSubview:label];
	label.highlightedTextColor = [UIColor whiteColor];
	label.textColor = [UIColor grayColor];
	[label release];

	// Lable for duration
	rect = CGRectMake(DURATION_COLUMN_OFFSET, (ROW_HEIGHT/2.0 - LABEL_HEIGHT), DURATION_COLUMN_WIDTH, LABEL_HEIGHT);
	label = [[UILabel alloc] initWithFrame:rect];
	label.tag = DURATION_TAG;
	label.font = [UIFont boldSystemFontOfSize:MAIN_FONT_SIZE];
	label.textAlignment = UITextAlignmentRight;
	[cell.contentView addSubview:label];
	label.highlightedTextColor = [UIColor whiteColor];
	label.adjustsFontSizeToFitWidth = YES;
	[label release];
	
	return cell;
}

NSString *formatPlayingTime(int n)
{
	int		minutes, seconds, hours;
	
	seconds = n % 60;
	minutes = n / 60;
	
	hours = minutes / 60;
	minutes %= 60;
	
	if (hours > 0)
		return [NSString stringWithFormat:@"%d:%02d:%02d", hours, minutes, seconds];
	else
		return [NSString stringWithFormat:@"%02d:%02d", minutes, seconds];
}

- (void)configureCell:(UITableViewCell *)cell forIndexPath:(NSIndexPath *)indexPath {

	NSInteger row = [indexPath row];
	MainViewController *mainViewController = (MainViewController *) self.delegate;
	MPMediaItemCollection *currentQueue = mainViewController.userMediaItemCollection;
	MPMediaItem *anItem = (MPMediaItem *)[currentQueue.items objectAtIndex: row];
	
	if (!anItem)
		return;

	UILabel *label;

	// No.
	label = (UILabel *)[cell viewWithTag:NO_TAG];
	label.text = [NSString stringWithFormat:@"%d.", row];

	// Title
	label = (UILabel *)[cell viewWithTag:TITLE_TAG];
	label.text = [anItem valueForProperty:MPMediaItemPropertyTitle];

	// Artist - album
	NSString *artist = [anItem valueForProperty:MPMediaItemPropertyArtist];
	NSString *album = [anItem valueForProperty:MPMediaItemPropertyAlbumTitle];
	label = (UILabel *)[cell viewWithTag:ARTIST_TAG];
	if (artist == nil || [artist length] == 0)
		label.text = album;
	else if (album == nil || [album length] == 0)
		label.text = artist;
	else
		label.text = [NSString stringWithFormat:@"%@ - %@", album, artist];
	
	// Duration
	label = (UILabel *)[cell viewWithTag:DURATION_TAG];
	label.text = formatPlayingTime(
			[[anItem valueForProperty:MPMediaItemPropertyPlaybackDuration] intValue]);
	
	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];
	
	// Set the image.
	UIImageView *imageView = (UIImageView *)[cell viewWithTag:PLAYING_TAG];
	if ([currentItem isEqual:anItem]) {
		imageView.image = [UIImage imageNamed:@"playing.png"];
		currentPlaying = row;
	}
	else
		imageView.image = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];

	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];
	MainViewController *mainViewController = (MainViewController *) self.delegate;
	MPMediaItemCollection *currentQueue = mainViewController.userMediaItemCollection;
	int i = 0;
	for (i = 0; i < currentQueue.items.count; i++) {
		MPMediaItem *anItem = (MPMediaItem *)[currentQueue.items objectAtIndex: i];
		if ([currentItem isEqual:anItem])
			break;
	}
	
	if (i == currentQueue.items.count)
		return;
	
	NSIndexPath *indexPath = [NSIndexPath indexPathForRow:i inSection:0];

	[mediaItemCollectionTable scrollToRowAtIndexPath:indexPath
									atScrollPosition:UITableViewScrollPositionMiddle
											animated:NO];
}

// To learn about using table views, see the TableViewSuite sample code  
//		and Table View Programming Guide for iPhone OS.

- (NSInteger) tableView: (UITableView *) table numberOfRowsInSection: (NSInteger)section {

	MainViewController *mainViewController = (MainViewController *) self.delegate;
	MPMediaItemCollection *currentQueue = mainViewController.userMediaItemCollection;
	return [currentQueue.items count];
}

- (UITableViewCell *) tableView: (UITableView *) tableView cellForRowAtIndexPath: (NSIndexPath *) indexPath {

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: kCellIdentifier];
	
	if (cell == nil) {
		cell = [self tableViewCellWithReuseIdentifier:kCellIdentifier];
	}
	
	[self configureCell:cell forIndexPath:indexPath];

	[tableView deselectRowAtIndexPath: indexPath animated: YES];
	
	return cell;
}

//	 To conform to the Human Interface Guidelines, selections should not be persistent --
//	 deselect the row after it has been selected.
- (void) tableView: (UITableView *) tableView didSelectRowAtIndexPath: (NSIndexPath *) indexPath {

	[tableView deselectRowAtIndexPath: indexPath animated: YES];
	
	// Playing selected item
	MainViewController *mainViewController = (MainViewController *) self.delegate;
	MPMediaItemCollection *currentQueue = mainViewController.userMediaItemCollection;
	MPMediaItem *anItem = (MPMediaItem *)[currentQueue.items objectAtIndex: [indexPath row]];
	
	if (anItem) {
		MPMusicPlaybackState state = [musicPlayer playbackState];
		[musicPlayer setNowPlayingItem:anItem];
		if (state == MPMusicPlaybackStatePlaying)
			[musicPlayer play];
	}
}

#pragma mark Application state management_____________
// Standard methods for managing application state.
- (void)didReceiveMemoryWarning {

	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {

	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {

    [super dealloc];
}


#pragma mark Music notification handlers__________________

// When the now-playing item changes, update the media item artwork and the now-playing label.
- (void) handle_NowPlayingItemChanged: (id) notification {
	
	UITableViewCell *cell;
	NSIndexPath *indexPath = [NSIndexPath indexPathForRow:currentPlaying inSection:0];

	cell = [mediaItemCollectionTable cellForRowAtIndexPath: indexPath];
	if (cell != nil) {
		UIImageView *imageView = (UIImageView *)[cell viewWithTag:PLAYING_TAG];
		imageView.image = nil;
	}
	
	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];
	MainViewController *mainViewController = (MainViewController *) self.delegate;
	MPMediaItemCollection *currentQueue = mainViewController.userMediaItemCollection;
	int i = 0;
	for (i = 0; i < currentQueue.items.count; i++) {
		MPMediaItem *anItem = (MPMediaItem *)[currentQueue.items objectAtIndex: i];
		if ([currentItem isEqual:anItem])
			break;
	}
	
	if (i == currentQueue.items.count)
		return;

	indexPath = [NSIndexPath indexPathForRow:i inSection:0];
	cell = [mediaItemCollectionTable cellForRowAtIndexPath: indexPath];
	if (cell == nil)
		return;

	// Set the image.
	UIImageView *imageView = (UIImageView *)[cell viewWithTag:PLAYING_TAG];
	imageView.image = [UIImage imageNamed:@"playing.png"];

	currentPlaying = i;
}

@end
