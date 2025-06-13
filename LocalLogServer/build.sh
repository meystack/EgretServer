#! /bin/bash

if [ -n "$1" ]; then
	BuiltType=$1
else
	BuiltType=Release
fi 

rm -rf CMakeCache.txt cmake_install.cmake CMakeFiles/

cmake -DCMAKE_BUILD_TYPE=${BuiltType} ./
make -j4

rm -rf CMakeCache.txt CMakeFiles/