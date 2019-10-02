CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

IF(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  SET(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
ENDIF()

INCLUDE(ExternalProject)

PROJECT(Atrc)

############## optional submodules

OPTION(USE_EMBREE              "use embree for finding triangle mesh intersection" OFF)
OPTION(USE_OIDN                "use oidn denoiser"                                 OFF)
OPTION(BUILD_MATERIAL_EXPLORER "build material explorer or not"                    OFF)

############## CXX properties

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Options: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET(CMAKE_CXX_STANDARD 17)

IF(MSVC)
    ADD_DEFINITIONS(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)
ENDIF()

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -pthread")
    SET(STDFS_LD_LIB "c++fs")
    SET(CMAKE_EXE_LINKER_FLAGS "-ldl")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    SET(STDFS_LD_LIB "stdc++fs")
    SET(CMAKE_EXE_LINKER_FLAGS "-ldl")
    MESSAGE(WARNING "g++ configuration is untested")
ENDIF()

############## embree

IF(USE_EMBREE)
    ADD_DEFINITIONS(-DUSE_EMBREE)
    INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-embree") 
ENDIF()

############## oidn

IF(USE_OIDN)
    ADD_DEFINITIONS(-DUSE_OIDN)
    INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-oidn")
ENDIF()

############## necessory submodules

SET(MISC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/misc/)

ADD_SUBDIRECTORY(lib/utility/src)

ADD_SUBDIRECTORY(src/core/common)
ADD_SUBDIRECTORY(src/core/rasterizer/)
ADD_SUBDIRECTORY(src/core/tracer)

ADD_SUBDIRECTORY(src/app/cli)
ADD_SUBDIRECTORY(src/app/cli_ras)
ADD_SUBDIRECTORY(src/app/obj_to_scene)

############## glfw && material explorer

IF(BUILD_MATERIAL_EXPLORER)
    INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-glfw")
	INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-glew")
	FIND_PACKAGE(OpenGL REQUIRED)
    ADD_SUBDIRECTORY(src/app/material_explorer)
ENDIF()
