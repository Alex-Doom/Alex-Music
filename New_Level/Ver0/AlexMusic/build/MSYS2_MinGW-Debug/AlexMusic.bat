@echo off
setlocal
set "PATH=%~dp0;%PATH%"
set "QT_PLUGIN_PATH=%~dp0"

:: === FFmpeg нужен PATH к себе ===
set "PATH=C:\msys64\mingw64\bin;%PATH%"

cd /d "%~dp0"
start "" "AlexMusic.exe"