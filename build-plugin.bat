@echo off
@if "%1"=="" (
set CFG=Debug
) else (
set CFG=%1
)

call vcvars32.bat
@echo on

devenv.com MPlayer-Plugins\MDAPE\MDAPE.sln /build "%CFG%|Win32"
devenv.com MPlayer-Plugins\MDFlac\MDFlac.sln /build "%CFG%|Win32"
devenv.com MPlayer-Plugins\MDMP4\MDMP4.sln /build "%CFG%|Win32"
devenv.com MPlayer-Plugins\MDVorbis\MDVorbis.sln /build "%CFG%|Win32"
devenv.com MPlayer-Plugins\MDWMA2\MDWMA.sln /build "%CFG%|Win32"
