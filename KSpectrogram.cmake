################################################################
# CMake file for using KSpectrogram as submodule               #
# 															   #
# Note : 'config.json' file must exist in '../config.json'     #
################################################################

list(APPEND SRC_Qt_MOC
	${CMAKE_CURRENT_LIST_DIR}/src/KRecordPlot.h
	${CMAKE_CURRENT_LIST_DIR}/src/KRecorderControl.h
)

list(APPEND SRC_Qt
	${CMAKE_CURRENT_LIST_DIR}/src/KInput.h
	${CMAKE_CURRENT_LIST_DIR}/src/KParam.h
	${CMAKE_CURRENT_LIST_DIR}/src/KRecorder.h
	${CMAKE_CURRENT_LIST_DIR}/src/KRecorderControl.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/KRecordPlot.cpp
	
)

list(APPEND SRC_Qt
	${CMAKE_CURRENT_LIST_DIR}/src/JsonConfig.h
	${CMAKE_CURRENT_LIST_DIR}/src/Config.h
	)
	
## PATH
list(APPEND COMPILE_OPTION
  -D_CONFIG_JSON="../config.json"
)