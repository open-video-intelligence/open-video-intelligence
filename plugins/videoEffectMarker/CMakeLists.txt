SET(TARGET_LIB video_effect_marker)

ADD_LIBRARY(${TARGET_LIB} SHARED videoEffectMarker.cpp)

INSTALL(TARGETS ${TARGET_LIB} DESTINATION ${PLUGIN_INSTALL_DIR})
