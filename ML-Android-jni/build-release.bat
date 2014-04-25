call ndk-build NDK_DEBUG=0
if errorlevel 1 exit /b %errorlevel%

rmdir /S /Q "..\..\ml_android\ZikiPlayer\libs\armeabi"
rmdir /S /Q "..\..\ml_android\ZikiPlayer\libs\armeabi-v7a"
rmdir /S /Q "..\..\ml_android\ZikiPlayer\libs\x86"
rmdir /S /Q "..\..\ml_android\ZikiPlayer\libs\mips"
xcopy /S libs ..\..\ml_android\ZikiPlayer\libs