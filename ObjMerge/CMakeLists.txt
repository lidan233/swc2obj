cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

project(mergeOBJ)

include_directories(./)
include_directories(../external/glm/)
include_directories(../external/cxxopts/include)


ADD_EXECUTABLE(mergeOBJ ObjMerger.cpp ObjMerger.h objMerge.cpp)
