#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface _MDAVPlayer : NSObject 
{
	AVPlayer			*mPlayer;
	class CMDAVPlayer	*mMDAVPlayer;
}

@end

#import "StdAfx.h"
#import "IMPlayer.h"
#import "MDAVPlayer.h"
#import "MPTime.h"
#import "MPlayer.h"


static void *AVSPPlayerItemStatusContext = &AVSPPlayerItemStatusContext;
static void *AVSPPlayerRateContext = &AVSPPlayerRateContext;


@implementation _MDAVPlayer

- (id)init:(CMDAVPlayer *)player
{
	self = [super init];
	if (self) {
		mPlayer = [[AVPlayer alloc] init];
		mMDAVPlayer = player;

		[self addObserver:self forKeyPath:@"player.rate" options:NSKeyValueObservingOptionNew context:AVSPPlayerRateContext];
		[self addObserver:self forKeyPath:@"player.currentItem.status" options:NSKeyValueObservingOptionNew context:AVSPPlayerItemStatusContext];
	}

	return self;
}

- (bool)play:(NSString *)file
{
	// Create an asset with our URL, asychronously load its tracks, its duration, and whether it's playable or protected.
	// When that loading is complete, configure a player to play the asset.
	AVURLAsset *asset = [AVAsset assetWithURL:[NSURL fileURLWithPath:file]];

	if (![asset isPlayable] || [asset hasProtectedContent])
		return false;

	// Create a new AVPlayerItem and make it our player's current item.
	AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
	[mPlayer replaceCurrentItemWithPlayerItem:playerItem];
	
	return true;
}

- (void)pause
{
	[mPlayer pause];
}

- (void)unpause
{
	[mPlayer play];
}

- (void)stop
{
	[mPlayer pause];
	[mPlayer seekToTime:CMTimeMake(0, 1)];
}

- (AVPlayer *)player
{
	return mPlayer;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if (context == AVSPPlayerItemStatusContext)
	{
		AVPlayerStatus status = [[change objectForKey:NSKeyValueChangeNewKey] integerValue];
		BOOL enable = NO;
		switch (status)
		{
			case AVPlayerItemStatusUnknown:
				break;
			case AVPlayerItemStatusReadyToPlay:
				enable = YES;
				break;
			case AVPlayerItemStatusFailed:
				break;
		}
		
		if (!enable)
		{
			if (mMDAVPlayer->GetMPlayer())
				mMDAVPlayer->GetMPlayer()->NotifyEod(mMDAVPlayer, ERR_OK);
		}
	}
	else if (context == AVSPPlayerRateContext)
	{
//		float rate = [[change objectForKey:NSKeyValueChangeNewKey] floatValue];
//		if (rate != 1.f)
//		{
//			[[self playPauseButton] setTitle:@"Play"];
//		}
//		else
//		{
//			[[self playPauseButton] setTitle:@"Pause"];
//		}
	}
	else
	{
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

- (void)close
{
	[[self player] pause];
	[self removeObserver:self forKeyPath:@"player.rate"];
	[self removeObserver:self forKeyPath:@"player.currentItem.status"];
}

@end

//////////////////////////////////////////////////////////////////////////

CMDAVPlayer::CMDAVPlayer(void)
{
	OBJ_REFERENCE_INIT;

	Init();
}

CMDAVPlayer::~CMDAVPlayer(void)
{
}

LPCXSTR CMDAVPlayer::GetDescription()
{
	return _T("AVPlayer");
}

LPCXSTR CMDAVPlayer::GetFileExtentions()
{
	return _T(".mp3|MP3 files|.mp4|MP4 files|.wma|WMA files|.mp2|MP2 files");
}

MLRESULT CMDAVPlayer::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;
	
	AVURLAsset *asset = [AVAsset assetWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String: pInput->GetSource()]]];

	if (![asset isPlayable] || [asset hasProtectedContent])
		return ERR_NOT_SUPPORT;

	CMTime duration = [asset duration];
	if (duration.timescale > 0)
		pMedia->SetAttribute(MA_DURATION, (int)(duration.value * 1000 / duration.timescale));

