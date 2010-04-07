
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
		else if (nCmdId == Glass.ID_HIDE_WND_FRAME)
		{
			if (Glass.nHideFrameOnLostFocus > 0)
			{
				Glass.nHideFrameOnLostFocus = 0;
				mSkinWnd.setProperty("TranslucencyAlpha", "255");
			}
			else
			{
				Glass.nHideFrameOnLostFocus = 1;
				mSkinWnd.setProperty("TranslucencyAlpha", "0");
			}

			SkinSystem.writeProfileInt("HideFrameOnLostFocus", Glass.nHideFrameOnLostFocus);
		
			// mSkinWnd.setProperty("TranslucencyAlphaOnActive", "255");
			// mSkinWnd.setProperty("TranslucencyAlphaOnHover", "255");
		}
		else if (nCmdId == Glass.ID_MOUSE_HOVER_SHOW_WND)
		{
			if (Glass.nShowFrameOnMouseHover > 0)
			{
				Glass.nShowFrameOnMouseHover = 0;
				mSkinWnd.setProperty("TranslucencyAlphaOnHover", "0");
			}
			else
			{
				Glass.nShowFrameOnMouseHover = 1;
				mSkinWnd.setProperty("TranslucencyAlphaOnHover", "255");
			}

			SkinSystem.writeProfileInt("ShowFrameOnMouseHover", Glass.nShowFrameOnMouseHover);
		}
		else if (nCmdId == Glass.CMD_ON_STATIC_LYR_OPEN)
		{
			mSkinWnd.setUIObjVisible(Glass.CID_STATIC_LYR_CTRL, true);
			return true;
		}
		else if (nCmdId == Glass.CMD_ON_SYNC_LYR_OPEN)
		{
			mSkinWnd.setUIObjVisible(Glass.CID_STATIC_LYR_CTRL, false);
			return true;
		}
		
		return false;
	}
	public void onCreate(SkinWnd wnd)
	{
		mSkinWnd = wnd;
		UnitTest.message("onCreate()");

		if (Glass.nHideFrameOnLostFocus > 0)
			mSkinWnd.setProperty("TranslucencyAlpha", "0");
		else
			mSkinWnd.setProperty("TranslucencyAlpha", "255");

		if (Glass.nShowFrameOnMouseHover > 0)
			mSkinWnd.setProperty("TranslucencyAlphaOnHover", "255");
		else
			mSkinWnd.setProperty("TranslucencyAlphaOnHover", "0");
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
	public static int ID_HIDE_WND_FRAME;
	public static int ID_MOUSE_HOVER_SHOW_WND;
	public static int nHideFrameOnLostFocus, nShowFrameOnMouseHover;
	public static int CMD_ON_STATIC_LYR_OPEN;
	public static int CMD_ON_SYNC_LYR_OPEN;
	public static int CID_STATIC_LYR_CTRL;

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
		ID_HIDE_WND_FRAME = SkinSystem.getCommandID("ID_HIDE_WND_FRAME");
		ID_MOUSE_HOVER_SHOW_WND = SkinSystem.getCommandID("ID_MOUSE_HOVER_SHOW_WND");
		CMD_ON_STATIC_LYR_OPEN = SkinSystem.getCommandID("CMD_ON_STATIC_LYR_OPEN");
		CMD_ON_SYNC_LYR_OPEN = SkinSystem.getCommandID("CMD_ON_SYNC_LYR_OPEN");
		CID_STATIC_LYR_CTRL = SkinSystem.getCommandID("CID_STATIC_LYR_CTRL");

		nHideFrameOnLostFocus = SkinSystem.getProfileInt("HideFrameOnLostFocus", 1);
		nShowFrameOnMouseHover = SkinSystem.getProfileInt("ShowFrameOnMouseHover", 1);

		static GlassSkinWndEvent	skinWndEvent = new GlassSkinWndEvent();
		
		SkinSystem.registerSkinWndEvent("minilyrics", skinWndEvent);
	}
}
