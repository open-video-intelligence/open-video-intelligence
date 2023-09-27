SET(PLUGINS_FLAGS "${PLUGINS_FLAGS} -DENABLE_FACEDETECTPY" CACHE STRING "" FORCE)

FIND_PACKAGE(Python REQUIRED COMPONENTS Interpreter)
EXECUTE_PROCESS(
	COMMAND pip show numpy
	RESULT_VARIABLE EXIT_CODE
	OUTPUT_QUIET
)
if (NOT ${EXIT_CODE} EQUAL 0)
	MESSAGE(
		WARNING
		"The \"numpy\" Python3 package is not installed. Please install it using the following command: \"pip3 install numpy\"."
	)
endif()
EXECUTE_PROCESS(
	COMMAND pip show mediapipe
	RESULT_VARIABLE EXIT_CODE
	OUTPUT_QUIET
)
if (NOT ${EXIT_CODE} EQUAL 0)
	MESSAGE(
		WARNING
		"The \"mediapipe\" Python3 package is not installed. Please install it using the following command: \"pip3 install mediapipe\"."
	)
endif()

INSTALL(
    FILES "faceDetect.py"
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

INSTALL(
	DIRECTORY "srcs" "models"
	DESTINATION ${PLUGIN_INSTALL_DIR}
    OPTIONAL
)
