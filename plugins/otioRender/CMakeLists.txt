SET(TARGET_LIB otio_render)

FIND_PACKAGE(OpenTime)
IF(NOT OpenTime_FOUND)
    MESSAGE("OpenTime not found")
ELSE()
FIND_PACKAGE(OpenTimelineIO REQUIRED COMPONENTS OpenTime)
IF(NOT OpenTimelineIO_FOUND)
    MESSAGE("OpenTimelineIO not found")
ELSE()
ADD_LIBRARY(${TARGET_LIB} SHARED otioRender.cpp)

SET(PLUGINS_FLAGS "${PLUGINS_FLAGS} -DENABLE_OTIORENDER" CACHE STRING "" FORCE)

TARGET_INCLUDE_DIRECTORIES(${TARGET_LIB} PUBLIC ${IMATH_INCLUDE_DIRS})
INSTALL(TARGETS ${TARGET_LIB} DESTINATION ${PLUGIN_INSTALL_DIR})
ENDIF()  # NOT OpenTimelineIO_FOUND
ENDIF()  # NOT OpenTime_FOUND
