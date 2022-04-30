#!/bin/bash

cd src
    gcc -c ../lib/*.cpp 
cd ..

g++ $1.cpp src/*.o -lpthread -o $1.out
./$1.out 
rm ./$1.out