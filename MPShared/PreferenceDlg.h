#pragma once

#include "PreferencePageBase.h"

void ShowPreferenceDialog(CSkinWnd *pParent, bool bFloatingLyr = false, PreferPageID preferPageId = PAGE_UI);

void RegisterPreferencePage(CSkinFactory *pSkinFactory);
