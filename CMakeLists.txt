cmake_minimum_required(VERSION 3.14)
project(SWC2Obj LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)

include(cmake/lidanBase.cmake)
include(cmake/LidanBuild.cmake)


SET(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(CONDIF_DIR "${CMAKE_SOURCE_DIR}/")

configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config/config.h"
)


ADD_SUBSRC(datadir data)
ADD_SUBSRC(swc2voldir swc2vol/)
ADD_SUBSRC(vol2objdir vol2obj)
ADD_SUBSRC(utildir util)

add_subdirectory("external/glm")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/data/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/swc2vol/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/vol2obj/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/util/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/"
#                    "${CMAKE_CURRENT_SOURCE_DIR}/config/"
                    "${CMAKE_CURRENT_BINARY_DIR}/config/"
                    )


add_executable(SWC2Obj ${datadir} ${swc2voldir} ${vol2objdir} ${utildir} main.cpp)

