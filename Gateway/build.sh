#!/bin/bash
cd $(cd "$(dirname "$0")" && pwd)
srcDIR=`pwd`
temp="/../temp"
name="/BuildGateway"
#cd gameworld/script/tolua++
#tolua++ -n server -o serverInterface.cc serverInterface.scp
#mkdir $srcDIR$temp
#mkdir $srcDIR$temp$name
mkdir temp_BuildGateway
cd temp_BuildGateway
if [ -n "$1" ]; then
        BuiltType=$1
else
        BuiltType=Release
fi
cmake .. -DCMAKE_BUILD_TYPE=${BuiltType}
make -j2

#svn log -l 1 > web/server.version
