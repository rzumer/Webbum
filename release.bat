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

SET QT_ROOT=C:\Applications\Qt
SET QT_VERSION=5.11.1
SET COMPILER=msvc2017
SET BIN_PATH=%QT_ROOT%\%QT_VERSION%\%COMPILER%_%BITARCH%\bin
%BIN_PATH%\lrelease .\Webbum.pro
move .\*.qm "%~1"
%BIN_PATH%\windeployqt --release "%~1"
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
