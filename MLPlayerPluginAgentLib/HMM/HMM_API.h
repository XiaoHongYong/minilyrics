#ifndef __HMM_API_H
#define __HMM_API_H

#define HMM_WND_CLASS		"THeliumMainForm"
#define WM_HMM_IPC			WM_USER


const int IPC_PLAY = 100;
// This message will start to play the active track in Helium Music Manager¡¯s Active Playlist

const int IPC_PLAYPAUSE = 101;
// This message will pause the playback (if playing), otherwise it will resume the playback (if paused)
	
const int IPC_FORCEPAUSE = 102;
// This message will force the playback to pause

const int IPC_STOP = 103;
// This message will stop the playback

const int IPC_NEXT = 104;
// This message will skip to the next track in the Active Playlist and start to play this track

const int IPC_PREVIOUS = 105;
// This message will skip to the previous track in the Active Playlist and start to play this track

const int IPC_GET_CURRENT_TRACK = 110;
// This message will retrieve information about the track that is currently being played.
// Remarks: See the callback topic above
// Details: The information is in the following form:
//
// Title (including subtitle and remix if available) 
// Artist (including secondary artist if available) 
// Album 
// Genre 
// Year 
// Comment 
// Current Track 
// The length of this file in seconds 
// The filename including the full path for this fileThe rating of this file (0-10) 
// The full path and filename to the assigned album cover 
// Composer 
// Lyricist 
// Publisher 
// Conductor 
// Producer 
// Copyright 
//
// When a stream is played in Helium Music Manager, the following data will be returned:
// 
// Title (retrieved from the stream-data) 
// Additional stream information 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// The full URL to the stream 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
// Reserved for future use 
//
// All fields are separated by a tab character.

const int IPC_GET_DURATION = 113;
// This message will retrieve the duration for the track currently being played
// Result: The duration (in seconds)

const int IPC_SET_POSITION = 114;
// This message will set the position for the track currently being played
// Parameters: The new position in wParam of the message. (In seconds)

const int IPC_IS_PLAYING = 115;
// This message will let you know if Helium Music Manager is currently playing a song.
// Result: 1 if playing, 0 otherwise

const int IPC_IS_PAUSED = 116;
// This message will let you know if Helium Music Manager currently is paused.
// Result: 1 if paused, 0 otherwise.

const int IPC_GET_LIST_ITEM = 119;

const int IPC_SET_CALLBACK_HWND = 120;
// This message will set the callback window handle used for callback messages. 
// Parameters: Your window handle in wParam of the message
// Result: 0 if the callback could not be set, 1 if the callback was successfully set.

const int IPC_GET_POSITION = 122;
// This message will return the current playing position in seconds

const int IPC_TRACK_CHANGED_NOTIFICATION = 123;
// If the client has registered itself with a callback to Helium Music Manager, 
// Helium Music Manager will notify the client using this message 
// each time a new track is played in Helium Music Manager.

const int IPC_GET_PLAYER_STATE = 125;
// This message will return the current state of the player.
// Result: 
// 0.     Player is stopped
// 1.     Player is playing
// 2.     Player is paused

const int IPC_PLAYER_STATE_CHANGED_NOTIFICATION = 126;
// When the playing state in Helium Music Manager changes, this message will be sent to a client.
// Parameters: The new player state (as described for IPC_GET_PLAYER_STATE) in wParam

const int IPC_SHUTDOWN_NOTIFICATION = 129;
// This message will be sent to a client when Helium Music Manager shuts down.

const int IPC_NEW_COVER_NOTIFICATION = 800;
// When a new album cover is either added/updated or removed from Helium Music Managers Internal Player, a connected client will be notified with this message.
// The full path and filename to the new cover is contained in the <COPYDATA_STRUCT>.

#endif

