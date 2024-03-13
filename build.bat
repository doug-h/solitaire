@echo off

if not exist "build" mkdir build

set cxxflags=-std=c++23 -sINITIAL_MEMORY=256KB -sUSE_SDL=2 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sGL_TESTING
set warnings=-Wall -Wpedantic -Wsign-conversion -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set debugflags=-sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2 -fno-omit-frame-pointer -g 
set releaseflags=-DNDEBUG -O3

echo em++ src/main.cpp -o build/main.js %cxxflags% %debugflags% %warnings%
em++ src/main.cpp -o build/main.js %cxxflags% %releaseflags%
