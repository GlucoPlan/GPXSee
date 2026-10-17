@echo off
setlocal enableextensions
title Build GPXSee (MSVC x64)

rem ============ SETTINGS (edit if needed) ============
rem  Path to the unpacked Qt (the folder that has bin\qmake.exe):
set "QTDIR=C:\Qt\6.9.2\msvc2022_64"
rem  What to build: release or debug
set "BUILD=release"
rem  Project file
set "PROJECT=gpxsee.pro"
rem ===================================================

rem --- go to the folder where this .bat lives (source root) ---
cd /d "%~dp0"

rem --- locate Visual Studio via vswhere ---
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo [ERROR] vswhere not found - Visual Studio is probably not installed.
  goto :fail
)

set "VSPATH="
for /f "usebackq tokens=* delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH (
  echo [ERROR] MSVC x64 C++ tools not found in Visual Studio.
  echo         Open "Visual Studio Installer" - Modify and enable
  echo         "Desktop development with C++".
  goto :fail
)

set "VCVARS=%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
  echo [ERROR] vcvars64.bat not found:
  echo         %VCVARS%
  goto :fail
)

rem --- check Qt ---
if not exist "%QTDIR%\bin\qmake.exe" (
  echo [ERROR] Qt not found: %QTDIR%\bin\qmake.exe
  echo         Fix the QTDIR variable at the top of this file.
  goto :fail
)

echo === Setting up MSVC x64 environment ===
call "%VCVARS%"
if errorlevel 1 goto :fail

set "PATH=%QTDIR%\bin;%PATH%"

echo.
echo === lrelease (translations) ===
lrelease "%PROJECT%"
if errorlevel 1 goto :fail

echo.
echo === qmake ===
qmake "%PROJECT%"
if errorlevel 1 goto :fail

echo.
echo === nmake %BUILD% ===
nmake %BUILD%
if errorlevel 1 goto :fail

echo.
echo ============================================================
echo  DONE. Executable: %CD%\%BUILD%\GPXSee.exe
echo.
echo  To make the exe run standalone, run once:
echo      windeployqt %BUILD%\GPXSee.exe
echo ============================================================
goto :end

:fail
echo.
echo *** Build stopped due to an error (see messages above). ***

:end
echo.
pause
endlocal
