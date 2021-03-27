CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Atrc)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

############## optional submodules

OPTION(USE_EMBREE              "use embree for finding triangle mesh intersection" OFF)
OPTION(USE_OIDN                "use oidn denoiser"                                 OFF)
OPTION(BUILD_GUI               "build graphics user interface"                     OFF)
OPTION(BUILD_EDITOR            "build scene editor"                                OFF)
OPTION(BUILD_CLI               "build cmd-line launcher"                           ON)

############## CXX properties

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Options: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

############## embree

IF(USE_EMBREE)
    FIND_PACKAGE(embree 3.0 REQUIRED)
ENDIF()

############## oidn

IF(USE_OIDN)
    ADD_DEFINITIONS(-DUSE_OIDN)
    FIND_PACKAGE(OpenImageDenoise REQUIRED)
ENDIF()

############## qt

IF(BUILD_GUI OR BUILD_EDITOR)
    FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Widgets)
    SET_PROPERTY(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP auto_gen)
    SET(CMAKE_AUTOMOC ON)
    SET(CMAKE_AUTOUIC ON)
    SET(CMAKE_AUTORCC ON)
ENDIF()

############## im3d

IF(BUILD_EDITOR)
    ADD_SUBDIRECTORY(lib/im3d)
    SET_TARGET_PROPERTIES(Im3D PROPERTIES FOLDER "ThirdParty")
ENDIF()

############## necessory submodules

SET(MISC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/misc/)

ADD_SUBDIRECTORY(lib/spdlog)
ADD_SUBDIRECTORY(lib/utility)

TARGET_COMPILE_DEFINITIONS(AGZUtils PUBLIC AGZ_UTILS_SSE)
SET_TARGET_PROPERTIES(spdlog AGZUtils PROPERTIES FOLDER "ThirdParty")

ADD_SUBDIRECTORY(src/tracer)
ADD_SUBDIRECTORY(src/factory)

IF(BUILD_CLI)
    ADD_SUBDIRECTORY(src/cli)
ENDIF()

IF(BUILD_GUI OR BUILD_EDITOR)
	ADD_SUBDIRECTORY(src/gui_common)
ENDIF()

IF(BUILD_GUI)
    ADD_SUBDIRECTORY(src/gui)
ENDIF()

IF(BUILD_EDITOR)
    ADD_SUBDIRECTORY(src/editor)
ENDIF()
