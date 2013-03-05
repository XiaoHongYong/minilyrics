#pragma once

void PostNotifySearchEnd(int nResult);

void PostNotifyLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile);

void PostNotifyMessage(LPCTSTR szMessage, bool bShowActivityIndicator, bool bInfoMsg);

void PostNotifyLyricsChanged();