/*
	m_player = [[AVPlayer alloc] init];
	
	CMPAutoPtr<IWMPMedia>	media;
	HRESULT hr = m_collection->add((BSTR)pInput->GetSource(), &media);
	if (FAILED(hr))
		return HResultToMLResult(hr);

	double duration;
	hr = media->get_duration(&duration);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_DURATION, (int)(duration * 1000));

	bstr_s	artist, album, title;

	BSTR	pbstrItemName = SysAllocString(L"Artist");
	hr = media->getItemInfo(pbstrItemName, &artist);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_ARTIST, artist.c_str());

	pbstrItemName = SysAllocString(L"Album");
	hr = media->getItemInfo(pbstrItemName, &album);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_ALBUM, album.c_str());

	pbstrItemName = SysAllocString(L"Title");
	hr = media->getItemInfo(pbstrItemName, &title);
	SysFreeString(pbstrItemName);
	if (SUCCEEDED(hr))
		pMedia->SetAttribute(MA_TITLE, title.c_str());
*/
	return ERR_OK;
}

//
// decode media file related methods
//

bool CMDAVPlayer::IsSeekable()
{
	return true;
}

bool CMDAVPlayer::IsUseOutputPlug()
{
	return false;
}

MLRESULT CMDAVPlayer::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[(_MDAVPlayer*)m_player play: [NSString stringWithUTF8String:pInput->GetSource()]];

	return ERR_OK;
}

MLRESULT CMDAVPlayer::Pause()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[(_MDAVPlayer*)m_player pause];

	return ERR_OK;
}

MLRESULT CMDAVPlayer::Unpause()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[(_MDAVPlayer*)m_player unpause];

	return ERR_OK;
}

MLRESULT CMDAVPlayer::Stop()
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[(_MDAVPlayer*)m_player stop];
	
	return ERR_OK;
}

uint32 CMDAVPlayer::GetLength()
{
	if (!IsOK())
		return 0;

	AVPlayerItem *item = [[(_MDAVPlayer*)m_player player] currentItem];
	if (item == nil)
		return 0;

	CMTime duration = [item duration];
	return (uint32)(duration.value * 1000 / duration.timescale);
}

MLRESULT CMDAVPlayer::Seek(uint32 nPos)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[[(_MDAVPlayer*)m_player player] seekToTime:CMTimeMake(nPos, 1000)];

	return ERR_OK;
}

uint32 CMDAVPlayer::GetPos()
{
	if (!IsOK())
		return 0;

	CMTime time = [[(_MDAVPlayer*)m_player player] currentTime];

	if (time.timescale == 0)
		return 0;
	return (uint32)(time.value * 1000 / time.timescale);
}

MLRESULT CMDAVPlayer::SetVolume(int nVolume, int nBanlance)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	[[(_MDAVPlayer*)m_player player] setVolume:nVolume / 100.0f];

	return ERR_OK;
}

bool CMDAVPlayer::Init()
{
	m_player = [[_MDAVPlayer alloc] init:this];

	return true;
}

MLRESULT CMDAVPlayer::DoDecode(IMedia *pMedia)
{
	if (!IsOK())
		return ERR_DECODER_INIT_FAILED;

	MLRESULT					nRet;
	CXStr						strMedia;
	CMPAutoPtr<IMediaInput>		pInput;
	CMPTime		timePlayed;

	nRet = pMedia->GetSourceUrl(&strMedia);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pPlayer->m_pluginMgrAgent.NewInput(strMedia.c_str(), &pInput);
	if (nRet != ERR_OK)
		return nRet;

	GetMediaInfo(m_pPlayer, pInput, pMedia);

	nRet = Play(m_pPlayer, pInput);
	if (nRet != ERR_OK)
	{
		if (nRet == ERR_MI_NOT_FOUND && pMedia->GetID() != MEDIA_ID_INVALID)
		{
			// the source can't be opened, set it as deleted.
			m_pPlayer->m_pMediaLib->SetDeleted(&pMedia);
		}
		goto R_FAILED;
	}

	if (m_pPlayer->m_nVolume != -1)
	{
		if (m_pPlayer->m_bMute)
			SetVolume(0, 0);
		else
			SetVolume(m_pPlayer->m_nVolume, m_pPlayer->m_nBalance);
	}

	// set the play time
	timePlayed.GetCurrentTime();
	pMedia->SetAttribute(MA_TIME_PLAYED, (int)timePlayed.m_time);

	if (pMedia->GetID() == MEDIA_ID_INVALID && m_pPlayer->IsAutoAddToMediaLib())
		m_pPlayer->m_pMediaLib->Add(pMedia);

	// If media info in media library isn't up to date,
	// update to media library.
	if (pMedia->GetID() != MEDIA_ID_INVALID && !pMedia->IsInfoUpdatedToMediaLib())
	{
		m_pPlayer->m_pMediaLib->UpdateMediaInfo(pMedia);
	}

	return ERR_OK;

R_FAILED:
	return nRet;
}

bool CMDAVPlayer::IsOK()
{
	return m_player != NULL && [(_MDAVPlayer*)m_player player] != nil;
}
