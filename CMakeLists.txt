CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

IF(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  SET(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
ENDIF()

INCLUDE(ExternalProject)

PROJECT(Atrc)

############## optional submodules

OPTION(USE_EMBREE     "use embree for finding triangle mesh intersection" OFF)
OPTION(USE_BCD        "use bcd denoiser"                                  OFF)
OPTION(USE_OIDN       "use oidn denoiser"                                 OFF)
OPTION(LUA_ENABLE_JIT "use luajit instead of lua"                         OFF)

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

	IF(MSVC)
		SET(EMBREE_IGNORE_CMAKE_CXX_FLAGS "")
	ELSE()
		SET(EMBREE_IGNORE_CMAKE_CXX_FLAGS "-DEMBREE_IGNORE_CMAKE_CXX_FLAGS=OFF")
	ENDIF()
	
	IF(NOT DEFINED EMBREE_MAX_ISA)
		SET(EMBREE_MAX_ISA AVX2)
	ENDIF()

	SET(Embree3_VERSION "3.5.2")
	ExternalProject_Add(
		Embree3
		PREFIX "${PROJECT_SOURCE_DIR}/lib/embree3"
		URL "https://github.com/embree/embree/archive/v${Embree3_VERSION}.tar.gz"
		URL_MD5 608072dba35b09b58e0b13e59699342a
		CMAKE_ARGS
			"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			"-DEMBREE_TUTORIALS=OFF"
			"-DEMBREE_FILTER_FUNCTION=OFF"
			"-DEMBREE_RAY_PACKETS=OFF"
			"-DEMBREE_TASKING_SYSTEM=INTERNAL"
			"-DEMBREE_GEOMETRY_QUAD=OFF"
			"-DEMBREE_GEOMETRY_CURVE=OFF"
			"-DEMBREE_GEOMETRY_SUBDIVISION=OFF"
			"-DEMBREE_GEOMETRY_INSTANCE=OFF"
			"-DEMBREE_GEOMETRY_USER=OFF"
			"-DEMBREE_GEOMETRY_POINT=OFF"
			"-DEMBREE_ISPC_SUPPORT=OFF"
			"-DEMBREE_STATIC_LIB=ON"
			"-DEMBREE_MAX_ISA=${EMBREE_MAX_ISA}"
			"${EMBREE_IGNORE_CMAKE_CXX_FLAGS}"
			"-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
			"-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
			"-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
		INSTALL_COMMAND ""
	)
	
	SET(EMBREE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3/include/")

	# 下面的一系列embree的静态库必须按此顺序链接
	IF(MSVC)
		SET(EMBREE_LIBRARIES
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/embree3.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/embree_sse42.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/embree_avx.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/embree_avx2.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/lexers.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/math.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/simd.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/tasking.lib"
			optimized "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Release/sys.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/embree3.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/embree_sse42.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/embree_avx.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/embree_avx2.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/lexers.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/math.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/simd.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/tasking.lib"
			debug     "${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/Debug/sys.lib"
			)
	ELSE()
		SET(EMBREE_LIBRARIES
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libembree3.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libembree_sse42.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libembree_avx.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libembree_avx2.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/liblexers.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libmath.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libsimd.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libtasking.a"
			"${PROJECT_SOURCE_DIR}/lib/embree3/src/Embree3-build/libsys.a"
			)
	ENDIF()
	
ENDIF()

############## bcd & eigen

IF(USE_BCD)
	ADD_DEFINITIONS(-DUSE_BCD)
	
	SET(Eigen3_VERSION "3.3.7" )
	ExternalProject_Add(
		Eigen3
		PREFIX "${PROJECT_SOURCE_DIR}/lib/eigen3"
		URL "https://github.com/eigenteam/eigen-git-mirror/archive/${Eigen3_VERSION}.tar.gz"
		URL_MD5 77a2c934eaf35943c43ee600a83b72df
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	SET(EIGEN3_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/eigen3/src/Eigen3)
	
	ADD_SUBDIRECTORY(lib/bcd)
ENDIF()

############## oidn

IF(USE_OIDN)
	ADD_DEFINITIONS(-DUSE_OIDN)

	IF(NOT DEFINED OIDN_PATH)
		MESSAGE(FATAL_ERROR "oidn path unspecified")
		EXIT()
	ENDIF()

	FIND_PACKAGE(OpenImageDenoise PATHS ${OIDN_PATH})

ENDIF()

############## lua

IF(LUA_ENABLE_JIT)
	SET(LUA_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/luajit/include)
	IF(MSVC)
		SET(LUA_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/luajit/lua51.lib)
	ELSE()
		SET(LUA_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/luajit/libluajit.so)
	ENDIF()
ELSE()
	ADD_SUBDIRECTORY(lib/lua)
	SET(LUA_LIBRARIES Lua)
ENDIF()

############## other submodules

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SET(MISC_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/misc/)

ADD_SUBDIRECTORY(lib/utility/src)

ADD_SUBDIRECTORY(src/tracer_utility)
ADD_SUBDIRECTORY(src/tracer)
ADD_SUBDIRECTORY(src/tracer_cshader)
ADD_SUBDIRECTORY(src/cli)
ADD_SUBDIRECTORY(src/obj_to_scene)

SUBDIRLIST(PLUGINS ${PROJECT_SOURCE_DIR}/src/c_materials)
MESSAGE("add material plugins: ${PLUGINS}")

FOREACH(subdir ${PLUGINS})
  ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/c_materials/${subdir})
ENDFOREACH()
