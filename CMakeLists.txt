cmake_minimum_required(VERSION 3.16)

project(xg-timer DESCRIPTION "timer")

#complate arguments
set(CMAKE "/home/ezgaoro/workspace/tools/cmake-3.17.3/bin/cmake")
set(CMAKE_C_COMPILER "/home/ezgaoro/workspace/gcc-11.1.0/bin/gcc")
set(CMAKE_CXX_COMPILER "/home/ezgaoro/workspace/gcc-11.1.0/bin/g++")

set(COMPILE_WARNINGS "-Wall")
set(COMPILE_OPTIMIZATION "-Wl,--as-needed -O2")
set(COMPILE_DEBUG "-g -ggdb")

set(C_COMPILE_FLAGS "")
set(CXX_COMPILE_FLAGS "-std=c++20 -fPIC ${COMPILE_WARNINGS} ${COMPILE_OPTIMIZATION} ${COMPILE_DEBUG}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILE_FLAGS}")

#project arguments
set(XG_TIMER_PROJ_TOP $ENV{XGTIMER_REPOTOP})
if(NOT OUT)
    set(OUT "${XG_TIMER_PROJ_TOP}/install")
endif()
set(CMAKE_INSTALL_PREFIX ${OUT})

option(BUILD_UT "Build unittest" ON)
option(BUILD_DEMO "Build test program" ON)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)

#test
if(BUILD_DEMO)
    add_subdirectory(demo)
endif()

#ut
if(BUILD_UT)
    add_subdirectory(unittest)
endif()
