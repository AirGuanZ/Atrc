CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

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

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
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

ADD_SUBDIRECTORY(lib/spdlog)
ADD_SUBDIRECTORY(lib/utility)
SET_TARGET_PROPERTIES(spdlog AGZUtils PROPERTIES FOLDER "ThirdParty")

ADD_SUBDIRECTORY(src/core/tracer)
ADD_SUBDIRECTORY(src/app/cli)

############## glfw && material explorer

IF(BUILD_MATERIAL_EXPLORER)
    INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-glfw")
	INCLUDE("${PROJECT_SOURCE_DIR}/cmake/cmake-glew")
	FIND_PACKAGE(OpenGL REQUIRED)
    ADD_SUBDIRECTORY(src/app/material_explorer)
ENDIF()
