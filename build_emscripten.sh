#!/bin/sh
source /etc/profile.d/emscripten.sh
emcc -s USE_SDL=2 -sINITIAL_MEMORY=79298560 -o output.html menue.cpp mixer.cpp panzer.cpp audio.cpp -O2 --preload-file res
