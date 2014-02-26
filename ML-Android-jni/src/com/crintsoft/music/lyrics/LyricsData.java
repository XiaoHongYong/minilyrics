package com.crintsoft.music.lyrics;


public class LyricsData {

	/**
	 * Open lyrics source (file or embedded lyrics), convert to general LRC file format.
	 * @param lyricsSrc
	 * @return
	 */
	private native String readLyricsSource(String mediaFile, String lyricsSrc);

	public String testReadLyricsSource(String mediaFile, String lyricsSrc) {
		return readLyricsSource(mediaFile, lyricsSrc);
	}

}
