#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
export PATH=/ucrt64/bin:$PATH

./build_gui.sh

exec ./busmate_gui.exe
