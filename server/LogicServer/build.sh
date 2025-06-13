#! /bin/bash

cd script/tolua++/
./../../../../common/cpp/libs/src/tolua++/tolua++ -n server -o serverInterface.cpp serverInterface.scp
if [ $? -ne 0 ]; then
	echo "fail to generate serverinterface.cpp!!!"
	exit 
fi
cd ../../
exit 0
if [ -n "$1" ]; then
	BuiltType=$1
else
	BuiltType=Release
fi 

rm -rf CMakeCache.txt cmake_install.cmake CMakeFiles/

cmake -DCMAKE_BUILD_TYPE=${BuiltType} ./
make -j4

rm -rf CMakeCache.txt CMakeFiles/

