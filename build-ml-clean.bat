echo off

if EXIST "%VS90COMNTOOLS%\vsvars32.bat" call "%VS90COMNTOOLS%\vsvars32.bat"
if EXIST "C:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat" call "C:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat"
if EXIST "D:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat" call "D:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat"
if EXIST "E:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat" call "E:\Program Files\Microsoft Visual Studio 9\Common7\Tools\vsvars32.bat"

echo on

build.py MiniLyrics /clean

pause