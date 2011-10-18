#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../gfx/gfx.h"
#import "../../GfxRaw/GfxRaw.h"
#import "../../Utils/Utils.h"
#import "helper.h"

tstring GetLyricsFolder()
{
	tstring strLyrDlFolder = GetAppDataDir();
	strLyrDlFolder += _T("lyrics") _T(SZ_DIR_SLASH);

	return strLyrDlFolder;
}

BOOL SetFileNoReadOnly(LPCTSTR szFile)
{
	return TRUE;
}
