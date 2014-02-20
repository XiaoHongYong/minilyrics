# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_EXPORT_CFLAGS := -D_ANDROID=1
LOCAL_CFLAGS := -D_ANDROID=1 -I../../base

LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections
LOCAL_CFLAGS += -ffunction-sections -fdata-sections 
LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_MODULE    := MiniLyrics
LOCAL_SRC_FILES := MiniLyrics.cpp \
	../../base/android/android.cpp ../../base/android/Thread.cpp ../../base/android/CharEncoding.cpp \
	../../base/Error.cpp ../../base/idstring.cpp ../../base/safestr.cpp ../../base/stringex.cpp \
	../../base/StrPrintf.cpp ../../base/CharEncodingBase.cpp ../../base/FileApiBase.cpp	\
	../../base/linux/Event.cpp ../../base/linux/fileapi.cpp ../../base/BaseDate.cpp \
	../../base/linux/Mutex.cpp \
	../../base/linux/misc.cpp ../../base/linux/OSError.cpp ../../base/TextFile.cpp  \
	../../base/MapString.cpp ../../base/FileEx.cpp \
	../../Utils/Log.cpp ../../Utils/MLBinXMLParser.cpp ../../Utils/SimpleXML.cpp ../../Utils/XMLWriter.cpp\
	../../Utils/SimpleXMLParser.cpp ../../Utils/XCharSeparatedValues.cpp ../../Utils/UUEncode.cpp \
	../../Utils/linux/Profile.cpp ../../Utils/linux/App.cpp ../../Utils/VersionNumber.cpp \
	../../Utils/digest.cpp ../../Utils/md5.cpp \
	../../ID3Lib/ID3Helper.cpp		../../ID3Lib/ID3v1.cpp		../../ID3Lib/ID3v2.cpp		\
	../../ID3Lib/ID3v2Frame.cpp		../../ID3Lib/ID3v2FrameParser.cpp	\
	../../TagLib/ID3v2IF.cpp		../../TagLib/LyricsData.cpp		../../TagLib/M4aTag.cpp		\
	../../TagLib/MidiTag.cpp		../../TagLib/TagLib.cpp	../../TagLib/MP3InfoReader.cpp \
	../../MLLib/Lyrics3v2.cpp 		../../MLLib/LrcParser.cpp 		../../MLLib/LyrTimeStamp.cpp \
	../../MLLib/LyricsSearch.cpp 	../../MLLib/MLData.cpp 			../../MLLib/HelperFun.cpp \
	../../MLLib/SncParser.cpp 		../../MLLib/LrcTag.cpp 			../../MLLib/LyricsKeywordFilter.cpp \
	../../MLLib/SrtParser.cpp 		../../MLLib/LyrScrollActionRecorder.cpp 	../../MLLib/MLProfile.cpp \
	iconv.c wctomb.c
	#../../MLProtocol/AppAnalyticsClient.cpp ../../MLProtocol/HttpClient.cpp \
	#../../MLProtocol/HttpProtocol.cpp ../../MLProtocol/ClientCom.cpp\
	#../../MLProtocol/MLProtocol.cpp ../../MLProtocol/MLUtils.cpp ../../MLProtocol/NetSDK.cpp \
	#

include $(BUILD_SHARED_LIBRARY)
