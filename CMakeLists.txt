# OpenTimelineIO needs CMake 3.18.2 or higher
CMAKE_MINIMUM_REQUIRED(VERSION 3.18.2)

# project
SET(VERSION "0.1.0")
SET(FW_NAME "ovi")

PROJECT(${FW_NAME} VERSION ${VERSION} LANGUAGES C CXX)
INCLUDE(GNUInstallDirs)

SET(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/${FW_NAME}/plugins" CACHE STRING
    "Install plugin libraries in this directory")
SET(PLUGIN_SRCS_DIR "${PLUGIN_INSTALL_DIR}/srcs")
SET(PLUGIN_MODELS_DIR "${PLUGIN_INSTALL_DIR}/models")

ADD_DEFINITIONS("-DPLUGIN_INSTALLED_DIR=\"${PLUGIN_INSTALL_DIR}\"")
ADD_DEFINITIONS("-DPLUGIN_SRC_DIR=\"${PLUGIN_SRCS_DIR}\"")
ADD_DEFINITIONS("-DPLUGIN_MODEL_DIR=\"${PLUGIN_MODELS_DIR}\"")
ADD_DEFINITIONS("-DINI_FILE_PATH=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}/${FW_NAME}/${FW_NAME}.ini\"")

OPTION(ENABLE_PYTHON "Enable Python." ON)
OPTION(BUILD_OVI_TOOLS "Build OVI tools." ON)

SET(FFMPEG_PKG libavutil libswscale libavcodec libavformat libswresample)
SET(SANITIZER_FLAGS " -fsanitize=address")
SET(COVERAGE_FLAGS " -fprofile-arcs -ftest-coverage")
SET(DEBUG_FLAGS " -O0 -g ${SANITIZER_FLAGS} ${COVERAGE_FLAGS}")

FIND_PACKAGE(Threads)

INCLUDE(FindPkgConfig)

IF(ENABLE_PYTHON)
    FIND_PACKAGE(Python3)
    PKG_CHECK_MODULES(PYTHON REQUIRED python-${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}-embed)
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${PYTHON_CFLAGS} -DOVI_ENABLE_PYTHON")
    LIST(APPEND EXTRA_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS})
    LIST(APPEND EXTRA_LDFLAGS ${PYTHON_LDFLAGS})
ENDIF(ENABLE_PYTHON)

# ffmpeg is still mandotary
PKG_CHECK_MODULES(FFMPEG REQUIRED ${FFMPEG_PKG})
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${FFMPEG_CFLAGS} -D__STDC_CONSTANT_MACROS")
LIST(APPEND EXTRA_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS})
LIST(APPEND EXTRA_LDFLAGS ${FFMPEG_LDFLAGS})

# opentimelineio is mandotary
PKG_CHECK_MODULES(IMATH REQUIRED Imath)
FIND_PACKAGE(OpenTime)
FIND_PACKAGE(OpenTimelineIO REQUIRED COMPONENTS OpenTime)
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS}")
LIST(APPEND EXTRA_INCLUDE_DIRS ${IMATH_INCLUDE_DIRS})
LIST(APPEND EXTRA_LDFLAGS opentimelineio)

# iniparser is optional
PKG_CHECK_MODULES(INIPARSER iniparser)
IF(NOT INIPARSER_FOUND)
    MESSAGE("iniparser not found, ovi will run as defaults.")
ELSE()
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${INIPARSER_CFLAGS} -DOVI_ENABLE_INIPARSER")
    LIST(APPEND EXTRA_INCLUDE_DIRS ${INIPARSER_INCLUDE_DIRS})
    LIST(APPEND EXTRA_LDFLAGS ${INIPARSER_LDFLAGS})
ENDIF()

SET(INC_DIR include)
INCLUDE_DIRECTORIES(${INC_DIR})

MESSAGE(STATUS "EXTRA_CFLAGS: " ${EXTRA_CFLAGS})
MESSAGE(STATUS "EXTRA_INCLUDE_DIRS: " ${EXTRA_INCLUDE_DIRS})
MESSAGE(STATUS "EXTRA_LDFLAGS: " ${EXTRA_LDFLAGS})

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Werror")
SET(CMAKE_C_FLAGS_DEBUG ${DEBUG_FLAGS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -std=c++17")
SET(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=${LIB_INSTALL_DIR}")

AUX_SOURCE_DIRECTORY(src SOURCES)
ADD_LIBRARY(${FW_NAME} SHARED ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(${FW_NAME} PUBLIC ${EXTRA_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${FW_NAME} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${EXTRA_LDFLAGS})

# include headers
SET(OVI_INCLUDE_FILES
# interface
    "${INC_DIR}/ovi.h"
    "${INC_DIR}/ovi_types.h"
# for plugins
    "${INC_DIR}/AttributeValidator.h"
    "${INC_DIR}/Log.h"
    "${INC_DIR}/Exception.h"
    "${INC_DIR}/IFormatConverter.h"
    "${INC_DIR}/FramePack.h"
    "${INC_DIR}/IPlugin.h"
    "${INC_DIR}/IPluginEffect.h"
    "${INC_DIR}/IPluginProcess.h"
    "${INC_DIR}/IPluginRender.h"
    "${INC_DIR}/TimelineHelper.h"
    "${INC_DIR}/Types.h"
)

INSTALL(TARGETS ${FW_NAME} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(
    FILES ${OVI_INCLUDE_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${FW_NAME}
)

# clear cache for plugin's cflags
SET(PLUGINS_FLAGS "" CACHE STRING "append plugin's cflags" FORCE)
ADD_SUBDIRECTORY(config)
ADD_SUBDIRECTORY(plugins)
IF(BUILD_OVI_TOOLS)
ADD_SUBDIRECTORY(tools)
ENDIF(BUILD_OVI_TOOLS)

# uninstall
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean
        COMMENT "distribution clean"
        COMMAND find
        ARGS    .
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)
