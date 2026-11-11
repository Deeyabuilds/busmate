@echo off
setlocal
set PATH=C:\msys64\ucrt64\bin;%PATH%
set "OUTPUT=busmate.exe"
if not "%~1"=="" set "OUTPUT=%~1"
for /f "delims=" %%i in ('pkg-config --cflags gtk+-3.0') do set "GTK_CFLAGS=%%i"
for /f "delims=" %%i in ('pkg-config --libs gtk+-3.0') do set "GTK_LIBS=%%i"

if not defined GTK_CFLAGS (
    echo Failed to get GTK3 compiler flags. Check MSYS2 UCRT64 and pkg-config.
    exit /b 1
)
if not defined GTK_LIBS (
    echo Failed to get GTK3 linker flags. Check MSYS2 UCRT64 and pkg-config.
    exit /b 1
)

if not exist data mkdir data

gcc -std=c11 -Wall -Wextra -Iinclude ^
    -o "%OUTPUT%" ^
    src\main.c src\gui.c src\file_handler.c src\seat_allocation.c ^
    %GTK_CFLAGS% %GTK_LIBS% -mwindows
if errorlevel 1 exit /b %errorlevel%

echo BusMate GUI build completed successfully: %OUTPUT%
exit /b 0