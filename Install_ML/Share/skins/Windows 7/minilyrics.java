
public class MiniLyricsSkinWndEvent extends SkinWndEvent {

	public SkinWnd mSkinWnd;

	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
	}

	public boolean onCommand(int nCmdId)
	{
		return false;
	}

	public void onActivate(boolean bActived)
	{
		if (bActived)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			// UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			// UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

	public void onMouseActive(boolean bMouseActive)
	{
		if (bMouseActive)
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			// UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(MiniLyrics.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(MiniLyrics.CID_AM_HIDE_PLAY);
			// UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

}

public class MiniLyrics
{
	public static int CID_AM_HIDE_PLAY;
	public static int CID_AM_SHOW_PLAY;

	public static void main()
	{
		CID_AM_HIDE_PLAY = SkinSystem.getCommandID("CID_AM_HIDE_PLAY");
		CID_AM_SHOW_PLAY = SkinSystem.getCommandID("CID_AM_SHOW_PLAY");

		static MiniLyricsSkinWndEvent	skinWndEvent = new MiniLyricsSkinWndEvent();
		
		SkinSystem.registerSkinWndEvent("minilyrics", skinWndEvent);
	}
}
