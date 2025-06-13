@echo "logicserver update"
svn update 
cd ../
cd common
svn update 

@echo "DBServer update"
cd ..
cd DBServer
svn update 

@echo "gamecommmon/include update"
echo "update gamecommmon/include"
cd ..
cd ..
cd ..
cd GameCommon
cd cpp
cd include
svn update 

@echo "update gamecommmon/libs"
echo "update gamecommmon/libs"
cd ..
cd libs
svn update 

@echo off





