#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
export PATH=/ucrt64/bin:$PATH

gcc $(pkg-config --cflags gtk+-3.0) \
    -Iinclude \
    -o busmate_gui.exe \
    src/main.c \
    src/gui.c \
    src/admin.c \
    src/file_handler.c \
    src/seat_allocation.c \
    src/auth.c \
    src/student_features.c \
    src/driver_features.c \
    src/reports.c \
    $(pkg-config --libs gtk+-3.0)

printf 'BusMate GUI build completed: %s\n' "$(pwd)/busmate_gui.exe"
