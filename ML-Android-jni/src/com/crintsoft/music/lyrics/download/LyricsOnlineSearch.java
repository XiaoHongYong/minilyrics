package com.crintsoft.music.lyrics.download;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.DefaultHttpClient;

import android.content.Context;
import android.util.Log;


public class LyricsOnlineSearch {

	private static final String TAG = "Search";

	public static native byte[] encodeSearchRequest(String artist, String title, boolean onlyMatched);
	public static native String decodeSearchReesult(byte data[], String file, String artist, String title);

	private static final String	MLS_SEARCH_URL = "http://search.crintsoft.com/searchlyrics.htm";

	private static final boolean USE_ENCRYPTED_MSG = true;

	@SuppressWarnings("serial")
	public static class LyrDownloadException extends Exception {

		public int		mFailedStringId = 0;
		public String 	mDetailedMsg = null;

		public LyrDownloadException(int failedStringId) {
			super();
			mFailedStringId = failedStringId;
		}

		public LyrDownloadException(int failedStringId, String detailedMsg) {
			super();
			mFailedStringId = failedStringId;
			mDetailedMsg = detailedMsg;
		}
		
		public String getDescription(Context ctx) {
			String str = ctx.getString(mFailedStringId);
			if (mDetailedMsg != null) {
				str += "\n\n";
				str += mDetailedMsg;
			}
			return str;
		}
	}

	public static byte[] DownloadFile(String url) throws LyrDownloadException, ClientProtocolException, IOException
	{
		DefaultHttpClient httpClient = new DefaultHttpClient();
    	HttpResponse response = null;

		HttpGet httpGet = new HttpGet(url);
		response = httpClient.execute(httpGet);

    	if (response.getStatusLine().getStatusCode() != 200) {
    		Log.e(TAG, "Failed to download lyrics, StatusCode not 200: " + response.getStatusLine().toString() + url);
    		throw new IOException("HttpStatusCode is: " + response.getStatusLine().getStatusCode());
		}

		HttpEntity retEntity = response.getEntity();
		ByteArrayOutputStream	retStream = new ByteArrayOutputStream();
		retEntity.writeTo(retStream);
		return retStream.toByteArray();
	}

	public static String Search(String file, String artist, String title) throws ClientProtocolException, IOException 
	{
		if (artist == null)
			artist = "";
		if (title == null)
			return null;

    	//
    	// Search lyrics online
    	//
		DefaultHttpClient httpClient = new DefaultHttpClient();
		HttpPost httpPost = new HttpPost(MLS_SEARCH_URL);

		httpPost.setHeader("User-Agent", "MiniLyrics4Android");

		// Construct XML search command string
		HttpEntity	entityMsg = new ByteArrayEntity(encodeSearchRequest(artist, title, true));
    	httpPost.setEntity(entityMsg);

		HttpResponse response;

		// Send command
		response = httpClient.execute(httpPost);

    	if (response.getStatusLine().getStatusCode() != 200) {
    		Log.e(TAG, "Failed to download lyrics, StatusCode not 200: " + response.getStatusLine().toString());
    		throw new IOException("HttpStatusCode is: " + response.getStatusLine().getStatusCode());
		}

		HttpEntity retEntity = response.getEntity();
		ByteArrayOutputStream	retStream = new ByteArrayOutputStream();
		retEntity.writeTo(retStream);

		String result = decodeSearchReesult(retStream.toByteArray(), file, artist, title);

		return result;
	}

}
