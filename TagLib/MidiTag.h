#pragma once

namespace MidiTag {

#define MIDI_HEADER		"MThd"
#define TRACK_HEADER	"MTrk"

#define TRUNK_HEADER_LEN	8

	struct FileHeader
	{
		byte trunkId[4];
		byte size;

	};

	struct TrackTrunk
	{
		uint32 sizeOfData;
		uint32 offset;
	};

	typedef list<TrackTrunk>	ListTrackTrunk;

	struct MidiEvent
	{
		int    DeltaTime;     /** The time between the previous event and this on */
		int    StartTime;     /** The absolute time this event occurs */
		bool   HasEventflag;  /** False if this is using the previous eventflag */
		byte   EventFlag;     /** NoteOn, NoteOff, etc.  Full list is in class MidiFile */
		byte   Channel;       /** The channel this event occurs on */
	};

	typedef list<MidiEvent>	ListMidiEvent;


	// Midi Events
	enum MidiEventType
	{
		MET_NOTE_OFF		= 0x80,
		MET_NOTE_ON			= 0x90,
		MET_KEY_PRESSURE	= 0xA0,
		MET_CONTROL_CHANGE  = 0xB0,
		MET_PROGRAM_CHANGE  = 0xC0,
		MET_CHANNEL_PRESSURE= 0xD0,
		MET_PITCH_BEND      = 0xE0,
		MET_SYSEX1          = 0xF0,
		MET_SYSEX2          = 0xF7,
		MET_META            = 0xFF,
	};

	// The Meta Events Type
	enum MetaEventType
	{
		MAT_SEQUENCE      = 0x0,
		MAT_TEXT          = 0x1,
		MAT_COPYRIGHT     = 0x2,
		MAT_SEQUENCE_NAME = 0x3,
		MAT_INSTRUMENT    = 0x4,
		MAT_LYRIC         = 0x5,
		MAT_MARKER        = 0x6,
		MAT_END_OF_TRACK  = 0x2F,
		MAT_TEMPO         = 0x51,
		MAT_SMPTEOFFSET   = 0x54,
		MAT_TIMESIGNATURE = 0x58,
		MAT_KEYSIGNATURE  = 0x59,
	};

};

class CMidiTag
{
public:
	CMidiTag();
	~CMidiTag();

	int Open(LPCTSTR szFile);
	void Close();

	int ListLyrics(VecString &vLyrNames);
	int GetLyrics(CLyricsData &lyricsData);

	bool HasLyrics();

protected:
	int ReadTrackTrunk(MidiTag::TrackTrunk &trunk);

protected:
	CFileEx		m_file;

};
