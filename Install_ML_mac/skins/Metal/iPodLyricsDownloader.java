
public class iPodLyricsDownloaderWndEvent extends SkinWndEvent {

	public SkinWnd mSkinWnd;

	// Return true, the command will not be passed down.
	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
		// UnitTest.message("onCreate()");
	}
	
	public boolean onCommand(int nCmdId)
	{
		return false;
	}

	public void onActivate(boolean bActived)
	{
		if (bActived)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_LYR_TB);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_LYR_TB);
			// UnitTest.message("CID_AM_SHOW_LYR_TB");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_LYR_TB);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_LYR_TB);
			// UnitTest.message("CID_AM_HIDE_LYR_TB");
		}
	}

	public void onMouseActive(boolean bMouseActive)
	{
		if (bMouseActive)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_LYR_TB);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_LYR_TB);
			// UnitTest.message("CID_AM_SHOW_LYR_TB");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_LYR_TB);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_LYR_TB);
			// UnitTest.message("CID_AM_HIDE_LYR_TB");
		}
	}

}

public class MiniLyrics
{
	public static int CID_AM_HIDE_LYR_TB;
	public static int CID_AM_SHOW_LYR_TB;

	public static void main()
	{
		// UnitTest.message("MiniLyrics main()");

		CID_AM_HIDE_LYR_TB = SkinSystem.getCommandID("CID_AM_HIDE_LYR_TB");
		CID_AM_SHOW_LYR_TB = SkinSystem.getCommandID("CID_AM_SHOW_LYR_TB");

		static iPodLyricsDownloaderWndEvent	skinWndEvent = new iPodLyricsDownloaderWndEvent();

		SkinSystem.registerSkinWndEvent("iPodLyricsDownloader", skinWndEvent);
	}
}
