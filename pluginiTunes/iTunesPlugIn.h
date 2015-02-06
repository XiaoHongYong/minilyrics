
#ifndef ITUNESPLUGIN_H
#define ITUNESPLUGIN_H

#include "iTunesVisualAPI.h"


#define	kTVisualPluginCreator			'dxhy'

#define	kTVisualPluginMajorVersion		2
#define	kTVisualPluginMinorVersion		0
#define	kTVisualPluginReleaseStage		finalStage
#define	kTVisualPluginNonFinalRelease	0

void		GetVisualName(ITUniStr255 name);
OSStatus	RegisterVisualPlugin(PluginMessageInfo * messageInfo);

void		DrawVisual(HWND destPort);

#endif
