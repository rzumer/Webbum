@ECHO OFF

REM Basic deployment script for Qt components in Windows builds.

IF "%1"=="" GOTO usage
IF "%1"=="-?" GOTO usage
IF "%1"=="-Help" GOTO usage
IF NOT EXIST "%~1" GOTO usage

SET BIT_ARCH=64
echo.%2 | findstr /C:"32" 1>nul
IF NOT ERRORLEVEL 1 SET BIT_ARCH=32
echo.%2 | findstr /C:"86" 1>nul
IF NOT ERRORLEVEL 1 SET BIT_ARCH=32

REM Set the following variables based on your environment.
SET QT_ROOT=C:\Applications\Qt
SET QT_VERSION=5.11.1
SET COMPILER32=msvc2015
SET COMPILER64=msvc2017_64

REM The following may not be necessary when compiling through MinGW. Check for warnings on deployment.
SET VC_INSTALL_DIR_32=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
SET VC_INSTALL_DIR_64=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC
SET CL_EXE_PATH_32=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.14.26428\bin\Hostx64\x86
SET CL_EXE_PATH_64=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.14.26428\bin\Hostx64\x64

setlocal enableextensions enabledelayedexpansion
IF %BIT_ARCH%==32 (
	SET COMPILER=%COMPILER32%
	SET VCINSTALLDIR=%VC_INSTALL_DIR_32%
) ELSE (
	SET COMPILER=%COMPILER64%
	SET VCINSTALLDIR=%VC_INSTALL_DIR_64%
)

SET PATH=%PATH%;%CL_EXE_PATH_64%
SET BIN_PATH=%QT_ROOT%\%QT_VERSION%\%COMPILER%\bin
%BIN_PATH%\lrelease .\Webbum.pro
move .\*.qm "%~1"
%BIN_PATH%\windeployqt --release "%~1\Webbum.exe"
endlocal

copy ".\LICENSE.md" "%~1\LICENSE.txt"
GOTO fetch_ffmpeg
EXIT /B 0

:usage
ECHO Usage: %~nx0 ^<OUT_DIR^> [^<BIT_ARCH^>]
ECHO OUT_DIR: Release binary output directory
ECHO BIT_ARCH: 32-bit or 64-bit architecture (default: 64)
EXIT /B 1

EXIT /B 0

:fetch_ffmpeg
SET FFMPEG_DEPS=wget unzip
SET FFMPEG_VER=4.0.2
SET PACKAGE=ffmpeg-%FFMPEG_VER%-win%BIT_ARCH%-shared
SET FETCH_DIR=https://ffmpeg.zeranoe.com/builds/win%BIT_ARCH%/shared/

ECHO Fetching %PACKAGE%...

CALL :check_deps

wget -N %FETCH_DIR%%PACKAGE%.zip
unzip -j %PACKAGE%.zip "%PACKAGE%\bin\*" -d "%~1"
REM del %PACKAGE%.zip

ECHO Completed successfully.
EXIT /B 0

:check_deps
FOR %%D IN (%FFMPEG_DEPS%) DO (
	%%D /? 2> NUL
	IF ERRORLEVEL 9009 (
		1>&2 ECHO Missing dependency: %%D
		EXIT /B 1
	)
)
EXIT /B 0
