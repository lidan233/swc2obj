cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

project(swc2Lobj)

include_directories(../external/glm/)
include_directories(../external/cxxopts/include)
include_directories(./)

ADD_EXECUTABLE(swc2Lobj swc2lobj.cpp ObjWriter.cpp ObjWriter.h SWCReader.cpp SWCReader.h based.h cmdline.h based.h)
