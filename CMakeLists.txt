CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

INCLUDE(ExternalProject)

PROJECT(Atrc)

############## optional submodules

OPTION(USE_EMBREE              "use embree for finding triangle mesh intersection" OFF)
OPTION(USE_OIDN                "use oidn denoiser"                                 OFF)
OPTION(BUILD_GUI               "build graphics user interface"                     OFF)
OPTION(BUILD_EDITOR            "build scene editor"                                OFF)

############## CXX properties

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Options: Debug Release RelWithDebInfo MinSizeRel." FORCE)
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

############## qt

IF(BUILD_GUI OR BUILD_EDITOR)
    FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Widgets)
    SET_PROPERTY(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP auto_moc)
    SET(CMAKE_AUTOMOC ON)
    SET(CMAKE_AUTOUIC ON)
    SET(CMAKE_AUTORCC ON)
ENDIF()

############## necessory submodules

SET(MISC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/misc/)

ADD_SUBDIRECTORY(lib/spdlog)
ADD_SUBDIRECTORY(lib/utility)
SET_TARGET_PROPERTIES(spdlog AGZUtils PROPERTIES FOLDER "ThirdParty")

ADD_SUBDIRECTORY(src/core/tracer)
ADD_SUBDIRECTORY(src/app/cli)

IF(BUILD_GUI)
    ADD_SUBDIRECTORY(src/app/gui)
ENDIF()

IF(BUILD_EDITOR)
    ADD_SUBDIRECTORY(src/app/editor)
ENDIF()
