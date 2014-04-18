APP_PLATFORM := android-8

APP_STL := gnustl_static

ifeq ($(NDK_DEBUG),0)
	APP_ABI := all
else
	APP_ABI := armeabi-v7a
endif
