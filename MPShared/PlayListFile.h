#pragma once

bool SavePlaylistAsM3u(IPlaylist *playList, LPCTSTR szFile);

bool LoadPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile);

bool LoadM3uPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile);
bool LoadPlsPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile);
bool LoadWplPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile);
