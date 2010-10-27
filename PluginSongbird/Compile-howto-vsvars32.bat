@SET VSINSTALLDIR=e:\Program Files\Microsoft Visual Studio 9.0
@SET VCINSTALLDIR=e:\Program Files\Microsoft Visual Studio 9.0\VC
@SET FrameworkDir=C:\WINDOWS\Microsoft.NET\Framework
@SET FrameworkVersion=v2.0.50727
@SET Framework35Version=v3.5
@if "%VSINSTALLDIR%"=="" goto error_no_VSINSTALLDIR
@if "%VCINSTALLDIR%"=="" goto error_no_VCINSTALLDIR

@echo Setting environment for using Microsoft Visual Studio 2008 x86 tools.

@call :GetWindowsSdkDir

@if not "%WindowsSdkDir%" == "" (
	set "PATH=%WindowsSdkDir%bin;%PATH%"
	set "INCLUDE=%WindowsSdkDir%include;%INCLUDE%"
	set "LIB=%WindowsSdkDir%lib;%LIB%"
)


@rem
@rem Root of Visual Studio IDE installed files.
@rem
@set DevEnvDir=e:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE

@set PATH=e:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE;e:\Program Files\Microsoft Visual Studio 9.0\VC\BIN;e:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools;C:\WINDOWS\Microsoft.NET\Framework\v3.5;C:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;e:\Program Files\Microsoft Visual Studio 9.0\VC\VCPackages;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin;E:\WinDDK\7600.16385.1\bin;%PATH%
@set INCLUDE=e:\Program Files\Microsoft Visual Studio 9.0\VC\ATLMFC\INCLUDE;e:\Program Files\Microsoft Visual Studio 9.0\VC\INCLUDE;C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include;E:\WinDDK\7600.16385.1\inc;E:\WinDDK\7600.16385.1\inc\api;%INCLUDE%
@set LIB=e:\Program Files\Microsoft Visual Studio 9.0\VC\ATLMFC\LIB;e:\Program Files\Microsoft Visual Studio 9.0\VC\LIB;C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib;E:\WinDDK\7600.16385.1\lib\wxp\i386;%LIB%
@set LIBPATH=C:\WINDOWS\Microsoft.NET\Framework\v3.5;C:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;e:\Program Files\Microsoft Visual Studio 9.0\VC\ATLMFC\LIB;e:\Program Files\Microsoft Visual Studio 9.0\VC\LIB;C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib;E:\WinDDK\7600.16385.1\lib\wxp\i386;%LIBPATH%

@goto end

:GetWindowsSdkDir
@call :GetWindowsSdkDirHelper HKLM > nul 2>&1
@if errorlevel 1 call :GetWindowsSdkDirHelper HKCU > nul 2>&1
@if errorlevel 1 set WindowsSdkDir=%VCINSTALLDIR%\PlatformSDK\
@exit /B 0

:GetWindowsSdkDirHelper
@for /F "tokens=1,2*" %%i in ('reg query "%1\SOFTWARE\Microsoft\Microsoft SDKs\Windows" /v "CurrentInstallFolder"') DO (
	if "%%i"=="CurrentInstallFolder" (
		SET "WindowsSdkDir=%%k"
	)
)
@if "%WindowsSdkDir%"=="" exit /B 1
@exit /B 0

:error_no_VSINSTALLDIR
@echo ERROR: VSINSTALLDIR variable is not set. 
@goto end

:error_no_VCINSTALLDIR
@echo ERROR: VCINSTALLDIR variable is not set. 
@goto end

:end
