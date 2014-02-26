package com.crintsoft.music.lyrics;


public class LyricsLocalSearch {

	public static native String search(String file, String artist, String title);

	public static native void setSearchOptions(String downloadDir, String lyricsDir);
	
}
