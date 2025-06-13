#! /bin/bash

# 编译模式
if [ -n "$1" ]; then
	BuiltType=$1
else
	BuiltType=Release
fi 

# 移除上次编译cache
rm -rf CMakeCache.txt cmake_install.cmake CMakeFiles/

#find /home -iname "*CMakeFiles*" -type d -exec rm -rf {} + ;
#find /home -iname "*CMakeCache*" -type f -exec rm -rf {} + ;
#find /home -iname "*cmake_install*" -type f -exec rm -rf {} + ;
#find /home -iname "makefile" -type f -exec rm -rf {} + ;

# 生成lua注册文件
cd $(cd "$(dirname "$0")" && pwd)
srcDIR=`pwd`
cd LogicServer/script/tolua++
./../../../../sdk/tolua++/tolua++ -n server -o serverInterface.cpp serverInterface.scp

#cd $srcDIR 
#mkdir temp_BuildAll
#cd temp_BuildAll
 

# 生成项目并编译
cd $srcDIR
cmake -DCMAKE_BUILD_TYPE=${BuiltType} ./
make -j4
 

#cmake .. -DCMAKE_BUILD_TYPE=${BuiltType}
#cmake -DCMAKE_BUILD_TYPE=${BuiltType} ./
#make -j4

# 记录版本
#svn log -l 1 > web/server.version
