@echo off
setlocal

set "PATH=C:\msys64\ucrt64\bin;%PATH%"
set "PKG_CONFIG_PATH=C:\msys64\ucrt64\lib\pkgconfig;C:\msys64\ucrt64\share\pkgconfig"

echo Getting GTK4 compiler flags...

for /f "delims=" %%I in ('pkg-config --cflags gtk4') do set "GTK_CFLAGS=%%I"
for /f "delims=" %%I in ('pkg-config --libs gtk4') do set "GTK_LIBS=%%I"

if not defined GTK_CFLAGS (
    echo ERROR: Could not find GTK4 compiler flags.
    pause
    exit /b 1
)

echo Building BusMate...

gcc src\main.c src\gui.c src\admin.c src\file_handler.c src\seat_allocation.c -o BusMate.exe -Iinclude %GTK_CFLAGS% %GTK_LIBS% -mwindows

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    pause
    exit /b 1
)

echo.
echo BUILD SUCCESSFUL!
BusMate.exe
pause