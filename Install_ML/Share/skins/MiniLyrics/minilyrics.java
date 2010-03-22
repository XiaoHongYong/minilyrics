
public class GlassSkinWndEvent extends SkinWndEvent {

	public SkinWnd mSkinWnd;

	// Return true, the command will not be passed down.
	public boolean onCommand(int nCmdId)
	{
		UnitTest.message("onCommand()");
		UnitTest.message(nCmdId);
		
		if (nCmdId == Glass.CMD_AUTO_SHOW_LYR)
		{
			UnitTest.message("CMD_AUTO_SHOW_LYR");
			return true;
		}
		else if (nCmdId == Glass.CMD_SHOW_INFO_TEXT)
		{
			UnitTest.message("CMD_SHOW_INFO_TEXT");
			// mSkinWnd.setProperty("TranslucencyAlpha", "0");
			// mSkinWnd.setUIObjProperty(Glass.CID_TOGGLE_LYR_PL, "CmdToggle", "CID_INFOTEXT");
			return true;
		}
		
		return false;
	}
	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
		UnitTest.message("onCreate()");
	}

	public void onDestroy()
	{
		UnitTest.message("onDestroy()");
	}

	public void onActivate(boolean bActived)
	{
		if (bActived)
		{
			mSkinWnd.stopAnimation(Glass.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(Glass.CID_AM_SHOW_PLAY);
			UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(Glass.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(Glass.CID_AM_HIDE_PLAY);
			UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

	public void onMouseActive(boolean bMouseActive)
	{
		if (bMouseActive)
		{
			mSkinWnd.stopAnimation(Glass.CID_AM_HIDE_PLAY);
			mSkinWnd.startAnimation(Glass.CID_AM_SHOW_PLAY);
			UnitTest.message("CID_AM_SHOW_PLAY");
		}
		else
		{
			mSkinWnd.stopAnimation(Glass.CID_AM_SHOW_PLAY);
			mSkinWnd.startAnimation(Glass.CID_AM_HIDE_PLAY);
			UnitTest.message("CID_AM_HIDE_PLAY");
		}
	}

}

public class Glass
{
	public static int ID_PLAYPAUSE;
	public static int CMD_AUTO_SHOW_PL;
	public static int CMD_AUTO_SHOW_LYR;
	public static int CMD_SHOW_INFO_TEXT;
	public static int CID_TOGGLE_LYR_PL;
	public static int CID_BT_TOGGLE_PL_LYR;
	public static int CID_AM_HIDE_PLAY;
	public static int CID_AM_SHOW_PLAY;

	public static void main()
	{
		UnitTest.message("Glass main()");

		ID_PLAYPAUSE = SkinSystem.getCommandID("ID_PLAYPAUSE");
		CMD_AUTO_SHOW_PL = SkinSystem.getCommandID("CMD_AUTO_SHOW_PL");
		CMD_AUTO_SHOW_LYR = SkinSystem.getCommandID("CMD_AUTO_SHOW_LYR");
		CMD_SHOW_INFO_TEXT = SkinSystem.getCommandID("CMD_SHOW_INFO_TEXT");
		CID_TOGGLE_LYR_PL = SkinSystem.getCommandID("CID_TOGGLE_LYR_PL");
		CID_BT_TOGGLE_PL_LYR = SkinSystem.getCommandID("CID_BT_TOGGLE_PL_LYR");
		CID_AM_HIDE_PLAY = SkinSystem.getCommandID("CID_AM_HIDE_PLAY");
		CID_AM_SHOW_PLAY = SkinSystem.getCommandID("CID_AM_SHOW_PLAY");

		static GlassSkinWndEvent	skinWndEvent = new GlassSkinWndEvent();
		
		SkinSystem.registerSkinWndEvent("minilyrics", skinWndEvent);
	}
}
