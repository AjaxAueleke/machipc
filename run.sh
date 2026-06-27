#!/bin/bash
#
# Build-and-run helper: compiles the shared socket library into src/, links it
# with the requested target, runs the binary in the foreground, then removes it.
# For a persistent build of every target, use the Makefile instead (`make`).
#
# Usage: ./run.sh <target>   e.g. ./run.sh send  |  ./run.sh mach/mach_central_server

mkdir -p src
cd src
    gcc -c ../lib/*.cpp
cd ..

g++ "$1.cpp" src/*.o -lpthread -o "$1.out"
"./$1.out"
rm "./$1.out"
