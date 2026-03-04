@echo off
setlocal enabledelayedexpansion

echo.
echo Fallout 2: Community Edition - Game Data Installer
echo.
echo This script copies the required retail Fallout 2 game files
echo into the game directory. You need a legitimate copy of Fallout 2
echo (GOG, Steam, or original CD).
echo.

if "%~1"=="" (
    echo Usage: %~nx0 ^<fallout2-install-directory^> [output-directory]
    echo.
    echo Examples:
    echo   %~nx0 "C:\GOG Games\Fallout 2"
    echo   %~nx0 "C:\Program Files (x86)\Steam\steamapps\common\Fallout 2"
    echo   %~nx0 "D:\" .
    exit /b 1
)

set "SOURCE=%~1"
set "DEST=%~2"
if "%DEST%"=="" set "DEST=%~dp0"

if not exist "%SOURCE%\master.dat" (
    echo ERROR: Could not find master.dat in %SOURCE%
    echo Make sure you point to your Fallout 2 install directory.
    exit /b 1
)

echo Source: %SOURCE%
echo Destination: %DEST%
echo.

echo Copying master.dat...
copy /Y "%SOURCE%\master.dat" "%DEST%\" >nul
echo Copying critter.dat...
copy /Y "%SOURCE%\critter.dat" "%DEST%\" >nul

echo Copying data\ folder...
if exist "%SOURCE%\data" (
    xcopy /E /I /Y "%SOURCE%\data" "%DEST%\data" >nul
)

REM Copy patch files if present
for %%f in ("%SOURCE%\patch*.dat") do (
    echo Copying %%~nxf...
    copy /Y "%%f" "%DEST%\" >nul
)

REM Preserve our custom configs - never overwrite
for %%c in (ddraw.ini f2_res.ini fallout2.cfg) do (
    if exist "%DEST%\%%c" (
        echo Keeping existing %%c ^(not overwritten^)
    ) else if exist "%SOURCE%\%%c" (
        echo Copying %%c...
        copy /Y "%SOURCE%\%%c" "%DEST%\" >nul
    )
)

echo.
echo === Game data installed to: %DEST% ===
echo You can now run the game!
