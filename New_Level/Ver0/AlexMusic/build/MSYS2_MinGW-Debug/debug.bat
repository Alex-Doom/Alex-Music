@echo off
setlocal
set "PATH=%~dp0;%PATH%"
set "QT_PLUGIN_PATH=%~dp0"
cd /d "%~dp0"

:: Запускаем с выводом в консоль (без start)
AlexMusic.exe
pause