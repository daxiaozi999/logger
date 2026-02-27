# 设置 CMake 标准
cmake_minimum_required(VERSION 3.10)

# 设置项目名
project(logger)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找 src 下的所有源文件
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 设置静态库输出路径
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# 创建静态库
add_library(logger STATIC ${SOURCES})