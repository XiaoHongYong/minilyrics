/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.crintsoft.music;

import java.io.File;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.crintsoft.music.lyrics.LyricsData;
import com.crintsoft.music.lyrics.LyricsLocalSearch;
import com.crintsoft.music.lyrics.download.LyricsOnlineSearch;
import com.example.hellojni.R;

/**
 * This is used to test MiniLyrics jni for Android.
 *
 */
public class LyricsApplication extends Activity implements OnClickListener
{
	private static final String TAG = "HelloJni";
	private static final String APP_NAME = "MiniLyrics";
	
	private String			FILE = "/sdcard/任贤齐 - 沧海一声笑.mp3";
	private String			TITLE = "沧海一声笑";
	private String			ARTIST = "任贤齐";
	
	private TextView		mTvMsg;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mTvMsg = (TextView)findViewById(R.id.msg);
        
        findViewById(R.id.encode_search).setOnClickListener(this);
        findViewById(R.id.search).setOnClickListener(this);
        findViewById(R.id.decode_search).setOnClickListener(this);

        File profile = getFileStreamPath("MiniLyrics.ini");

        init(this, profile.getParent(), APP_NAME, profile.getAbsolutePath(), getAppNameLong());

        // TODO: Converted lyrics tag name is incorrect for UTF8.
        mTvMsg.setText(new LyricsData().testReadLyricsSource("/sdcard/1.mp3", "/sdcard/1.lrc"));
        // tv.setText(new LyricsData().testReadLyricsSource("/sdcard/任贤齐 - 沧海一声笑.mp3", "song://id3v2/sylt/eng"));
        
        LyricsLocalSearch.setSearchOptions("/sdcard/lyrics", "/sdcard/lyrics");
        
        mTvMsg.setText(LyricsLocalSearch.search(FILE, ARTIST, TITLE));
    }

    public native String  init(Context ctx, String workingFolder, String appName, String profile, String appNameLong);

    static {
        System.loadLibrary("hello-jni");
    }

	@Override
	public void onClick(View v) {
		mTvMsg.setText("");
		switch (v.getId()) {
		case R.id.encode_search:
			byte data[] = LyricsOnlineSearch.encodeSearchRequest(ARTIST, TITLE, true);
			
			mTvMsg.setText(byte2hex(data));
			break;
		case R.id.search:
			searchOnline();
			break;
		case R.id.decode_search:
			break;
		default:
			break;
		}
	}
	
	private void searchOnline() {
		new Thread() {
			@Override
			public void run() {
				try {
					String result = LyricsOnlineSearch.Search(FILE, ARTIST, TITLE);
					setMsgBackground(result);
				} catch (final Exception e) {
					Log.e(TAG, "Failed to search", e);

					setMsgBackground(e.toString());
				}
			}
		}.start();
	}
	
	private void setMsgBackground(final String msg) {
		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Log.e(TAG, msg);
				mTvMsg.setText(msg);
			}
		});
	}

	public static String byte2hex(byte[] bytes) {
		StringBuffer sb = new StringBuffer();
		for (byte b : bytes) {
			sb.append(String.format("%02x", (b & 0xff)));
		}

		return sb.toString();
	}

	private String getAppNameLong() {
		try {
			PackageInfo info = getPackageManager().getPackageInfo(getPackageName(), 0);
			return "MiniLyrics " + info.versionName + " for Android";
		} catch (NameNotFoundException e) {
			return "Unknown";
		}
	}

}
