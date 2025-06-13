rm -rf CMakeCache.txt CMakeFiles/ cmake_install.cmake
cmake -DCMAKE_BUILD_TYPE=Debug ./
make

rm -rf CMakeCache.txt CMakeFiles/ cmake_install.cmake
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ./
make 
